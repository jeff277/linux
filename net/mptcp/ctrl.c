// SPDX-License-Identifier: GPL-2.0
/* Multipath TCP
 *
 * Copyright (c) 2019, Tessares SA.
 */

#include <linux/sysctl.h>

#include <net/net_namespace.h>
#include <net/netns/generic.h>

#include "protocol.h"

#define MPTCP_SYSCTL_PATH "net/mptcp"

static int mptcp_pernet_id;
struct mptcp_pernet {
	struct ctl_table_header *ctl_table_hdr;

	int mptcp_enabled;
};

static struct mptcp_pernet *mptcp_get_pernet(struct net *net)
{
	return net_generic(net, mptcp_pernet_id);
}

int mptcp_is_enabled(struct net *net)
{
	return mptcp_get_pernet(net)->mptcp_enabled;
}

// 5.10内核, sysctl仅支持一个全局enable
static struct ctl_table mptcp_sysctl_table[] = {
	{
		.procname = "enabled",
		.maxlen = sizeof(int),
		.mode = 0644,
		/* users with CAP_NET_ADMIN or root (not and) can change this
		 * value, same as other sysctl or the 'net' tree.
		 */
		.proc_handler = proc_dointvec,
	},
	{}
};

static void mptcp_pernet_set_defaults(struct mptcp_pernet *pernet)
{
	pernet->mptcp_enabled = 1;
}

static int mptcp_pernet_new_table(struct net *net, struct mptcp_pernet *pernet)
{
	struct ctl_table_header *hdr;
	struct ctl_table *table;

	table = mptcp_sysctl_table;
	if (!net_eq(net, &init_net)) {
		table = kmemdup(table, sizeof(mptcp_sysctl_table), GFP_KERNEL);
		if (!table)
			goto err_alloc;
	}

	table[0].data = &pernet->mptcp_enabled;

	hdr = register_net_sysctl(net, MPTCP_SYSCTL_PATH, table);
	if (!hdr)
		goto err_reg;

	pernet->ctl_table_hdr = hdr;

	return 0;

err_reg:
	if (!net_eq(net, &init_net))
		kfree(table);
err_alloc:
	return -ENOMEM;
}

static void mptcp_pernet_del_table(struct mptcp_pernet *pernet)
{
	struct ctl_table *table = pernet->ctl_table_hdr->ctl_table_arg;

	unregister_net_sysctl_table(pernet->ctl_table_hdr);

	kfree(table);
}

static int __net_init mptcp_net_init(struct net *net)
{
	struct mptcp_pernet *pernet = mptcp_get_pernet(net);

	mptcp_pernet_set_defaults(pernet);

	return mptcp_pernet_new_table(net, pernet);
}

/* Note: the callback will only be called per extra netns */
static void __net_exit mptcp_net_exit(struct net *net)
{
	struct mptcp_pernet *pernet = mptcp_get_pernet(net);

	mptcp_pernet_del_table(pernet);
}

//"pernet" 在 Linux 内核术语中代表 "per-network namespace"，意即“每个网络命名空间”。网络命名空间是 Linux 提供的一种虚拟化技术，允许用户创建隔离的网络环境。
//每个网络命名空间都拥有独立的网络资源，如IP地址、路由表、防火墙规则和其他网络相关的状态。这种隔离使得运行在不同网络命名空间中的进程拥有各自独立的网络视图，从而增强了安全性和灵活性。
//在这个上下文中，"pernet subsystem"（每个网络子系统）是指那些需要在每个网络命名空间中独立管理和配置的子系统。这意味着每当创建新的网络命名空间时，
//这些子系统的实例也会在新的命名空间中被创建和初始化。这对于保持网络操作的隔离和一致性至关重要，尤其是在复杂的多租户环境或需要高度网络隔离的应用场景中。
//使用 pernet 子系统的设计允许内核在网络层面上提供更细粒度的控制和灵活性。对于MPTCP这样的协议，通过 pernet 子系统注册，它可以确保MPTCP在
//每个网络命名空间中正确地管理其连接、路径和相关的配置，而不会与其他命名空间冲突或相互干扰。这种方法强调了Linux网络栈的模块化和可扩展性，允许不同的网络技术和
//协议独立于全局系统环境而存在和操作。
// 大白话就是: 正常人的一台电脑就是一个namespace(一个pernet). 如果有安装VM, VM就是另外一个独立的namespace(另外一个pernet).
static struct pernet_operations mptcp_pernet_ops = {
	.init = mptcp_net_init,
	.exit = mptcp_net_exit,
	.id = &mptcp_pernet_id,
	.size = sizeof(struct mptcp_pernet),
};

// OS启动时调用, mptcp协议栈在内核的全局初始化.
void __init mptcp_init(void)
{
	mptcp_join_cookie_init();
	mptcp_proto_init();

	if (register_pernet_subsys(&mptcp_pernet_ops) < 0)
		panic("Failed to register MPTCP pernet subsystem.\n");
}

#if IS_ENABLED(CONFIG_MPTCP_IPV6)
int __init mptcpv6_init(void)
{
	int err;

	err = mptcp_proto_v6_init();

	return err;
}
#endif
