/* @(#)root/zip:$Id: ZInflate.c 29733 2009-08-07 20:10:30Z brun $ */
/* Author: */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "lzo/lzoutil.h"

#include "zopfli/zopfli.h"
/* I define the following headers:
 * ZPZ: ZLIB decompression
 * ZPG: GZIP decompression
 * ZPD: deflate decompression
 */

#ifdef WIN32
#define __STDC__
#endif
#ifdef __MWERKS__
#define __STDC__
#endif

#ifndef NULL
#define NULL 0L
#endif

#include "zlib.h"

/* inflate.c -- put in the public domain by Mark Adler
   version c14o, 23 August 1994 */


/* You can do whatever you like with this source file, though I would
   prefer that if you modify it and redistribute it that you include
   comments to that effect with your name and the date.  Thank you.

   History:
   vers    date          who           what
   ----  ---------  --------------  ------------------------------------
    a    ~~ Feb 92  M. Adler        used full (large, one-step) lookup table
    b1   21 Mar 92  M. Adler        first version with partial lookup tables
    b2   21 Mar 92  M. Adler        fixed bug in fixed-code blocks
    b3   22 Mar 92  M. Adler        sped up match copies, cleaned up some
    b4   25 Mar 92  M. Adler        added prototypes; removed window[] (now
                                    is the responsibility of unzip.h--also
                                    changed name to slide[]), so needs diffs
                                    for unzip.c and unzip.h (this allows
                                    compiling in the small model on MSDOS);
                                    fixed cast of q in huft_build();
    b5   26 Mar 92  M. Adler        got rid of unintended macro recursion.
    b6   27 Mar 92  M. Adler        got rid of nextbyte() routine.  fixed
                                    bug in inflate_fixed().
    c1   30 Mar 92  M. Adler        removed lbits, dbits environment variables.
                                    changed BMAX to 16 for explode.  Removed
                                    OUTB usage, and replaced it with flush()--
                                    this was a 20% speed improvement!  Added
                                    an explode.c (to replace unimplod.c) that
                                    uses the huft routines here.  Removed
                                    register union.
    c2    4 Apr 92  M. Adler        fixed bug for file sizes a multiple of 32k.
    c3   10 Apr 92  M. Adler        reduced memory of code tables made by
                                    huft_build significantly (factor of two to
                                    three).
    c4   15 Apr 92  M. Adler        added NOMEMCPY do kill use of memcpy().
                                    worked around a Turbo C optimization bug.
    c5   21 Apr 92  M. Adler        added the WSIZE #define to allow reducing
                                    the 32K window size for specialized
                                    applications.
    c6   31 May 92  M. Adler        added some typecasts to eliminate warnings
    c7   27 Jun 92  G. Roelofs      added some more typecasts (444:  MSC bug).
    c8    5 Oct 92  J-l. Gailly     added ifdef'd code to deal with PKZIP bug.
    c9    9 Oct 92  M. Adler        removed a memory error message (~line 416).
    c10  17 Oct 92  G. Roelofs      changed ULONG/UWORD/byte to ulg/ush/uch,
                                    removed old inflate, renamed inflate_entry
                                    to inflate, added Mark's fix to a comment.
   c10.5 14 Dec 92  M. Adler        fix up error messages for incomplete trees.
    c11   2 Jan 93  M. Adler        fixed bug in detection of incomplete
                                    tables, and removed assumption that EOB is
                                    the longest code (bad assumption).
    c12   3 Jan 93  M. Adler        make tables for fixed blocks only once.
    c13   5 Jan 93  M. Adler        allow all zero length codes (pkzip 2.04c
                                    outputs one zero length code for an empty
                                    distance tree).
    c14  12 Mar 93  M. Adler        made inflate.c standalone with the
                                    introduction of inflate.h.
   c14b  16 Jul 93  G. Roelofs      added (unsigned) typecast to w at 470.
   c14c  19 Jul 93  J. Bush         changed v[N_MAX], l[288], ll[28x+3x] arrays
                                    to static for Amiga.
   c14d  13 Aug 93  J-l. Gailly     de-complicatified Mark's c[*p++]++ thing.
   c14e   8 Oct 93  G. Roelofs      changed memset() to memzero().
   c14f  22 Oct 93  G. Roelofs      renamed quietflg to qflag; made Trace()
                                    conditional; added inflate_free().
   c14g  28 Oct 93  G. Roelofs      changed l/(lx+1) macro to pointer (Cray bug)
   c14h   7 Dec 93  C. Ghisler      huft_build() optimizations.
   c14i   9 Jan 94  A. Verheijen    set fixed_t{d,l} to NULL after freeing;
                    G. Roelofs      check NEXTBYTE macro for EOF.
   c14j  23 Jan 94  G. Roelofs      removed Ghisler "optimizations"; ifdef'd
                                    EOF check.
   c14k  27 Feb 94  G. Roelofs      added some typecasts to avoid warnings.
   c14l   9 Apr 94  G. Roelofs      fixed split comments on preprocessor lines
                                    to avoid bug in Encore compiler.
   c14m   7 Jul 94  P. Kienitz      modified to allow assembler version of
                                    inflate_codes() (define ASM_INFLATECODES)
   c14n  22 Jul 94  G. Roelofs      changed fprintf to FPRINTF for DLL versions
   c14o  23 Aug 94  C. Spieler      added a newline to a debug statement;
                    G. Roelofs      added another typecast to avoid MSC warning
 */


/*
   Inflate deflated (PKZIP's method 8 compressed) data.  The compression
   method searches for as much of the current string of bytes (up to a
   length of 258) in the previous 32K bytes.  If it doesn't find any
   matches (of at least length 3), it codes the next byte.  Otherwise, it
   codes the length of the matched string and its distance backwards from
   the current position.  There is a single Huffman code that codes both
   single bytes (called "literals") and match lengths.  A second Huffman
   code codes the distance information, which follows a length code.  Each
   length or distance code actually represents a base value and a number
   of "extra" (sometimes zero) bits to get to add to the base value.  At
   the end of each deflated block is a special end-of-block (EOB) literal/
   length code.  The decoding process is basically: get a literal/length
   code; if EOB then done; if a literal, emit the decoded byte; if a
   length then get the distance and emit the referred-to bytes from the
   sliding window of previously emitted data.

   There are (currently) three kinds of inflate blocks: stored, fixed, and
   dynamic.  The compressor outputs a chunk of data at a time and decides
   which method to use on a chunk-by-chunk basis.  A chunk might typically
   be 32K to 64K, uncompressed.  If the chunk is uncompressible, then the
   "stored" method is used.  In this case, the bytes are simply stored as
   is, eight bits per byte, with none of the above coding.  The bytes are
   preceded by a count, since there is no longer an EOB code.

   If the data is compressible, then either the fixed or dynamic methods
   are used.  In the dynamic method, the compressed data is preceded by
   an encoding of the literal/length and distance Huffman codes that are
   to be used to decode this block.  The representation is itself Huffman
   coded, and so is preceded by a description of that code.  These code
   descriptions take up a little space, and so for small blocks, there is
   a predefined set of codes, called the fixed codes.  The fixed method is
   used if the block ends up smaller that way (usually for quite small
   chunks); otherwise the dynamic method is used.  In the latter case, the
   codes are customized to the probabilities in the current block and so
   can code it much better than the pre-determined fixed codes can.

   The Huffman codes themselves are decoded using a mutli-level table
   lookup, in order to maximize the speed of decoding plus the speed of
   building the decoding tables.  See the comments below that precede the
   lbits and dbits tuning parameters.
 */


/*
   Notes beyond the 1.93a appnote.txt:

   1. Distance pointers never point before the beginning of the output
      stream.
   2. Distance pointers can point back across blocks, up to 32k away.
   3. There is an implied maximum of 7 bits for the bit length table and
      15 bits for the actual data.
   4. If only one code exists, then it is encoded using one bit.  (Zero
      would be more efficient, but perhaps a little confusing.)  If two
      codes exist, they are coded using one bit each (0 and 1).
   5. There is no way of sending zero distance codes--a dummy must be
      sent if there are none.  (History: a pre 2.0 version of PKZIP would
      store blocks with no distance codes, but this was discovered to be
      too harsh a criterion.)  Valid only for 1.93a.  2.04c does allow
      zero distance codes, which is sent as one code of zero bits in
      length.
   6. There are up to 286 literal/length codes.  Code 256 represents the
      end-of-block.  Note however that the static length tree defines
      288 codes just to fill out the Huffman codes.  Codes 286 and 287
      cannot be used though, since there is no length base or extra bits
      defined for them.  Similarily, there are up to 30 distance codes.
      However, static trees define 32 codes (all 5 bits) to fill out the
      Huffman codes, but the last two had better not show up in the data.
   7. Unzip can check dynamic Huffman blocks for complete code sets.
      The exception is that a single code would not be complete (see #4).
   8. The five bits following the block type is really the number of
      literal codes sent minus 257.
   9. Length codes 8,16,16 are interpreted as 13 length codes of 8 bits
      (1+6+6).  Therefore, to output three times the length, you output
      three codes (1+1+1), whereas to output four times the same length,
      you only need two codes (1+3).  Hmm.
  10. In the tree reconstruction algorithm, Code = Code + Increment
      only if BitLength(i) is not zero.  (Pretty obvious.)
  11. Correction: 4 Bits: # of Bit Length codes - 4     (4 - 19)
  12. Note: length code 284 can represent 227-258, but length code 285
      really is 258.  The last length deserves its own, short code
      since it gets used a lot in very redundant files.  The length
      258 is special since 258 - 3 (the min match length) is 255.
  13. The literal/length and distance code bit lengths are read as a
      single stream of lengths.  It is possible (and advantageous) for
      a repeat code (16, 17, or 18) to go across the boundary between
      the two sets of lengths.
 */


#if 0
#define PKZIP_BUG_WORKAROUND    /* PKZIP 1.93a problem--live with it */
#endif

/*
    inflate.h must supply the uch slide[WSIZE] array and the NEXTBYTE,
    FLUSH() and memzero macros.  If the window size is not 32K, it
    should also define WSIZE.  If INFMOD is defined, it can include
    compiled functions to support the NEXTBYTE and/or FLUSH() macros.
    There are defaults for NEXTBYTE and FLUSH() below for use as
    examples of what those functions need to do.  Normally, you would
    also want FLUSH() to compute a crc on the data.  inflate.h also
    needs to provide these typedefs:

        typedef unsigned char uch;
        typedef unsigned short ush;
        typedef unsigned long ulg;

    This module uses the external functions malloc() and free() (and
    probably memset() or bzero() in the memzero() macro).  Their
    prototypes are normally found in <string.h> and <stdlib.h>.
 */
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
extern int R__Inflate OF((uch**, long*, uch**, long*));
extern int R__ZipMode;
extern void R__error(const char *msg);
extern void R__unzipLZMA(int*, uch*, int*, uch*, int*);
extern void R__zipLZMA(int, int*, char*, int*, char*, int*);
#define HDRSIZE 9


extern int R__lzo_decompress(uch* ibufptr, long ibufsz,
    uch* obufptr, long* obufsz, uch method);
extern int R__lzo_compress(int cxlevel, uch* ibufptr, lzo_uint ibufsz,
    uch* obufptr, lzo_uintp obufsz);
extern int R__lz4_compress(int cxlevel, uch* ibufptr, lzo_uint ibufsz,
    uch* obufptr, lzo_uintp obufsz);
extern int R__lz4_decompress(uch* ibufptr, long ibufsz,
    uch* obufptr, long* obufsz, uch method);
extern void R__ZopfliCompress(ZopfliOptions* zpfopts, ZopfliFormat zpftype,
    uch* src, size_t *srcsize, uch** target, size_t* dstsz);

/***********************************************************************
 *                                                                     *
 * Name: R__unzip                                    Date:    20.01.95 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: In memory ZIP decompression. Can be issued from FORTRAN.  *
 *           Written for DELPHI collaboration (CERN)                   *
 *                                                                     *
 * Input: scrsize - size of input buffer                               *
 *        src     - input buffer                                       *
 *        tgtsize - size of target buffer                              *
 *                                                                     *
 * Output: tgt - target buffer (decompressed)                          *
 *         irep - size of decompressed data                            *
 *                0 - if error                                         *
 *                                                                     *
 ***********************************************************************/

int R__unzip_header(int *srcsize, uch *src, int *tgtsize)
{ 
  /* Reads header envelope, and determines target size.
   * Returns 0 in case of success. */

  *srcsize = 0;
  *tgtsize = 0;

  /*   C H E C K   H E A D E R   */
  if (!(src[0] == 'Z' && src[1] == 'L' && src[2] == Z_DEFLATED) &&
      !(src[0] == 'C' && src[1] == 'S' && src[2] == Z_DEFLATED) &&
      !(src[0] == 'X' && src[1] == 'Z' && src[2] == 0) &&
      !(src[0] == 'L' && src[1] == '4') &&
      !(src[0] == 'L' && src[1] == 'Z') &&
      !(src[0] == 'Z' && src[1] == 'P')) {
    fprintf(stderr, "Error R__unzip_header: error in header\n");
    return 1;
  }

  *srcsize = HDRSIZE + ((long)src[3] | ((long)src[4] << 8) | ((long)src[5] << 16));
  *tgtsize = (long)src[6] | ((long)src[7] << 8) | ((long)src[8] << 16);

  return 0;
}

void R__unzip(int *srcsize, uch *src, int *tgtsize, uch *tgt, int *irep)
{
  long isize;
  uch  *ibufptr,*obufptr;
  long  ibufcnt, obufcnt;

  *irep = 0L;

  /*   C H E C K   H E A D E R   */

  if (*srcsize < HDRSIZE) {
    fprintf(stderr,"R__unzip: too small source\n");
    return;
  }

  if (!(src[0] == 'Z' && src[1] == 'L' && src[2] == Z_DEFLATED) &&
      !(src[0] == 'C' && src[1] == 'S' && src[2] == Z_DEFLATED) &&
      !(src[0] == 'X' && src[1] == 'Z' && src[2] == 0) &&
      !(src[0] == 'L' && src[1] == '4') &&
      !(src[0] == 'L' && src[1] == 'Z') &&
      !(src[0] == 'Z' && src[1] == 'P')) {
    fprintf(stderr,"R__unzip: error in header\n");
    return;
  }

  ibufptr = src + HDRSIZE;
  ibufcnt = (long)src[3] | ((long)src[4] << 8) | ((long)src[5] << 16);
  isize   = (long)src[6] | ((long)src[7] << 8) | ((long)src[8] << 16);
  obufptr = tgt;
  obufcnt = *tgtsize;

  if (obufcnt < isize) {
    fprintf(stderr,"R__unzip: too small target\n");
    return;
  }

  if (ibufcnt + HDRSIZE != *srcsize) {
    fprintf(stderr,"R__unzip: discrepancy in source length\n");
    return;
  }

  /*   D E C O M P R E S S   D A T A  */

  if (src[0] == 'L' && src[1] == 'Z') {
    /*fprintf(stdout,"LZO decompression");*/ /*TODO: use some output level magic here*/
    if (R__lzo_decompress(
          ibufptr, ibufcnt, obufptr, &obufcnt, src[2])) {
      fprintf(stderr, "R__unzip: failure to decompress with liblzo\n");
      return;
    }
    if (isize == obufcnt) *irep = obufcnt;
    return;
  }
  if (src[0] == 'L' && src[1] == '4') {
    /*fprintf(stdout,"LZ4 decompression");*/
    if (R__lz4_decompress(
          ibufptr, ibufcnt, obufptr, &obufcnt, src[2])) {
      fprintf(stderr, "R__unzip: failure to decompress with liblz4\n");
      return;
    }
    if (isize == obufcnt) *irep = obufcnt;
    return;
  }

  if (src[0] == 'X' && src[1] == 'Z') {
    /*fprintf(stdout,"LZMA decompression");*/
    R__unzipLZMA(srcsize, src, tgtsize, tgt, irep);
    return;
  }

  if ((src[0] == 'Z' && src[1] == 'P' && src[2] == 'D') ||
      (src[0] == 'Z' && src[1] == 'G' && src[2] == 'G')) {
    fprintf(stderr,"Zopfli decompression for deflate or gzip not implemented\n");
    return;
  }
  /* New zlib format */
  if ((src[0] == 'Z' && src[1] == 'L') ||
      (src[0] == 'Z' && src[1] == 'P' && src[2] == 'Z')) {
    z_stream stream; /* decompression stream */
    int err = 0;
    /*fprintf(stdout,"ZLIB decompression");*/

    stream.next_in   = (Bytef*)(&src[HDRSIZE]);
    stream.avail_in  = (uInt)(*srcsize);
    stream.next_out  = (Bytef*)tgt;
    stream.avail_out = (uInt)(*tgtsize);
    stream.zalloc    = (alloc_func)0;
    stream.zfree     = (free_func)0;
    stream.opaque    = (voidpf)0;

    err = inflateInit(&stream);
    if (err != Z_OK) {
      fprintf(stderr,"R__unzip: error %d in inflateInit (zlib)\n",err);
      return;
    }

    err = inflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
      inflateEnd(&stream);
      fprintf(stderr,"R__unzip: error %d in inflate (zlib)\n",err);
      return;
    }

    inflateEnd(&stream);

    *irep = stream.total_out;
    return;
  }

  /* Old zlib format */
  if (R__Inflate(&ibufptr, &ibufcnt, &obufptr, &obufcnt)) {
    fprintf(stderr,"R__unzip: error during decompression\n");
    return;
  }

  /* if (obufptr - tgt != isize) {
    There are some rare cases when a few more bytes are required */
  if (obufptr - tgt > *tgtsize) {
    fprintf(stderr,"R__unzip: discrepancy (%ld) with initial size: %ld, tgtsize=%d\n",
            (long)(obufptr - tgt),isize,*tgtsize);
    *irep = obufptr - tgt;
    return;
  }

  *irep = isize;
}

/***********************************************************************
 *                                                                     *
 * Name: R__zip                                      Date:    20.01.95 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: In memory ZIP compression                                 *
 *           It's a variant of R__memcompress adopted to be issued from*
 *           FORTRAN. Written for DELPHI collaboration (CERN)          *
 *                                                                     *
 * Input: cxlevel - compression level                                  *
 *        srcsize - size of input buffer                               *
 *        src     - input buffer                                       *
 *        tgtsize - size of target buffer                              *
 *                                                                     *
 * Output: tgt - target buffer (compressed)                            *
 *         irep - size of compressed data (0 - if error)               *
 *                                                                     *
 ***********************************************************************/

void R__zipMultipleAlgorithm(int cxlevel, int *srcsize, char *src, int *tgtsize, char *tgt, int *irep, int compressionAlgorithm)
     /* int cxlevel;                      compression level */
     /* int  *srcsize, *tgtsize, *irep;   source and target sizes, replay */
     /* char *tgt, *src;                  source and target buffers */

{
  int err;
  int method   = Z_DEFLATED;

  if (cxlevel <= 0) {
    *irep = 0;
    return;
  }

  if (0 == compressionAlgorithm)
    compressionAlgorithm = R__ZipMode;

  switch (compressionAlgorithm) {
    case 6: /* zopfli */
      {
        size_t dstsz = 0;
        ZopfliOptions zpfopts;
        ZopfliInitOptions(&zpfopts);
        zpfopts.numiterations = cxlevel;
        ZopfliFormat zpftype;
        zpftype = ZOPFLI_FORMAT_ZLIB; /* also possible GZIP or DEFLATE */
        uch* tgtu = (uch*) tgt;
        R__ZopfliCompress( &zpfopts, zpftype, (uch*) src, (size_t)*srcsize, &tgtu, &dstsz);
        *tgtsize = dstsz;
      }
      break;
    case 4:
      {
        lzo_uint dstsz = *tgtsize;
        if (R__lzo_compress(
              cxlevel, (uch*) src, (lzo_uint) *srcsize, (uch*) tgt, &dstsz)) {
          *irep = 0;
        } else {
          *irep = dstsz;
        }
        return;
      }
      break;
    case 5:
      {
        lzo_uint dstsz = *tgtsize;
        if (R__lz4_compress(
              cxlevel, (uch*) src, (lzo_uint) *srcsize, (uch*) tgt, &dstsz)) {
          *irep = 0;
        } else {
          *irep = dstsz;
        }
        return;
      }
      break;
    case 1:
      {
        unsigned in_size, out_size;
        z_stream stream;
        *irep = 0;

        if (*tgtsize <= HDRSIZE) {
          R__error("target buffer too small");
          return;
        }
        if (*srcsize > 0xffffff) {
          R__error("source buffer too big");
          return;
        }


        stream.next_in   = (Bytef*)src;
        stream.avail_in  = (uInt)(*srcsize);

        stream.next_out  = (Bytef*)(&tgt[HDRSIZE]);
        stream.avail_out = (uInt)(*tgtsize);

        stream.zalloc    = (alloc_func)0;
        stream.zfree     = (free_func)0;
        stream.opaque    = (voidpf)0;

        err = deflateInit(&stream, cxlevel);
        if (err != Z_OK) {
          printf("error %d in deflateInit (zlib)\n",err);
          return;
        }

        err = deflate(&stream, Z_FINISH);
        if (err != Z_STREAM_END) {
          deflateEnd(&stream);
          /* No need to print an error message. We simply abandon the compression
             the buffer cannot be compressed or compressed buffer would be larger than original buffer
             printf("error %d in deflate (zlib) is not = %d\n",err,Z_STREAM_END);
             */
          return;
        }

        err = deflateEnd(&stream);

        tgt[0] = 'Z';               /* Signature ZLib */
        tgt[1] = 'L';
        tgt[2] = (char) method;

        in_size   = (unsigned) (*srcsize);
        out_size  = stream.total_out;             /* compressed size */
        tgt[3] = (char)(out_size & 0xff);
        tgt[4] = (char)((out_size >> 8) & 0xff);
        tgt[5] = (char)((out_size >> 16) & 0xff);

        tgt[6] = (char)(in_size & 0xff);         /* decompressed size */
        tgt[7] = (char)((in_size >> 8) & 0xff);
        tgt[8] = (char)((in_size >> 16) & 0xff);

        *irep = stream.total_out + HDRSIZE;
      }
      break;
    case 2:
       R__zipLZMA(cxlevel, srcsize, src, tgtsize, tgt, irep);
       break;
    case 0:
    case 3:
      /* FIXME: put in code to support old compression alg here... */
      *irep = 0;
      R__error("old ZIP compression no longer supported");
      break;
    default:
      R__error("unsupported compression type");
      break;
  }
}


