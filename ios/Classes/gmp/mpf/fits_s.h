/* mpf_fits_s*_p -- test whether an mpf fits a C signed type.

Copyright 2001, 2002, 2014 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"


/* Notice this is equivalent to mpz_set_f + mpz_fits_s*_p.  */

int
FUNCTION (mpf_srcptr f) __GMP_NOTHROW
{
  mp_size_t  fs, fn;
  mp_srcptr  fp;
  mp_exp_t   exp;
  mp_limb_t  fl;

  exp = EXP(f);
  if (exp < 1)
    return 1;  /* -1 < f < 1 truncates to zero, so fits */

  fs = SIZ (f);
  fp = PTR(f);
  fn = ABS (fs);

  if (exp == 1)
    {
      fl = fp[fn-1];
    }
#if GMP_NAIL_BITS != 0
  else if (exp == 2 && MAXIMUM > GMP_NUMB_MAX)
    {
      fl = fp[fn-1];
      if ((fl >> GMP_NAIL_BITS) != 0)
	return 0;
      fl = (fl << GMP_NUMB_BITS);
      if (fn >= 2)
        fl |= fp[fn-2];
    }
#endif
  else
    return 0;

  return fl <= (fs >= 0 ? (mp_limb_t) MAXIMUM : - (mp_limb_t) MINIMUM);
}
