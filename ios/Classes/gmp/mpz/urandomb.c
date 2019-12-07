/* mpz_urandomb (rop, state, n) -- Generate a uniform pseudorandom
   integer in the range 0 to 2^N - 1, inclusive, using STATE as the
   random state previously initialized by a call to gmp_randinit().

Copyright 1999, 2000, 2002 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_urandomb (mpz_ptr rop, gmp_randstate_t rstate, mp_bitcnt_t nbits)
{
  mp_ptr rp;
  mp_size_t size;

  size = BITS_TO_LIMBS (nbits);
  rp = MPZ_NEWALLOC (rop, size);

  _gmp_rand (rp, rstate, nbits);
  MPN_NORMALIZE (rp, size);
  SIZ (rop) = size;
}
