/* mpz_mod -- The mathematical mod function.

Copyright 1991, 1993-1996, 2001, 2002, 2005, 2010, 2012 Free Software
Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_mod (mpz_ptr rem, mpz_srcptr dividend, mpz_srcptr divisor)
{
  mp_size_t rn, bn;
  mpz_t temp_divisor;
  TMP_DECL;

  TMP_MARK;

  bn = ABSIZ(divisor);

  /* We need the original value of the divisor after the remainder has been
     preliminary calculated.  We have to copy it to temporary space if it's
     the same variable as REM.  */
  if (rem == divisor)
    {
      PTR(temp_divisor) = TMP_ALLOC_LIMBS (bn);
      MPN_COPY (PTR(temp_divisor), PTR(divisor), bn);
    }
  else
    {
      PTR(temp_divisor) = PTR(divisor);
    }
  SIZ(temp_divisor) = bn;
  divisor = temp_divisor;

  mpz_tdiv_r (rem, dividend, divisor);

  rn = SIZ (rem);
  if (rn < 0)
    mpz_add (rem, rem, divisor);

  TMP_FREE;
}
