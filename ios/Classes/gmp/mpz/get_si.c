/* mpz_get_si(integer) -- Return the least significant digit from INTEGER.

Copyright 1991, 1993-1995, 2000-2002, 2006, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

signed long int
mpz_get_si (mpz_srcptr z) __GMP_NOTHROW
{
  mp_ptr zp = PTR (z);
  mp_size_t size = SIZ (z);
  mp_limb_t zl = zp[0];

#if GMP_NAIL_BITS != 0
  if (ULONG_MAX > GMP_NUMB_MAX && ABS (size) >= 2)
    zl |= zp[1] << GMP_NUMB_BITS;
#endif

  if (size > 0)
    return zl & LONG_MAX;
  else if (size < 0)
    /* This expression is necessary to properly handle 0x80000000 */
    return -1 - (long) ((zl - 1) & LONG_MAX);
  else
    return 0;
}
