/* mpz_fdiv_qr -- Division rounding the quotient towards -infinity.
   The remainder gets the same sign as the denominator.

Copyright 1994-1996, 2000, 2001, 2005, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_fdiv_qr (mpz_ptr quot, mpz_ptr rem, mpz_srcptr dividend, mpz_srcptr divisor)
{
  mp_size_t divisor_size = SIZ (divisor);
  mp_size_t xsize;
  mpz_t temp_divisor;		/* N.B.: lives until function returns! */
  TMP_DECL;

  TMP_MARK;

  /* We need the original value of the divisor after the quotient and
     remainder have been preliminary calculated.  We have to copy it to
     temporary space if it's the same variable as either QUOT or REM.  */
  if (quot == divisor || rem == divisor)
    {
      MPZ_TMP_INIT (temp_divisor, ABS (divisor_size));
      mpz_set (temp_divisor, divisor);
      divisor = temp_divisor;
    }

  xsize = SIZ (dividend) ^ divisor_size;;
  mpz_tdiv_qr (quot, rem, dividend, divisor);

  if (xsize < 0 && SIZ (rem) != 0)
    {
      mpz_sub_ui (quot, quot, 1L);
      mpz_add (rem, rem, divisor);
    }

  TMP_FREE;
}
