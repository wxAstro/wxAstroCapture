#ifndef BYTESWAP_H
#define BYTESWAP_H

#include <sys/types.h>

#ifndef BIG_ENDIAN
    #define BIG_ENDIAN 0
    #define LITTLE_ENDIAN 1
    #define BYTE_ORDER LITTLE_ENDIAN
#endif


#define SWAP2(x) (((x>>8) & 0x00ff) |\
                  ((x<<8) & 0xff00))

#define SWAP4(x) (((x>>24) & 0x000000ff) |\
                  ((x>>8)  & 0x0000ff00) |\
                  ((x<<8)  & 0x00ff0000) |\
                  ((x<<24) & 0xff000000))

#endif /* BYTESWAP_H */
