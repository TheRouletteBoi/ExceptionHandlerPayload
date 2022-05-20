#ifndef LV2_NETWORK_H
#define LV2_NETWORK_H

#include "../Utils.h"
#include "SystemCalls.h"
#include "../PatternScanner.h"

typedef unsigned int in_addr_t;
typedef unsigned short in_port_t;
typedef unsigned char sa_family_t;
typedef unsigned int socklen_t;

#define SOCK_STREAM	1
#define SOCK_DGRAM	2
#define SOCK_RAW	3
#define SOCK_DGRAM_P2P	6
#define SOCK_STREAM_P2P	10

#define IPPROTO_IP	0
#define	IPPROTO_ICMP	1
#define IPPROTO_IGMP	2
#define	IPPROTO_TCP	6
#define	IPPROTO_UDP	17
#define IPPROTO_ICMPV6	58

/* address families */
#define AF_UNSPEC		0
#define AF_LOCAL		1
#define AF_UNIX			AF_LOCAL
#define AF_INET			2
#define AF_IMPLINK		3
#define AF_PUP			4
#define AF_CHAOS		5
#define AF_NS			6
#define AF_ISO			7
#define AF_OSI			AF_ISO
#define AF_ECMA			8
#define AF_DATAKIT		9
#define AF_CCITT		10
#define AF_SNA			11
#define AF_DECnet		12
#define AF_DLI			13
#define AF_LAT			14
#define AF_HYLINK		15
#define AF_APPLETALK		16
#define AF_ROUTE		17
#define AF_LINK			18
#define pseudo_AF_XTP		19
#define AF_COIP			20
#define AF_CNT			21
#define pseudo_AF_RTIP		22
#define AF_IPX			23
#define AF_INET6		24
#define pseudo_AF_PIP		25
#define AF_ISDN			26
#define AF_E164			AF_ISDN
#define AF_NATM			27
#define AF_ARP			28
#define pseudo_AF_KEY		29
#define pseudo_HDRCMPLT		30
#define AF_MAX			31

/* option flags */
#define SO_REUSEADDR	0x0004
#define SO_KEEPALIVE	0x0008
#define SO_BROADCAST	0x0020
#define SO_LINGER	0x0080
#define SO_OOBINLINE	0x0100
#define SO_REUSEPORT	0x0200
#define SO_ONESBCAST	0x0800
#define SO_USECRYPTO	0x1000
#define SO_USESIGNATURE	0x2000

/* additional options */
#define SO_SNDBUF	0x1001
#define SO_RCVBUF	0x1002
#define SO_SNDLOWAT	0x1003
#define SO_RCVLOWAT	0x1004
#define SO_SNDTIMEO	0x1005
#define SO_RCVTIMEO	0x1006
#define SO_ERROR	0x1007
#define SO_TYPE		0x1008

#define SO_NBIO		0x1100
#define SO_TPPOLICY	0x1101

struct linger {
    int l_onoff;			/* on/off */
    int l_linger;			/* linger time [sec] */
};

#define	SOL_SOCKET	0xffff		/* options for socket level */

#define	IN_CLASSA(i)		(((unsigned int)(i) & 0x80000000) == 0)
#define	IN_CLASSA_NET		0xff000000
#define	IN_CLASSA_NSHIFT	24
#define	IN_CLASSA_HOST		0x00ffffff
#define	IN_CLASSA_MAX		128

#define	IN_CLASSB(i)		(((unsigned int)(i) & 0xc0000000) == 0x80000000)
#define	IN_CLASSB_NET		0xffff0000
#define	IN_CLASSB_NSHIFT	16
#define	IN_CLASSB_HOST		0x0000ffff
#define	IN_CLASSB_MAX		65536

#define	IN_CLASSC(i)		(((unsigned int)(i) & 0xe0000000) == 0xc0000000)
#define	IN_CLASSC_NET		0xffffff00
#define	IN_CLASSC_NSHIFT	8
#define	IN_CLASSC_HOST		0x000000ff

#define	IN_CLASSD(i)		(((unsigned int)(i) & 0xf0000000) == 0xe0000000)
#define	IN_MULTICAST(i)		IN_CLASSD(i)

#define	INADDR_ANY		(unsigned int)0x00000000
#define	INADDR_LOOPBACK		(unsigned int)0x7f000001
#define	INADDR_BROADCAST	(unsigned int)0xffffffff
#define	INADDR_NONE		0xffffffff

#define	IN_LOOPBACKNET		127

struct sockaddr {
        unsigned char sa_len;
        sa_family_t sa_family;
        char sa_data[14];
};

struct in_addr {
    in_addr_t s_addr;
};

struct sockaddr_in {
    unsigned char sin_len;
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;
    char sin_zero[8];
};

struct sockaddr_in_p2p {
    unsigned char sin_len;
    sa_family_t	sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;
    in_port_t sin_vport;
    char sin_zero[6];
};

namespace lv2 {

static inline int sys_net_bnet_socket(int family, int type, int protocol)
{
    return SYSCALL_FN(&sys_net_bnet_socket, SYS_NET_SOCKET)(family, type, protocol);
}

static inline int sys_net_bnet_connect(int socket, const struct sockaddr* address, socklen_t addresslen)
{
    return SYSCALL_FN(&sys_net_bnet_connect, SYS_NET_CONNECT)(socket, address, addresslen);
}

static inline int sys_net_bnet_sendto(int socket, const void* buffer, int length, 
    int flags, const struct sockaddr* address, socklen_t addresslen)
{
    return SYSCALL_FN(&sys_net_bnet_sendto, SYS_NET_SENDTO)(socket, buffer, length, flags, address, addresslen);
}

static inline int sys_net_bnet_close(int socket)
{
    return SYSCALL_FN(&sys_net_bnet_close, SYS_NET_CLOSE)(socket);
}


}

#endif // !LV2_NETWORK_H