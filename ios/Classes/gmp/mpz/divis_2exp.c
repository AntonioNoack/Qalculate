/* mpz_divisible_2exp_p -- mpz by 2^n divisibility test

Copyright 2001, 2002 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"


int
mpz_divisible_2exp_p (mpz_srcptr a, mp_bitcnt_t d) __GMP_NOTHROW
{
  mp_size_t      i, dlimbs;
  unsigned       dbits;
  mp_ptr         ap;
  mp_limb_t      dmask;
  mp_size_t      asize;

  asize = ABSIZ(a);
  dlimbs = d / GMP_NUMB_BITS;

  /* if d covers the whole of a, then only a==0 is divisible */
  if (asize <= dlimbs)
    return asize == 0;

  /* whole limbs must be zero */
  ap = PTR(a);
  for (i = 0; i < dlimbs; i++)
    if (ap[i] != 0)
      return 0;

  /* left over bits must be zero */
  dbits = d % GMP_NUMB_BITS;
  dmask = (CNST_LIMB(1) << dbits) - 1;
  return (ap[dlimbs] & dmask) == 0;
}
