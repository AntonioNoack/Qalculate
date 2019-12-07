/* mpz_init_set_d(integer, val) -- Initialize and assign INTEGER with a double
   value VAL.

Copyright 1996, 2000, 2001, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_init_set_d (mpz_ptr dest, double val)
{
  ALLOC (dest) = 1;
  PTR (dest) = __GMP_ALLOCATE_FUNC_LIMBS (1);
  SIZ (dest) = 0;
  mpz_set_d (dest, val);
}
