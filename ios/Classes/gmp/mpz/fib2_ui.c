/* mpz_fib2_ui -- calculate Fibonacci numbers.

Copyright 2001, 2012, 2014 Free Software Foundation, Inc.
  */

#include <stdio.h>
#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_fib2_ui (mpz_ptr fn, mpz_ptr fnsub1, unsigned long n)
{
  mp_ptr     fp, f1p;
  mp_size_t  size;

  if (n <= FIB_TABLE_LIMIT)
    {
      PTR(fn)[0] = FIB_TABLE (n);
      SIZ(fn) = (n != 0);      /* F[0]==0, others are !=0 */
      PTR(fnsub1)[0] = FIB_TABLE ((int) n - 1);
      SIZ(fnsub1) = (n != 1);  /* F[1-1]==0, others are !=0 */
      return;
    }

  size = MPN_FIB2_SIZE (n);
  fp =  MPZ_NEWALLOC (fn,     size);
  f1p = MPZ_NEWALLOC (fnsub1, size);

  size = mpn_fib2_ui (fp, f1p, n);

  SIZ(fn)     = size;
  SIZ(fnsub1) = size - (f1p[size-1] == 0);
}
