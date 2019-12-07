/* mpf_clear -- de-allocate the space occupied by the dynamic digit space of
   an integer.

Copyright 1993-1995, 2000, 2001 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

void
mpf_clear (mpf_ptr x)
{
  __GMP_FREE_FUNC_LIMBS (PTR(x), PREC(x) + 1);
}
