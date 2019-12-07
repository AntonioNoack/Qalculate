/* double mpf_get_d (mpf_t src) -- return SRC truncated to a double.

Copyright 1996, 2001-2004 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

double
mpf_get_d (mpf_srcptr src)
{
  mp_size_t  size, abs_size;
  long       exp;

  size = SIZ (src);
  if (UNLIKELY (size == 0))
    return 0.0;

  abs_size = ABS (size);
  exp = (EXP (src) - abs_size) * GMP_NUMB_BITS;
  return mpn_get_d (PTR (src), abs_size, size, exp);
}
