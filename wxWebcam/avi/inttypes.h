

#ifndef _INTTYPES_H__
#define _INTTYPES_H__

#ifdef __UNIX__
#include <sys/types.h>
#include <inttypes.h>
#define MAXPATHLEN      250

#else // WIN32
#include <stdlib.h>

struct iovec
{
        void  *iov_base;  /* BSD uses caddr_t (1003.1g requires void *) */
        size_t iov_len;   /* Must be size_t (1003.1g) */
};

#define MAXPATHLEN      _MAX_PATH

#endif  // __UNIX__
#endif //_INTTYPES_H__

