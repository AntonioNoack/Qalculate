/* mpf_size(x) -- return the number of limbs currently used by the
   value of the float X.

Copyright 1993-1995, 2001 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

size_t
mpf_size (mpf_srcptr f) __GMP_NOTHROW
{
  return __GMP_ABS (f->_mp_size);
}
