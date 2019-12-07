/* mpz_gcd_ui -- Calculate the greatest common divisor of two integers.

Copyright 1994, 1996, 1999-2004 Free Software Foundation, Inc.
  */

#include <stdio.h> /* for NULL */
#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

unsigned long int
mpz_gcd_ui (mpz_ptr w, mpz_srcptr u, unsigned long int v)
{
  mp_size_t un;
  mp_limb_t res;

#if BITS_PER_ULONG > GMP_NUMB_BITS  /* avoid warnings about shift amount */
  if (v > GMP_NUMB_MAX)
    {
      mpz_t vz;
      mp_limb_t vlimbs[2];
      vlimbs[0] = v & GMP_NUMB_MASK;
      vlimbs[1] = v >> GMP_NUMB_BITS;
      PTR(vz) = vlimbs;
      SIZ(vz) = 2;
      mpz_gcd (w, u, vz);
      /* because v!=0 we will have w<=v hence fitting a ulong */
      ASSERT (mpz_fits_ulong_p (w));
      return mpz_get_ui (w);
    }
#endif

  un = ABSIZ(u);

  if (un == 0)
    res = v;
  else if (v == 0)
    {
      if (w != NULL)
	{
	  if (u != w)
	    {
	      MPZ_REALLOC (w, un);
	      MPN_COPY (PTR(w), PTR(u), un);
	    }
	  SIZ(w) = un;
	}
      /* Return u if it fits a ulong, otherwise 0. */
      res = PTR(u)[0];
      return (un == 1 && res <= ULONG_MAX ? res : 0);
    }
  else
    res = mpn_gcd_1 (PTR(u), un, (mp_limb_t) v);

  if (w != NULL)
    {
      PTR(w)[0] = res;
      SIZ(w) = res != 0;
    }
  return res;
}