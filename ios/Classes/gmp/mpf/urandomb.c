/* mpf_urandomb (rop, state, nbits) -- Generate a uniform pseudorandom
   real number between 0 (inclusive) and 1 (exclusive) of size NBITS,
   using STATE as the random state previously initialized by a call to
   gmp_randinit().

Copyright 1999-2002 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpf_urandomb (mpf_t rop, gmp_randstate_t rstate, mp_bitcnt_t nbits)
{
  mp_ptr rp;
  mp_size_t nlimbs;
  mp_exp_t exp;
  mp_size_t prec;

  rp = PTR (rop);
  nlimbs = BITS_TO_LIMBS (nbits);
  prec = PREC (rop);

  if (nlimbs > prec + 1 || nlimbs == 0)
    {
      nlimbs = prec + 1;
      nbits = nlimbs * GMP_NUMB_BITS;
    }

  _gmp_rand (rp, rstate, nbits);

  /* If nbits isn't a multiple of GMP_NUMB_BITS, shift up.  */
  if (nbits % GMP_NUMB_BITS != 0)
    mpn_lshift (rp, rp, nlimbs, GMP_NUMB_BITS - nbits % GMP_NUMB_BITS);

  exp = 0;
  while (nlimbs != 0 && rp[nlimbs - 1] == 0)
    {
      nlimbs--;
      exp--;
    }
  EXP (rop) = exp;
  SIZ (rop) = nlimbs;
}
