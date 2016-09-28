#ifndef __BYTE_ORDER_H_
#define __BYTE_ORDER_H_

#include <byteswap.h>

#ifdef WIN32
#include <Winsock2.h>
#define ntohll(x)	bswap_64 (x)
#define htonll(x)	bswap_64 (x)
#else
#if __BYTE_ORDER == __BIG_ENDIAN
#define ntohl(x)	(x)
#define ntohs(x)	(x)       
#define htonl(x)	(x)       
#define htons(x)	(x)       
#define ntohll(x)	(x)
#define htonll(x)	(x)
#else 
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ntohl(x)	__bswap_32 (x)
#define ntohs(x)	__bswap_16 (x)
#define htonl(x)	__bswap_32 (x)
#define htons(x)	__bswap_16 (x)
#define ntohll(x)	__bswap_64 (x)
#define htonll(x)	__bswap_64 (x)
#endif
#endif
#endif

#endif
