/* jacobi.c

   THE FUNCTIONS IN THIS FILE ARE INTERNAL WITH MUTABLE INTERFACES.  IT IS ONLY
   SAFE TO REACH THEM THROUGH DOCUMENTED INTERFACES.  IN FACT, IT IS ALMOST
   GUARANTEED THAT THEY'LL CHANGE OR DISAPPEAR IN A FUTURE GNU MP RELEASE.

Copyright 1996, 1998, 2000-2004, 2008, 2010, 2011 Free Software Foundation,
Inc.
  */

#ifndef JACOBI_XXX_H
#define JACOBI_XXX_H

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"
#include "../../struct_helper/hgcd_matrix.h"

#ifndef JACOBI_DC_THRESHOLD
#define JACOBI_DC_THRESHOLD GCD_DC_THRESHOLD
#endif

/* Schönhage's rules:
 *
 * Assume r0 = r1 q1 + r2, with r0 odd, and r1 = q2 r2 + r3
 *
 * If r1 is odd, then
 *
 *   (r1 | r0) = s(r1, r0) (r0 | r1) = s(r1, r0) (r2, r1)
 *
 * where s(x,y) = (-1)^{(x-1)(y-1)/4} = (-1)^[x = y = 3 (mod 4)].
 *
 * If r1 is even, r2 must be odd. We have
 *
 *   (r1 | r0) = (r1 - r0 | r0) = (-1)^(r0-1)/2 (r0 - r1 | r0)
 *             = (-1)^(r0-1)/2 s(r0, r0 - r1) (r0 | r0 - r1)
 *             = (-1)^(r0-1)/2 s(r0, r0 - r1) (r1 | r0 - r1)
 *
 * Now, if r1 = 0 (mod 4), then the sign factor is +1, and repeating
 * q1 times gives
 *
 *   (r1 | r0) = (r1 | r2) = (r3 | r2)
 *
 * On the other hand, if r1 = 2 (mod 4), the sign factor is
 * (-1)^{(r0-1)/2}, and repeating q1 times gives the exponent
 *
 *   (r0-1)/2 + (r0-r1-1)/2 + ... + (r0 - (q1-1) r1)/2
 *   = q1 (r0-1)/2 + q1 (q1-1)/2
 *
 * and we can summarize the even case as
 *
 *   (r1 | r0) = t(r1, r0, q1) (r3 | r2)
 *
 * where t(x,y,q) = (-1)^{[x = 2 (mod 4)] (q(y-1)/2 + y(q-1)/2)}
 *
 * What about termination? The remainder sequence ends with (0|1) = 1
 * (or (0 | r) = 0 if r != 1). What are the possible cases? If r1 is
 * odd, r2 may be zero. If r1 is even, then r2 = r0 - q1 r1 is odd and
 * hence non-zero. We may have r3 = r1 - q2 r2 = 0.
 *
 * Examples: (11|15) = - (15|11) = - (4|11)
 *            (4|11) =    (4| 3) =   (1| 3)
 *            (1| 3) = (3|1) = (0|1) = 1
 *
 *             (2|7) = (2|1) = (0|1) = 1
 *
 * Detail:     (2|7) = (2-7|7) = (-1|7)(5|7) = -(7|5) = -(2|5)
 *             (2|5) = (2-5|5) = (-1|5)(3|5) =  (5|3) =  (2|3)
 *             (2|3) = (2-3|3) = (-1|3)(1|3) = -(3|1) = -(2|1)
 *
 */

/* In principle, the state consists of four variables: e (one bit), a,
   b (two bits each), d (one bit). Collected factors are (-1)^e. a and
   b are the least significant bits of the current remainders. d
   (denominator) is 0 if we're currently subtracting multiplies of a
   from b, and 1 if we're subtracting b from a.

   e is stored in the least significant bit, while a, b and d are
   coded as only 13 distinct values in bits 1-4, according to the
   following table. For rows not mentioning d, the value is either
   implied, or it doesn't matter. */

#if WANT_ASSERT
static const struct
{
  unsigned char a;
  unsigned char b;
} decode_table[13] = {
  /*  0 */ { 0, 1 },
  /*  1 */ { 0, 3 },
  /*  2 */ { 1, 1 },
  /*  3 */ { 1, 3 },
  /*  4 */ { 2, 1 },
  /*  5 */ { 2, 3 },
  /*  6 */ { 3, 1 },
  /*  7 */ { 3, 3 }, /* d = 1 */
  /*  8 */ { 1, 0 },
  /*  9 */ { 1, 2 },
  /* 10 */ { 3, 0 },
  /* 11 */ { 3, 2 },
  /* 12 */ { 3, 3 }, /* d = 0 */
};
#define JACOBI_A(bits) (decode_table[(bits)>>1].a)
#define JACOBI_B(bits) (decode_table[(bits)>>1].b)
#endif /* WANT_ASSERT */

const unsigned char jacobi_table[208] = {
#include "jacobitab.h"
};

#define BITS_FAIL 31

static void
jacobi_hook (void *p, mp_srcptr gp, mp_size_t gn,
	     mp_srcptr qp, mp_size_t qn, int d);

#define CHOOSE_P(n) (2*(n) / 3)

int
mpn_jacobi_n (mp_ptr ap, mp_ptr bp, mp_size_t n, unsigned bits);

#endif
