/* mpz_roinit_n -- Initialize mpz with read-only limb array.

Copyright 2013 Free Software Foundation, Inc.
  */

#ifndef ROINIT_N_C
#define ROINIT_N_C

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

mpz_srcptr
mpz_roinit_n (mpz_ptr x, mp_srcptr xp, mp_size_t xs)
{
  mp_size_t xn = ABS(xs);
  MPN_NORMALIZE (xp, xn);

  ALLOC (x) = 0;
  SIZ (x) = xs < 0 ? -xn : xn;
  PTR (x) = (mp_ptr) xp;
  return x;
}

#endif