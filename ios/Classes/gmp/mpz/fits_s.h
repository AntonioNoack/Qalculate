/* int mpz_fits_X_p (mpz_t z) -- test whether z fits signed type X.

Copyright 1997, 2000-2002 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

#include "../gmp-impl.c"


int
FUNCTION (mpz_srcptr z) __GMP_NOTHROW
{
  mp_size_t n = SIZ(z);
  mp_ptr p = PTR(z);
  mp_limb_t limb = p[0];

  if (n == 0)
    return 1;
  if (n == 1)
    return limb <= MAXIMUM;
  if (n == -1)
    return limb <= NEG_CAST (mp_limb_t, MINIMUM);
#if GMP_NAIL_BITS != 0
  {
    if ((p[1] >> GMP_NAIL_BITS) == 0)
      {
	limb += p[1] << GMP_NUMB_BITS;
	if (n == 2)
	  return limb <= MAXIMUM;
	if (n == -2)
	  return limb <= NEG_CAST (mp_limb_t, MINIMUM);
      }
  }
#endif
  return 0;
}
