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

int R__ZopfliCompress(ZopfliOptions* zpfopts, ZopfliFormat zpftype,
        uch* src, size_t srcsize, char* target, size_t* dstsz)
{
  uch* compression_target = 0;
  size_t compression_size = 0;
  unsigned long adler32;
  lzo_uint obufs;
  lzo_uint osz;
  lzo_uintp obufsz;
  uch* obufptr;

  /*printf("obufptr length: %d\n",strlen(target));*/
  /*printf("obufsz        : %d\n",*dstsz);*/
  /*for (k = 0 ; k < *dstsz ; ++k) {
    printf("%d\t%d\n",k,(target)[k]);
  }*/

  obufs = *dstsz;
  obufsz = &obufs;
  (target)[0] = 'Z';
  (target)[1] = 'P';
  if (ZOPFLI_FORMAT_ZLIB == zpftype) (target)[2] = 'Z';
  if (ZOPFLI_FORMAT_GZIP == zpftype) (target)[2] = 'G';
  if (ZOPFLI_FORMAT_DEFLATE == zpftype) (target)[2] = 'D';
  ZopfliCompress(zpfopts, zpftype, src, srcsize, &compression_target, &compression_size);
  if (*dstsz < compression_size + HDRSIZE + 4) {
    printf("this is going to fail\n");
    printf("had: %zu\tneeded: %zu\n",*dstsz,compression_size);
    R__error("will fail");
    return -1;
  }
  /*for (k = 0 ; k < compression_size ; ++k) {
    target[HDRSIZE+k] = (char)compression_target[k];
  }*/
  memmove(target + HDRSIZE,compression_target,compression_size);
  free(compression_target);

  *dstsz = compression_size + HDRSIZE + 4;
  *obufsz = compression_size + HDRSIZE + 4;
  osz = *obufsz - HDRSIZE;
  (target)[3] = (char)(((osz) >> 0) & 0xff);
  (target)[4] = (char)(((osz) >> 8) * 0xff);
  (target)[5] = (char)(((osz) >>16) * 0xff);
  (target)[6] = (char)(((srcsize) >> 0) & 0xff);
  (target)[7] = (char)(((srcsize) >> 8) * 0xff);
  (target)[8] = (char)(((srcsize) >>16) * 0xff);
  /* calculate checksum */
  adler32 = lzo_adler32(
      lzo_adler32(0, NULL,0), (target) + HDRSIZE, osz - 4);
  obufptr = target;
  obufptr += *obufsz - 4;
  obufptr[0] = (char) (adler32 & 0xff);
  obufptr[1] = (char) ((adler32 >> 8) & 0xff);
  obufptr[2] = (char) ((adler32 >> 16) & 0xff);
  obufptr[3] = (char) ((adler32 >> 24) & 0xff);

  
  /*printf("AFTER COMPRESSION\n");*/
  /*printf("ibufptr          : %s<<<\n",src);*/
  /*printf("ibufsz           : %u<<<\n",(unsigned int)srcsize);*/
  /*printf("strlen(ibufptr)  : %u<<<\n",(unsigned int)strlen((const char*)src));*/
  /*for (k = 0 ; k < srcsize ; ++k) {
    printf("%d\t%d\n",(unsigned int)k,src[k]);
  }*/
  /*printf("obufptr          : %s<<<\n",obufptr);*/
  /*printf("obufsz           : %u<<<\n",(unsigned int)*obufsz);*/
  /*printf("strlen(obufptr)  : %u<<<\n",(unsigned int)strlen((const char*)obufptr));*/
  /*for (k = 0 ; k < *obufsz ; ++k) {
    printf("%d\t%d\n",(unsigned int)k,obufptr[k]);
  }*/

  return 0;

}

