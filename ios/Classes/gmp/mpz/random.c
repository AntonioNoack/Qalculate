/* mpz_random -- Generate a random mpz_t of specified size in limbs.

Copyright 2001, 2002 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_random (mpz_ptr x, mp_size_t size)
{
  mpz_urandomb (x, RANDS, (unsigned long) (ABS (size) * GMP_NUMB_BITS));
  if (size < 0)
    SIZ(x) = -SIZ(x);
}
