/* mpz_divisible_p -- mpz by mpz divisibility test

Copyright 2001, 2002 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

int
mpz_divisible_p (mpz_srcptr a, mpz_srcptr d)
{
  mp_size_t dsize = SIZ(d);
  mp_size_t asize = SIZ(a);

  if (UNLIKELY (dsize == 0))
    return (asize == 0);

  return mpn_divisible_p (PTR(a), ABS(asize), PTR(d), ABS(dsize));
}
