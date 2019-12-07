/* mpq_init -- Make a new rational number with value 0/1.

Copyright 1991, 1994, 1995, 2000-2002 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

void
mpq_init (mpq_t x)
{
  ALLOC(NUM(x)) = 1;
  PTR(NUM(x)) = __GMP_ALLOCATE_FUNC_LIMBS (1);
  SIZ(NUM(x)) = 0;
  ALLOC(DEN(x)) = 1;
  PTR(DEN(x)) = __GMP_ALLOCATE_FUNC_LIMBS (1);
  PTR(DEN(x))[0] = 1;
  SIZ(DEN(x)) = 1;

#ifdef __CHECKER__
  /* let the low limb look initialized, for the benefit of mpz_get_ui etc */
  PTR(NUM(x))[0] = 0;
#endif
}
