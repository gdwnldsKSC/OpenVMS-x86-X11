/* $Xorg: Xpoll.h,v 1.4 2001/02/09 02:03:23 xorgcvs Exp $ */

/*

Copyright 1994, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/include/Xpoll.h,v 3.8 2001/01/17 17:53:11 dawes Exp $ */

#ifndef _XPOLL_H_
#define _XPOLL_H_

#ifndef WIN32

#ifndef USE_POLL

#include <X11/Xos.h>
#if !defined(DGUX)
#if (defined(SVR4) || defined(CRAY) || defined(AIXV3)) && !defined(FD_SETSIZE)
#include <sys/select.h>
#ifdef luna
#include <sysent.h>
#endif
#endif
#else /* DGUX  -- No sys/select in Intel DG/ux */
#include <sys/time.h> 
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef __QNX__  /* Make sure we get 256 bit select masks */
#define FD_SETSIZE 256
#include <sys/select.h>
#endif

/* AIX 4.2 fubar-ed <sys/select.h>, so go to heroic measures to get it */
#if defined(AIXV4) && !defined(NFDBITS)
#include <sys/select.h>
#endif
#include <X11/Xmd.h>
#ifdef CSRG_BASED
#include <sys/param.h>
# if BSD < 199103
typedef long fd_mask;
# endif
#endif

#define XFD_SETSIZE	256
#ifndef FD_SETSIZE
#define FD_SETSIZE	XFD_SETSIZE
#endif

#ifndef NBBY
#define NBBY	8		/* number of bits in a byte */
#endif

#ifndef NFDBITS
#define NFDBITS (sizeof(fd_mask) * NBBY)	/* bits per mask */
#endif

#ifndef howmany
#define howmany(x,y)	(((x)+((y)-1))/(y))
#endif

#ifdef BSD
# if BSD < 198911	/* 198911 == OSF/1, 199103 == CSRG_BASED */
#  ifndef luna		/* and even though on LUNA BSD ==  43, it has it */
typedef struct fd_set {
	fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;
#  endif
# endif
#endif

#ifndef hpux /* and perhaps old BSD ??? */
# define Select(n,r,w,e,t) select(n,(fd_set*)r,(fd_set*)w,(fd_set*)e,(struct timeval*)t)
#else
# ifndef _XPG4_EXTENDED /* HPUX 9.x and earlier */
#  define Select(n,r,w,e,t) select(n,(int*)r,(int*)w,(int*)e,(struct timeval*)t)
# else
#  define Select(n,r,w,e,t) select(n,(fd_set*)r,(fd_set*)w,(fd_set*)e,(struct timeval*)t)
# endif
#endif

#ifndef FD_SET
#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= ((fd_mask)1 << ((n) % NFDBITS)))
#endif
#ifndef FD_CLR
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~((fd_mask)1 << ((n) % NFDBITS)))
#endif
#ifndef FD_ISSET
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & ((fd_mask)1 << ((n) % NFDBITS)))
#endif
#ifndef FD_ZERO
#define FD_ZERO(p)      bzero((char *)(p), sizeof(*(p)))
#endif

/*
 * The howmany(FD_SETSIZE, NFDBITS) computes the number of elements in the
 * array. before accessing an element in the array we check it exists.
 * If it does not exist then the compiler discards the code to access it. 
 */
#define XFD_ANYSET(p) \
        ((howmany(FD_SETSIZE, NFDBITS) > 0 && (p)->fds_bits[0]) || \
        (howmany(FD_SETSIZE, NFDBITS) > 1 && (p)->fds_bits[1]) || \
        (howmany(FD_SETSIZE, NFDBITS) > 2 && (p)->fds_bits[2]) || \
        (howmany(FD_SETSIZE, NFDBITS) > 3 && (p)->fds_bits[3]) || \
        (howmany(FD_SETSIZE, NFDBITS) > 4 && (p)->fds_bits[4]) || \
        (howmany(FD_SETSIZE, NFDBITS) > 5 && (p)->fds_bits[5]) || \
        (howmany(FD_SETSIZE, NFDBITS) > 6 && (p)->fds_bits[6]) || \
        (howmany(FD_SETSIZE, NFDBITS) > 7 && (p)->fds_bits[7]))

#define XFD_COPYSET(src,dst) { \
        int __i__; \
		for (__i__ = 0; __i__ < howmany(FD_SETSIZE, NFDBITS); __i__++) \
            (dst)->fds_bits[__i__] = (src)->fds_bits[__i__]; \
        }
#define XFD_ANDSET(dst,b1,b2) { \
        int __i__; \
        for (__i__ = 0; __i__ < howmany(FD_SETSIZE, NFDBITS); __i__++) \
            (dst)->fds_bits[__i__] = ((b1)->fds_bits[__i__] & (b2)->fds_bits[__i__]); \
        }
#define XFD_ORSET(dst,b1,b2) { \
        int __i__; \
        for (__i__ = 0; __i__ < howmany(FD_SETSIZE, NFDBITS); __i__++) \
		    (dst)->fds_bits[__i__] = ((b1)->fds_bits[__i__] | (b2)->fds_bits[__i__]); \
        }        
#define XFD_UNSET(dst,b1) { \
        int __i__; \
        for (__i__ = 0; __i__ < howmany(FD_SETSIZE, NFDBITS); __i__++) \
    		(dst)->fds_bits[__i__] &= ~((b1)->fds_bits[__i__]); \
        }

#else /* USE_POLL */
#include <sys/poll.h>
#endif /* USE_POLL */

#else /* WIN32 */

#define XFD_SETSIZE	256
#ifndef FD_SETSIZE
#define FD_SETSIZE	XFD_SETSIZE
#endif
#include <X11/Xwinsock.h>

#define Select(n,r,w,e,t) select(0,(fd_set*)r,(fd_set*)w,(fd_set*)e,(struct timeval*)t)

#define XFD_SETCOUNT(p)	(((fd_set FAR *)(p))->fd_count)
#define XFD_FD(p,i) (((fd_set FAR *)(p))->fd_array[i])
#define XFD_ANYSET(p)	XFD_SETCOUNT(p)

#define XFD_COPYSET(src,dst) { \
    u_int __i; \
    FD_ZERO(dst); \
    for (__i = 0; __i < XFD_SETCOUNT(src) ; __i++) { \
        XFD_FD(dst,__i) = XFD_FD(src,__i); \
    } \
    XFD_SETCOUNT(dst) = XFD_SETCOUNT(src); \
}

#define XFD_ANDSET(dst,b1,b2) { \
    u_int __i; \
    FD_ZERO(dst); \
    for (__i = 0; __i < XFD_SETCOUNT(b1) ; __i++) { \
        if (FD_ISSET(XFD_FD(b1,__i), b2)) \
	   FD_SET(XFD_FD(b1,__i), dst); \
    } \
}

#define XFD_ORSET(dst,b1,b2) { \
    u_int __i; \
    XFD_COPYSET(b1,dst); \
    for (__i = 0; __i < XFD_SETCOUNT(b2) ; __i++) { \
        if (!FD_ISSET(XFD_FD(b2,__i), dst)) \
	   FD_SET(XFD_FD(b2,__i), dst); \
    } \
}

/* this one is really sub-optimal */
#define XFD_UNSET(dst,b1) { \
    u_int __i; \
    for (__i = 0; __i < XFD_SETCOUNT(b1) ; __i++) { \
	FD_CLR(XFD_FD(b1,__i), dst); \
    } \
}

/* we have to pay the price of having an array here, unlike with bitmasks
   calling twice FD_SET with the same fd is not transparent, so be careful */
#undef FD_SET
#define FD_SET(fd,set) do { \
    if (XFD_SETCOUNT(set) < FD_SETSIZE && !FD_ISSET(fd,set)) \
        XFD_FD(set,XFD_SETCOUNT(set)++)=(fd); \
} while(0)

#define getdtablesize() FD_SETSIZE 

#endif /* WIN32 */

#endif /* _XPOLL_H_ */
