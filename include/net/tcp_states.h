/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Definitions for the TCP protocol sk_state field.
 */
#ifndef _LINUX_TCP_STATES_H
#define _LINUX_TCP_STATES_H

enum {
	TCP_ESTABLISHED = 1,
	TCP_SYN_SENT,
	TCP_SYN_RECV,           // 词不达意! 被fastopen占用. (listen sock 收到第三次握手后, 创建子tcp sock的初始状态)
	TCP_FIN_WAIT1,          // 已经发送了FIN(写关闭)
	TCP_FIN_WAIT2,          // 收到FIN的ACK
	TCP_TIME_WAIT,          // 收到FIN,且发送了FIN的ACK后
	TCP_CLOSE,
	TCP_CLOSE_WAIT,         // 收到FIN(读关闭)
	TCP_LAST_ACK,           // 收到FIN后,(自己的数据也发完了后), 自己也发了FIN, 等对方的最后一个ACK
	TCP_LISTEN,
	TCP_CLOSING,	/* Now a valid state */
	TCP_NEW_SYN_RECV,       // 三次握手的第二阶段，服务器发送synack后，会进入TCP_NEW_SYN_RECV状态，并插入ehash中

	TCP_MAX_STATES	/* Leave at the end! */
};

#define TCP_STATE_MASK	0xF

#define TCP_ACTION_FIN	(1 << TCP_CLOSE)

enum {
	TCPF_ESTABLISHED = (1 << TCP_ESTABLISHED),
	TCPF_SYN_SENT	 = (1 << TCP_SYN_SENT),
	TCPF_SYN_RECV	 = (1 << TCP_SYN_RECV),
	TCPF_FIN_WAIT1	 = (1 << TCP_FIN_WAIT1),
	TCPF_FIN_WAIT2	 = (1 << TCP_FIN_WAIT2),
	TCPF_TIME_WAIT	 = (1 << TCP_TIME_WAIT),
	TCPF_CLOSE	 = (1 << TCP_CLOSE),
	TCPF_CLOSE_WAIT	 = (1 << TCP_CLOSE_WAIT),
	TCPF_LAST_ACK	 = (1 << TCP_LAST_ACK),
	TCPF_LISTEN	 = (1 << TCP_LISTEN),
	TCPF_CLOSING	 = (1 << TCP_CLOSING),
	TCPF_NEW_SYN_RECV = (1 << TCP_NEW_SYN_RECV),
};

#endif	/* _LINUX_TCP_STATES_H */
