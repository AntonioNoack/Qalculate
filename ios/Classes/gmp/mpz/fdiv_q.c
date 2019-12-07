/* mpz_fdiv_q -- Division rounding the quotient towards -infinity.
   The remainder gets the same sign as the denominator.

Copyright 1994-1996, 2000, 2001, 2005, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_fdiv_q (mpz_ptr quot, mpz_srcptr dividend, mpz_srcptr divisor)
{
  mp_size_t dividend_size = SIZ (dividend);
  mp_size_t divisor_size = SIZ (divisor);
  mpz_t rem;
  TMP_DECL;

  TMP_MARK;

  MPZ_TMP_INIT (rem, ABS (divisor_size));

  mpz_tdiv_qr (quot, rem, dividend, divisor);

  if ((divisor_size ^ dividend_size) < 0 && SIZ (rem) != 0)
    mpz_sub_ui (quot, quot, 1L);

  TMP_FREE;
}
