/* mpz_limbs_modify -- Read-and-modify access to the mpn-style limb array.

Copyright 2013 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

mp_ptr
mpz_limbs_modify (mpz_ptr x, mp_size_t n)
{
  ASSERT (n > 0);
  return MPZ_REALLOC (x, n);
}
