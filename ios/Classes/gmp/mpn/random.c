/* mpn_random -- Generate random numbers.

Copyright 2001, 2002 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpn_random (mp_ptr ptr, mp_size_t size)
{
  gmp_randstate_ptr  rands;

  /* FIXME: Is size==0 supposed to be allowed? */
  ASSERT (size >= 0);

  if (size == 0)
    return;

  rands = RANDS;
  _gmp_rand (ptr, rands, size * GMP_NUMB_BITS);

  /* Make sure the most significant limb is non-zero.  */
  while (ptr[size-1] == 0)
    _gmp_rand (&ptr[size-1], rands, GMP_NUMB_BITS);
}
