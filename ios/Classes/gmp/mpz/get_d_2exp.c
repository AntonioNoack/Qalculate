/* double mpz_get_d_2exp (signed long int *exp, mpz_t src).

Copyright 2001, 2003, 2004, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"
#include "../gmp-impl.c"

double
mpz_get_d_2exp (signed long int *exp2, mpz_srcptr src)
{
  mp_size_t size, abs_size;
  mp_srcptr ptr;
  long exp;

  size = SIZ(src);
  if (UNLIKELY (size == 0))
    {
      *exp2 = 0;
      return 0.0;
    }

  ptr = PTR(src);
  abs_size = ABS(size);
  MPN_SIZEINBASE_2EXP(exp, ptr, abs_size, 1);
  *exp2 = exp;
  return mpn_get_d (ptr, abs_size, size, -exp);
}
