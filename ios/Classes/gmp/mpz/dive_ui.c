/* mpz_divexact_ui -- exact division mpz by ulong.

Copyright 2001, 2002, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_divexact_ui (mpz_ptr dst, mpz_srcptr src, unsigned long divisor)
{
  mp_size_t  size, abs_size;
  mp_ptr     dst_ptr;

  if (UNLIKELY (divisor == 0))
    DIVIDE_BY_ZERO;

  /* For nails don't try to be clever if d is bigger than a limb, just fake
     up an mpz_t and go to the main mpz_divexact.  */
  if (divisor > GMP_NUMB_MAX)
    {
      mp_limb_t  dlimbs[2];
      mpz_t      dz;
      ALLOC(dz) = 2;
      PTR(dz) = dlimbs;
      mpz_set_ui (dz, divisor);
      mpz_divexact (dst, src, dz);
      return;
    }

  size = SIZ(src);
  if (size == 0)
    {
      SIZ(dst) = 0;
      return;
    }
  abs_size = ABS (size);

  dst_ptr = MPZ_REALLOC (dst, abs_size);

  MPN_DIVREM_OR_DIVEXACT_1 (dst_ptr, PTR(src), abs_size, (mp_limb_t) divisor);
  abs_size -= (dst_ptr[abs_size-1] == 0);
  SIZ(dst) = (size >= 0 ? abs_size : -abs_size);
}
