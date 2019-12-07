/* mpz_init() -- Make a new multiple precision number with value 0.

Copyright 1991, 1993-1995, 2000-2002, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_init (mpz_ptr x)
{
  ALLOC (x) = 1;
  PTR (x) = __GMP_ALLOCATE_FUNC_LIMBS (1);
  SIZ (x) = 0;

#ifdef __CHECKER__
  /* let the low limb look initialized, for the benefit of mpz_get_ui etc */
  PTR (x)[0] = 0;
#endif
}
