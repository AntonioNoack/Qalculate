/* double mpz_get_d (mpz_t src) -- Return the double approximation to SRC.

Copyright 1996, 1997, 2000-2003 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

double
mpz_get_d (mpz_srcptr z)
{
  mp_size_t size;

  size = SIZ (z);
  if (UNLIKELY (size == 0))
    return 0.0;

  return mpn_get_d (PTR (z), ABS (size), size, 0L);
}
