/* @(#)root/zip:$Id: ZInflate.c 29733 2009-08-07 20:10:30Z brun $ */
/* Author: */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "lzo/lzoutil.h" /* TODO remove */
#include "zopfli/zopfli.h"

#ifdef WIN32
#define __STDC__
#endif
#ifdef __MWERKS__
#define __STDC__
#endif

#ifndef NULL
#define NULL 0L
#endif

#if 0
#define PKZIP_BUG_WORKAROUND    /* PKZIP 1.93a problem--live with it */
#endif

typedef char              boolean;
typedef unsigned char     uch;  /* code assumes unsigned bytes; these type-  */
typedef unsigned short    ush;  /*  defs replace byte/UWORD/ULONG (which are */
typedef unsigned long     ulg;  /*  predefined on some systems) & match zip  */

/* Function prototypes */
#ifndef OF
#  ifdef __STDC__
#    define OF(a) a
#  else /* !__STDC__ */
#    define OF(a) ()
#  endif /* ?__STDC__ */
#endif
extern void R__error(const char *msg);
#define HDRSIZE 9

void R__ZopfliCompress(ZopfliOptions* zpfopts, ZopfliFormat zpftype,
        uch* src, size_t srcsize, uch** target, size_t* dstsz)
{
  uch* compression_target;
  size_t* compression_size;
  /* TODO some header stuff */
  (*target)[0] = 'Z';
  (*target)[1] = 'P';
  if (ZOPFLI_FORMAT_ZLIB == zpftype) (*target)[2] = 'Z';
  if (ZOPFLI_FORMAT_GZIP == zpftype) (*target)[2] = 'G';
  if (ZOPFLI_FORMAT_DEFLATE == zpftype) (*target)[2] = 'D';
  compression_target = (*target)+HDRSIZE;
  ZopfliCompress(zpfopts, zpftype, src, srcsize, &compression_target, compression_size);
  *dstsz = *compression_size + HDRSIZE;
  (*target)[3] = (char)(((*compression_size) >> 0) & 0xff);
  (*target)[4] = (char)(((*compression_size) >> 8) * 0xff);
  (*target)[5] = (char)(((*compression_size) >>16) * 0xff);
  (*target)[6] = (char)(((srcsize) >> 0) & 0xff);
  (*target)[7] = (char)(((srcsize) >> 8) * 0xff);
  (*target)[8] = (char)(((srcsize) >>16) * 0xff);
  
}

