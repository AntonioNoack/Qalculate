/* mpf_random2 -- Generate a positive random mpf_t of specified size, with
   long runs of consecutive ones and zeros in the binary representation.
   Intended for testing of other MP routines.

Copyright 1995, 1996, 2001-2003 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"


void mpf_random2 (mpf_ptr x, mp_size_t xs, mp_exp_t exp){
  mp_size_t xn;
  mp_size_t prec;
  mp_limb_t elimb;

  xn = ABS (xs);
  prec = PREC(x);

  if (xn == 0)
    {
      EXP(x) = 0;
      SIZ(x) = 0;
      return;
    }

  if (xn > prec + 1)
    xn = prec + 1;

  /* General random mantissa.  */
  mpn_random2 (PTR(x), xn);

  /* Generate random exponent.  */
  _gmp_rand (&elimb, RANDS, GMP_NUMB_BITS);
  exp = ABS (exp);
  exp = elimb % (2 * exp + 1) - exp;

  EXP(x) = exp;
  SIZ(x) = xs < 0 ? -xn : xn;
}
