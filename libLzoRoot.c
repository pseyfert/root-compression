/* @(#)root/zip:$Id: ZInflate.c 29733 2009-08-07 20:10:30Z brun $ */
/* Author: */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lzo/lzoutil.h"
#include "lzo/lzo1.h"
#include "lzo/lzo1a.h"
#include "lzo/lzo1b.h"
#include "lzo/lzo1c.h"
#include "lzo/lzo1f.h"
#include "lzo/lzo1x.h"
#include "lzo/lzo1y.h"
#include "lzo/lzo1z.h"
#include "lzo/lzo2a.h"

#include "lz4/lz4.h"

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


/***********************************************************************
 *
 * begin liblzo related routines and definitions
 *
 **********************************************************************/ 
static int R__lzo_init()
{
  static volatile int R__lzo_inited = 0;
  if (R__lzo_inited) return 1;
  return (R__lzo_inited = (lzo_init() == LZO_E_OK));
}

static int R__lzo_decompress(uch* ibufptr, long ibufsz,
    uch* obufptr, long* obufsz, uch method)
{
  lzo_uint osz = *obufsz;
  if (ibufsz < 4) {
    return -1;
  }
  /* initialise liblzo */
  if (!R__lzo_init()) return -1;
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
  switch (method) {
    case 0: /* just store the uncompressed data */
      if (*obufsz != ibufsz - 4) return -1;
      if (ibufptr != obufptr)
	lzo_memmove(obufptr, ibufptr, ibufsz - 4);
      break;
    case 1: /* LZO1 */
      if (LZO_E_OK != lzo1_decompress(ibufptr, ibufsz - 4,
	    obufptr, &osz, NULL) || ((unsigned long) *obufsz != osz))
	return -1;
      break;
    case 2: /* LZO1A */
      if (LZO_E_OK != lzo1a_decompress(ibufptr, ibufsz - 4,
	    obufptr, &osz, NULL) || ((unsigned long) *obufsz != osz))
	return -1;
      break;
    case 3: /* LZO1B */
      if (LZO_E_OK != lzo1b_decompress_safe(ibufptr, ibufsz - 4,
	    obufptr, &osz, NULL) || ((unsigned long) *obufsz != osz))
	return -1;
      break;
    case 4: /* LZO1C */
      if (LZO_E_OK != lzo1c_decompress_safe(ibufptr, ibufsz - 4,
	    obufptr, &osz, NULL) || ((unsigned long) *obufsz != osz))  
	return -1;
      break;
    case 5: /* LZO1F */
      if (LZO_E_OK != lzo1f_decompress_safe(ibufptr, ibufsz - 4,
	    obufptr, &osz, NULL) || ((unsigned long) *obufsz != osz))
	return -1;
      break;
    case 6: /* LZO1X */
      if (LZO_E_OK != lzo1x_decompress_safe(ibufptr, ibufsz - 4,
	    obufptr, &osz, NULL) || ((unsigned long) *obufsz != osz))
	return -1;
      break;
    case 7: /* LZO1Y */
      if (LZO_E_OK != lzo1y_decompress_safe(ibufptr, ibufsz - 4,
	    obufptr, &osz, NULL) || ((unsigned long) *obufsz != osz))
	return -1;
      break;
    case 8: /* LZO1Z */
      if (LZO_E_OK != lzo1z_decompress_safe(ibufptr, ibufsz - 4,
	    obufptr, &osz, NULL) || ((unsigned long) *obufsz != osz))
	return -1;
      break;
    case 9: /* LZO2A */
      if (LZO_E_OK != lzo2a_decompress_safe(ibufptr, ibufsz - 4,
	    obufptr, &osz, NULL) || ((unsigned long) *obufsz != osz))
	return -1;
      break;
    default:
      /* unknown method */
      return -1;
  }
  return 0;
}

/* definition of struct to map compression level to algorithms and settings */
struct R__lzo_tbl_t {
  int method;			/* method code to be written to file */
  lzo_compress_t compress;	/* ptr to compression routine */
  unsigned long wkspsz;	/* size of required workspace */
  lzo_optimize_t optimize;	/* ptr to optimize routine */
};

static struct R__lzo_tbl_t R__lzo_compr_tbl[9][11] = {
  { /* idx 0: LZO1X */
    { 6, lzo1x_1_11_compress, LZO1X_1_11_MEM_COMPRESS, lzo1x_optimize },
    { 6, lzo1x_1_12_compress, LZO1X_1_12_MEM_COMPRESS, lzo1x_optimize },
    { 6, lzo1x_1_compress, LZO1X_1_MEM_COMPRESS, lzo1x_optimize },
    { 6, lzo1x_1_compress, LZO1X_1_MEM_COMPRESS, lzo1x_optimize },
    { 6, lzo1x_1_15_compress, LZO1X_1_15_MEM_COMPRESS, lzo1x_optimize },
    { 6, lzo1x_1_15_compress, LZO1X_1_15_MEM_COMPRESS, lzo1x_optimize },
    { 6, lzo1x_999_compress, LZO1X_999_MEM_COMPRESS, lzo1x_optimize },
    { 6, lzo1x_999_compress, LZO1X_999_MEM_COMPRESS, lzo1x_optimize },
    { 6, lzo1x_999_compress, LZO1X_999_MEM_COMPRESS, lzo1x_optimize },
    { 6, lzo1x_999_compress, LZO1X_999_MEM_COMPRESS, lzo1x_optimize },
    { 6, lzo1x_999_compress, LZO1X_999_MEM_COMPRESS, lzo1x_optimize },
  },
  { /* idx 1: LZO1 */
    { 1, lzo1_compress, LZO1_MEM_COMPRESS, NULL },
    { 1, lzo1_compress, LZO1_MEM_COMPRESS, NULL },
    { 1, lzo1_compress, LZO1_MEM_COMPRESS, NULL },
    { 1, lzo1_compress, LZO1_MEM_COMPRESS, NULL },
    { 1, lzo1_compress, LZO1_MEM_COMPRESS, NULL },
    { 1, lzo1_compress, LZO1_MEM_COMPRESS, NULL },
    { 1, lzo1_99_compress, LZO1_99_MEM_COMPRESS, NULL },
    { 1, lzo1_99_compress, LZO1_99_MEM_COMPRESS, NULL },
    { 1, lzo1_99_compress, LZO1_99_MEM_COMPRESS, NULL },
    { 1, lzo1_99_compress, LZO1_99_MEM_COMPRESS, NULL },
    { 1, lzo1_99_compress, LZO1_99_MEM_COMPRESS, NULL },
  },
  { /* idx 2: LZO1A */
    { 2, lzo1a_compress, LZO1A_MEM_COMPRESS, NULL },
    { 2, lzo1a_compress, LZO1A_MEM_COMPRESS, NULL },
    { 2, lzo1a_compress, LZO1A_MEM_COMPRESS, NULL },
    { 2, lzo1a_compress, LZO1A_MEM_COMPRESS, NULL },
    { 2, lzo1a_compress, LZO1A_MEM_COMPRESS, NULL },
    { 2, lzo1a_compress, LZO1A_MEM_COMPRESS, NULL },
    { 2, lzo1a_99_compress, LZO1A_99_MEM_COMPRESS, NULL },
    { 2, lzo1a_99_compress, LZO1A_99_MEM_COMPRESS, NULL },
    { 2, lzo1a_99_compress, LZO1A_99_MEM_COMPRESS, NULL },
    { 2, lzo1a_99_compress, LZO1A_99_MEM_COMPRESS, NULL },
    { 2, lzo1a_99_compress, LZO1A_99_MEM_COMPRESS, NULL },
  },
  { /* idx 3: LZO1B */
    { 3, lzo1b_1_compress, LZO1B_MEM_COMPRESS, NULL },
    { 3, lzo1b_2_compress, LZO1B_MEM_COMPRESS, NULL },
    { 3, lzo1b_3_compress, LZO1B_MEM_COMPRESS, NULL },
    { 3, lzo1b_4_compress, LZO1B_MEM_COMPRESS, NULL },
    { 3, lzo1b_5_compress, LZO1B_MEM_COMPRESS, NULL },
    { 3, lzo1b_6_compress, LZO1B_MEM_COMPRESS, NULL },
    { 3, lzo1b_7_compress, LZO1B_MEM_COMPRESS, NULL },
    { 3, lzo1b_8_compress, LZO1B_MEM_COMPRESS, NULL },
    { 3, lzo1b_9_compress, LZO1B_MEM_COMPRESS, NULL },
    { 3, lzo1b_99_compress, LZO1B_99_MEM_COMPRESS, NULL },
    { 3, lzo1b_999_compress, LZO1B_999_MEM_COMPRESS, NULL },
  },
  { /* idx 4: LZO1C */
    { 4, lzo1c_1_compress, LZO1C_MEM_COMPRESS, NULL },
    { 4, lzo1c_2_compress, LZO1C_MEM_COMPRESS, NULL },
    { 4, lzo1c_3_compress, LZO1C_MEM_COMPRESS, NULL },
    { 4, lzo1c_4_compress, LZO1C_MEM_COMPRESS, NULL },
    { 4, lzo1c_5_compress, LZO1C_MEM_COMPRESS, NULL },
    { 4, lzo1c_6_compress, LZO1C_MEM_COMPRESS, NULL },
    { 4, lzo1c_7_compress, LZO1C_MEM_COMPRESS, NULL },
    { 4, lzo1c_8_compress, LZO1C_MEM_COMPRESS, NULL },
    { 4, lzo1c_9_compress, LZO1C_MEM_COMPRESS, NULL },
    { 4, lzo1c_99_compress, LZO1C_99_MEM_COMPRESS, NULL },
    { 4, lzo1c_999_compress, LZO1C_999_MEM_COMPRESS, NULL },
  },
  { /* idx 5: LZO1F */
    { 5, lzo1f_1_compress, LZO1F_MEM_COMPRESS, NULL },
    { 5, lzo1f_1_compress, LZO1F_MEM_COMPRESS, NULL },
    { 5, lzo1f_1_compress, LZO1F_MEM_COMPRESS, NULL },
    { 5, lzo1f_1_compress, LZO1F_MEM_COMPRESS, NULL },
    { 5, lzo1f_1_compress, LZO1F_MEM_COMPRESS, NULL },
    { 5, lzo1f_1_compress, LZO1F_MEM_COMPRESS, NULL },
    { 5, lzo1f_999_compress, LZO1F_999_MEM_COMPRESS, NULL },
    { 5, lzo1f_999_compress, LZO1F_999_MEM_COMPRESS, NULL },
    { 5, lzo1f_999_compress, LZO1F_999_MEM_COMPRESS, NULL },
    { 5, lzo1f_999_compress, LZO1F_999_MEM_COMPRESS, NULL },
    { 5, lzo1f_999_compress, LZO1F_999_MEM_COMPRESS, NULL },
  },
  { /* idx 6: LZO1Y */
    { 7, lzo1y_1_compress, LZO1Y_MEM_COMPRESS, lzo1y_optimize },
    { 7, lzo1y_1_compress, LZO1Y_MEM_COMPRESS, lzo1y_optimize },
    { 7, lzo1y_1_compress, LZO1Y_MEM_COMPRESS, lzo1y_optimize },
    { 7, lzo1y_1_compress, LZO1Y_MEM_COMPRESS, lzo1y_optimize },
    { 7, lzo1y_1_compress, LZO1Y_MEM_COMPRESS, lzo1y_optimize },
    { 7, lzo1y_1_compress, LZO1Y_MEM_COMPRESS, lzo1y_optimize },
    { 7, lzo1y_999_compress, LZO1Y_999_MEM_COMPRESS, lzo1y_optimize },
    { 7, lzo1y_999_compress, LZO1Y_999_MEM_COMPRESS, lzo1y_optimize },
    { 7, lzo1y_999_compress, LZO1Y_999_MEM_COMPRESS, lzo1y_optimize },
    { 7, lzo1y_999_compress, LZO1Y_999_MEM_COMPRESS, lzo1y_optimize },
    { 7, lzo1y_999_compress, LZO1Y_999_MEM_COMPRESS, lzo1y_optimize },
  },
  { /* idx 7: LZO1Z */
    { 8, lzo1z_999_compress, LZO1Z_999_MEM_COMPRESS, NULL },
    { 8, lzo1z_999_compress, LZO1Z_999_MEM_COMPRESS, NULL },
    { 8, lzo1z_999_compress, LZO1Z_999_MEM_COMPRESS, NULL },
    { 8, lzo1z_999_compress, LZO1Z_999_MEM_COMPRESS, NULL },
    { 8, lzo1z_999_compress, LZO1Z_999_MEM_COMPRESS, NULL },
    { 8, lzo1z_999_compress, LZO1Z_999_MEM_COMPRESS, NULL },
    { 8, lzo1z_999_compress, LZO1Z_999_MEM_COMPRESS, NULL },
    { 8, lzo1z_999_compress, LZO1Z_999_MEM_COMPRESS, NULL },
    { 8, lzo1z_999_compress, LZO1Z_999_MEM_COMPRESS, NULL },
    { 8, lzo1z_999_compress, LZO1Z_999_MEM_COMPRESS, NULL },
    { 8, lzo1z_999_compress, LZO1Z_999_MEM_COMPRESS, NULL },
  },
  { /* idx 8: LZO2A */
    { 9, lzo2a_999_compress, LZO2A_999_MEM_COMPRESS, NULL },
    { 9, lzo2a_999_compress, LZO2A_999_MEM_COMPRESS, NULL },
    { 9, lzo2a_999_compress, LZO2A_999_MEM_COMPRESS, NULL },
    { 9, lzo2a_999_compress, LZO2A_999_MEM_COMPRESS, NULL },
    { 9, lzo2a_999_compress, LZO2A_999_MEM_COMPRESS, NULL },
    { 9, lzo2a_999_compress, LZO2A_999_MEM_COMPRESS, NULL },
    { 9, lzo2a_999_compress, LZO2A_999_MEM_COMPRESS, NULL },
    { 9, lzo2a_999_compress, LZO2A_999_MEM_COMPRESS, NULL },
    { 9, lzo2a_999_compress, LZO2A_999_MEM_COMPRESS, NULL },
    { 9, lzo2a_999_compress, LZO2A_999_MEM_COMPRESS, NULL },
    { 9, lzo2a_999_compress, LZO2A_999_MEM_COMPRESS, NULL },
  },
};

static int R__lzo_compress(int cxlevel, uch* ibufptr, lzo_uint ibufsz,
    uch* obufptr, lzo_uintp obufsz)
{
  lzo_uint osz = *obufsz, minosz; 
  unsigned long adler32 = 0;
  int level = cxlevel & 0xf;
  int alg = (cxlevel >> 4) & 0xf;
  int opt = cxlevel & 0x100;
  *obufsz = 0;
  if (level > 0xb || alg > 0x8 || cxlevel > 0x1ff)
    return -1;
  if (0 == level) alg = opt = 0;
  /* calculate the buffer size needed for safe in place compression */
  minosz = ibufsz + (ibufsz / 16) + 64 + 3;
  if (8 == alg) minosz = ibufsz + (ibufsz / 8) + 128 + 3;
  if (0 == level) minosz = ibufsz;
  minosz += HDRSIZE + 4; /* header plus check sum */
  /* check buffer sizes */
  if (osz <= HDRSIZE + 4) {
    R__error("target buffer too small");
    return -1;
  }
  if (ibufsz > 0xffffff) {
    R__error("source buffer too large");
    return -1;
  }
  /* init header */
  obufptr[0] = 'L';
  obufptr[1] = 'Z';
  /* compress with specified level and algorithm */
  if (level > 0) {
    struct R__lzo_tbl_t *algp = &R__lzo_compr_tbl[alg][level - 1];
    uch* obuf = obufptr + HDRSIZE;
    uch* wksp = NULL;
    lzo_uint csz = 0;
    /* initialise liblzo */
    if (!R__lzo_init()) return -1;
    /* allocate workspace and safe temp buffer (if needed) */
    if (minosz > osz) {
      wksp = (uch*) lzo_malloc(algp->wkspsz + minosz - HDRSIZE - 4);
      obuf = wksp + algp->wkspsz;
    } else {
      wksp = (uch*) lzo_malloc(algp->wkspsz);
    }
    if (NULL == wksp) {
      R__error("out of memory");
      return -1;
    }
    /* compress */
    if (LZO_E_OK != algp->compress(ibufptr, ibufsz, obuf, &csz, wksp)) {
      /* something is wrong, try to store uncompressed below */
      alg = level = opt = 0;
      R__error("liblzo: unable to compress, trying to store as is");
    } else {
      /* compression ok, check if we need to optimize */
      if (opt && algp->optimize) {
	lzo_uint ucsz = ibufsz;
	if (LZO_E_OK != algp->optimize(obuf, csz, ibufptr, &ucsz, NULL) ||
	    ibufsz != ucsz) {
	  /* something is wrong, try to store uncompressed below */
	  alg = level = opt = 0;
	  R__error("liblzo: unable to optimize, trying to store as is");
	}
      }

      /* check compression ratio */
      if (csz < ibufsz && 0 != level) {
	/* check if we need to copy from temp to final buffer */
	if (obuf != obufptr + HDRSIZE) {
	  /* check for sufficient space and copy */
	  minosz = csz + HDRSIZE + 4;
	  if (osz < minosz) {
	    /* not enough space - try to store */
	    alg = level = opt = 0;
	  } else {
	    lzo_memcpy(obufptr + HDRSIZE, obuf, csz);
	    obufptr[2] = algp->method;
	    *obufsz = csz + HDRSIZE + 4;
	  }
	} else {
	  obufptr[2] = algp->method;
	  *obufsz = csz + HDRSIZE + 4;
	}
      } else {
	/* uncompressible, try to store uncompressed below */
	alg = level = opt = 0;
      }
    }
    lzo_free(wksp);
  }
  /* check if we are to store uncompressed */
  if (0 == level) {
    /* check for sufficient space */
    minosz = ibufsz + HDRSIZE + 4;
    if (osz < minosz) {
      R__error("target buffer too small");
      return -1;
    }
    *obufsz = minosz;
    /* copy to output buffer (move, buffers might overlap) */
    lzo_memmove(obufptr + HDRSIZE, ibufptr, ibufsz);
    obufptr[2] = 0; /* store uncompressed */
  };
  /* fill in sizes */
  osz = *obufsz - HDRSIZE;
  obufptr[3] = (char)(osz & 0xff);	/* compressed size */
  obufptr[4] = (char)((osz >> 8) & 0xff);
  obufptr[5] = (char)((osz >> 16) & 0xff);

  obufptr[6] = (char)(ibufsz & 0xff);	/* decompressed size */
  obufptr[7] = (char)((ibufsz >> 8) & 0xff);
  obufptr[8] = (char)((ibufsz >> 16) & 0xff);
  /* calculate checksum */
  adler32 = lzo_adler32(
	  lzo_adler32(0, NULL,0), obufptr + HDRSIZE, osz - 4);
  obufptr += *obufsz - 4;
  obufptr[0] = (char) (adler32 & 0xff);
  obufptr[1] = (char) ((adler32 >> 8) & 0xff);
  obufptr[2] = (char) ((adler32 >> 16) & 0xff);
  obufptr[3] = (char) ((adler32 >> 24) & 0xff);

  return 0;
}

/***********************************************************************
 *
 * end liblzo related routines and definitions
 *
 **********************************************************************/ 
static int R__lz4_compress(int cxlevel, uch* ibufptr, lzo_uint ibufsz,
    uch* obufptr, lzo_uintp obufsz)
{
  lzo_uint osz = *obufsz, minosz; 
  unsigned level = (cxlevel ? 1: 0);
  unsigned long adler32 = 0;
  *obufsz = 0;
  /* check source buffer size */
  if (ibufsz > 0xffffff) {
    R__error("source buffer too large");
    return -1;
  }
  /* calculate the buffer size needed for safe in place compression */
  minosz = LZ4_compressBound(ibufsz);
  if (0 == level) minosz = ibufsz;
  minosz += HDRSIZE + 4; /* header plus check sum */
  /* check buffer sizes */
  if (osz <= HDRSIZE + 4) {
    R__error("target buffer too small");
    return -1;
  }
  /* init header */
  obufptr[0] = 'L';
  obufptr[1] = '4';
  /* compress with specified level and algorithm */
  if (level > 0) {
    uch* obuf = obufptr + HDRSIZE;
    lzo_uint csz = LZ4_compress(ibufptr, obuf, ibufsz);
    /* check compression ratio */
    if (csz < ibufsz && 0 != level) {
	obufptr[2] = 1;
	*obufsz = csz + HDRSIZE + 4;
    } else {
      /* uncompressible, try to store uncompressed below */
      level = 0;
    }
  }
  /* check if we are to store uncompressed */
  if (0 == level) {
    /* check for sufficient space */
    minosz = ibufsz + HDRSIZE + 4;
    if (osz < minosz) {
      R__error("target buffer too small");
      return -1;
    }
    *obufsz = minosz;
    /* copy to output buffer (move, buffers might overlap) */
    memmove(obufptr + HDRSIZE, ibufptr, ibufsz);
    obufptr[2] = 0; /* store uncompressed */
  }
  /* fill in sizes */
  osz = *obufsz - HDRSIZE;
  obufptr[3] = (char)(osz & 0xff);	/* compressed size */
  obufptr[4] = (char)((osz >> 8) & 0xff);
  obufptr[5] = (char)((osz >> 16) & 0xff);

  obufptr[6] = (char)(ibufsz & 0xff);	/* decompressed size */
  obufptr[7] = (char)((ibufsz >> 8) & 0xff);
  obufptr[8] = (char)((ibufsz >> 16) & 0xff);
  /* calculate checksum */
  adler32 = lzo_adler32(
	  lzo_adler32(0, NULL,0), obufptr + HDRSIZE, osz - 4);
  obufptr += *obufsz - 4;
  obufptr[0] = (char) (adler32 & 0xff);
  obufptr[1] = (char) ((adler32 >> 8) & 0xff);
  obufptr[2] = (char) ((adler32 >> 16) & 0xff);
  obufptr[3] = (char) ((adler32 >> 24) & 0xff);

  return 0;
}

static int R__lz4_decompress(uch* ibufptr, long ibufsz,
    uch* obufptr, long* obufsz, uch method)
{
  lzo_uint osz = *obufsz;
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
  switch (method) {
    case 0: /* just store the uncompressed data */
      if (*obufsz != ibufsz - 4) return -1;
      if (ibufptr != obufptr)
	memmove(obufptr, ibufptr, ibufsz - 4);
      break;
    case 1: /* LZ4 */
      osz = LZ4_uncompress_unknownOutputSize(ibufptr, obufptr, ibufsz - 4, *obufsz);
      if (osz != *obufsz) return -1;
      break;
    default:
      /* unknown method */
      return -1;
  }
  return 0;
}


