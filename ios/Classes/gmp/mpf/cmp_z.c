/* mpf_cmp_z -- Compare a float with an integer.

Copyright 2015 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

int
mpf_cmp_z (mpf_srcptr u, mpz_srcptr v) __GMP_NOTHROW
{
  mpf_t vf;
  mp_size_t size;

  SIZ (vf) = size = SIZ (v);
  EXP (vf) = size = ABS (size);
  /* PREC (vf) = size; */ 
  PTR (vf) = PTR (v);

  return mpf_cmp (u, vf);
}
