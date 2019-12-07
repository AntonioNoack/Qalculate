/* mpz_finish_limbs -- Update mpz after writing to the limb array.

Copyright 2013 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_limbs_finish (mpz_ptr x, mp_size_t xs)
{
  mp_size_t xn = ABS(xs);
  MPN_NORMALIZE (PTR (x), xn);
  SIZ (x) = xs < 0 ? -xn : xn;
}
