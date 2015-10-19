/* @(#)root/zip:$Id: ZInflate.c 29733 2009-08-07 20:10:30Z brun $ */
/* Author: */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "lzo/lzoutil.h" /* TODO remove */

#include "./brotli/enc/encode.h"
#include "./brotli/dec/decode.h"
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
extern "C" void R__error(const char *msg);
#define HDRSIZE 9

extern "C" int R__BrotliCompress(int cxlevel , uch* src, size_t srcsize, uch* target, size_t* dstsz);


int R__BrotliCompress(int cxlevel , uch* src, size_t srcsize, uch* target, size_t* dstsz)
{
  brotli::BrotliParams params;
  size_t compression_size = *dstsz;
  unsigned long adler32;
  lzo_uint obufs;
  lzo_uint osz;
  lzo_uintp obufsz;
  uch* obufptr;
  int status;

  params.quality = cxlevel;
  obufs = *dstsz;
  obufsz = &obufs;
  (target)[0] = 'B';
  (target)[1] = 'R';
  (target)[2] = 'O';
  status = BrotliCompressBuffer(params,srcsize,src,&compression_size,target + HDRSIZE);
  if (status == 0) {
    return -1;
    if (*dstsz < srcsize + HDRSIZE + 4) {
      R__error("could not leave uncompressed");
      return -1;
    }
    memmove(target + HDRSIZE,src,srcsize);
    target[2]=0;
  } else {
    if (*dstsz < compression_size + HDRSIZE + 4) {
      /* this is actually caught */
      R__error("could not compress");
      return -1;
    }
  }

  /* does all this make sense? */
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

  return 0;

}

int R__Bro_decompress(uch* ibufptr, long ibufsz,
        uch* obufptr, size_t* obufsz)
{
  int status;
  if (ibufsz < 4) {
    return -1;
  }
  {
    /* check adler32 checksum */
    uch *p = ibufptr + (ibufsz - 4);
    unsigned long adler = ((unsigned long) p[0]) | ((unsigned long) p[1] << 8) |
      ((unsigned long) p[2] << 16) | ((unsigned long) p[3] << 24);
    if (adler != lzo_adler32(lzo_adler32(0, NULL, 0), ibufptr, ibufsz - 4)) {
      /* corrupt compressed data */
      return -1;
    }
  }
  status = BrotliDecompressBuffer(ibufsz-4,ibufptr,obufsz,obufptr);
  if (0==status) return -1;
  return 0;
}

