/*
 * Copyright (c) 1995 Danny Gasparovski.
 *
 * Please read the file COPYRIGHT for the
 * terms and conditions of the copyright.
 */

/* MINE */

#ifndef _SLIRP_SOCKET_H_
#define _SLIRP_SOCKET_H_

#define SO_EXPIRE 240000
#define SO_EXPIREFAST 10000

/*
 * Our socket structure
 */

struct socket {
  struct socket *so_next,*so_prev;      /* For a linked list of sockets */

  int s;                           /* The actual socket */

			/* XXX union these with not-yet-used sbuf params */
  struct mbuf *so_m;	           /* Pointer to the original SYN packet,
				    * for non-blocking connect()'s, and
				    * PING reply's */
  struct tcpiphdr *so_ti;	   /* Pointer to the original ti within
				    * so_mconn, for non-blocking connections */
  int so_urgc;
  SockAddress faddr;
  SockAddress laddr;
#define so_faddr_ip faddr.u.inet.address
#define so_laddr_ip laddr.u.inet.address
#define so_faddr_port faddr.u.inet.port
#define so_laddr_port laddr.u.inet.port
  uint16_t   so_haddr_port;

  u_int8_t	so_iptos;	/* Type of service */
  u_int8_t	so_emu;		/* Is the socket emulated? */

  u_char	so_type;		/* Type of socket, UDP or TCP */
  SocketFamily  so_family;      /* Family of socket */
  int	so_state;		/* internal state flags SS_*, below */

  struct 	tcpcb *so_tcpcb;	/* pointer to TCP protocol control block */
  u_int	so_expire;		/* When the socket will expire */

  int	so_queued;		/* Number of packets queued from this socket */
  int	so_nqueued;		/* Number of packets queued in a row
				 * Used to determine when to "downgrade" a session
					 * from fastq to batchq */

  struct sbuf so_rcv;		/* Receive buffer */
  struct sbuf so_snd;		/* Send buffer */
  void * extra;			/* Extra pointer */
};


/*
 * Socket state bits. (peer means the host on the Internet,
 * local host means the host on the other end of the modem)
 */
#define SS_NOFDREF		0x001	/* No fd reference */

#define SS_ISFCONNECTING	0x002	/* Socket is connecting to peer (non-blocking connect()'s) */
#define SS_ISFCONNECTED		0x004	/* Socket is connected to peer */
#define SS_FCANTRCVMORE		0x008	/* Socket can't receive more from peer (for half-closes) */
#define SS_FCANTSENDMORE	0x010	/* Socket can't send more to peer (for half-closes) */
/* #define SS_ISFDISCONNECTED	0x020*/	/* Socket has disconnected from peer, in 2MSL state */
#define SS_FWDRAIN		0x040	/* We received a FIN, drain data and set SS_FCANTSENDMORE */

#define SS_CTL			0x080
#define SS_FACCEPTCONN		0x100	/* Socket is accepting connections from a host on the internet */
#define SS_FACCEPTONCE		0x200	/* If set, the SS_FACCEPTCONN socket will die after one accept */
#define SS_PROXIFIED            0x400   /* Socket is trying to connect through a proxy, only makes sense
                                           when SS_ISFCONNECTING is also set */

#define SS_IPV6                 0x800   /* Listen on IPV6 address */

extern struct socket tcb;

void so_init _P((void));
struct socket * solookup _P((struct socket **, struct socket *,
                             SockAddress *, SockAddress *));
struct socket * socreate _P((void));
void sofree _P((struct socket *));
int soread _P((struct socket *));
void sorecvoob _P((struct socket *));
int sosendoob _P((struct socket *));
int sowrite _P((struct socket *));
void sorecvfrom _P((struct socket *));
int sosendto _P((struct socket *, struct mbuf *));
struct socket * solisten _P((u_int, u_int32_t, u_int, int));
int  sounlisten _P((struct socket *, u_int port));
void soisfconnecting _P((register struct socket *));
void soisfconnected _P((register struct socket *));
void soisfdisconnected _P((struct socket *));
void sofwdrain _P((struct socket *));
struct iovec; /* For win32 */
size_t sopreprbuf(struct socket *so, struct iovec *iov, int *np);
int soreadbuf(struct socket *so, const char *buf, int size);
int sotranslate_out(struct socket *so, SockAddress *addr);

#endif /* _SOCKET_H_ */
