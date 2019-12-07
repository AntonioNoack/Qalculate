/* double mpf_get_d_2exp (signed long int *exp, mpf_t src).

Copyright 2001-2004 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"


double
mpf_get_d_2exp (signed long int *exp2, mpf_srcptr src)
{
  mp_size_t size, abs_size;
  mp_srcptr ptr;
  int cnt;
  long exp;

  size = SIZ(src);
  if (UNLIKELY (size == 0))
    {
      *exp2 = 0;
      return 0.0;
    }

  ptr = PTR(src);
  abs_size = ABS (size);
  count_leading_zeros (cnt, ptr[abs_size - 1]);
  cnt -= GMP_NAIL_BITS;

  exp = EXP(src) * GMP_NUMB_BITS - cnt;
  *exp2 = exp;

  return mpn_get_d (ptr, abs_size, (mp_size_t) 0,
                    (long) - (abs_size * GMP_NUMB_BITS - cnt));
}
