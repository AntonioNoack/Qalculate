/* mpz_clear -- de-allocate the space occupied by the dynamic digit space of
   an integer.

Copyright 1991, 1993-1995, 2000, 2001, 2012, 2014, 2015 Free Software
Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_clear (mpz_ptr x)
{
  __GMP_FREE_FUNC_LIMBS (PTR (x), ALLOC(x));
}
