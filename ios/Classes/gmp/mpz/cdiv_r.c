/* mpz_cdiv_r -- Division rounding the quotient towards +infinity.  The
   remainder gets the opposite sign as the denominator.

Copyright 1994-1996, 2001, 2005, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_cdiv_r (mpz_ptr rem, mpz_srcptr dividend, mpz_srcptr divisor)
{
  mp_size_t divisor_size = SIZ (divisor);
  mpz_t temp_divisor;		/* N.B.: lives until function returns! */
  TMP_DECL;

  TMP_MARK;

  /* We need the original value of the divisor after the remainder has been
     preliminary calculated.  We have to copy it to temporary space if it's
     the same variable as REM.  */
  if (rem == divisor)
    {

      MPZ_TMP_INIT (temp_divisor, ABS (divisor_size));
      mpz_set (temp_divisor, divisor);
      divisor = temp_divisor;
    }

  mpz_tdiv_r (rem, dividend, divisor);

  if ((divisor_size ^ SIZ (dividend)) >= 0 && SIZ (rem) != 0)
    mpz_sub (rem, rem, divisor);

  TMP_FREE;
}
