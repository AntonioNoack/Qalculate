/* mpq_clear -- free the space occupied by an mpq_t.

Copyright 1991, 1994, 1995, 2000, 2001, 2015 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

void
mpq_clear (mpq_t x)
{
  __GMP_FREE_FUNC_LIMBS (PTR(NUM(x)), ALLOC(NUM(x)));
  __GMP_FREE_FUNC_LIMBS (PTR(DEN(x)), ALLOC(DEN(x)));
}
