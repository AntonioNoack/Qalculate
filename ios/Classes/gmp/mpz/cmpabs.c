/* mpz_cmpabs(u,v) -- Compare U, V.  Return positive, zero, or negative
   based on if U > V, U == V, or U < V.

Copyright 1991, 1993, 1994, 1996, 1997, 2000-2002 Free Software Foundation,
Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"


int
mpz_cmpabs (mpz_srcptr u, mpz_srcptr v) __GMP_NOTHROW
{
  mp_size_t  usize, vsize, dsize;
  mp_srcptr  up, vp;
  int        cmp;

  usize = ABSIZ (u);
  vsize = ABSIZ (v);
  dsize = usize - vsize;
  if (dsize != 0)
    return dsize;

  up = PTR(u);
  vp = PTR(v);
  MPN_CMP (cmp, up, vp, usize);
  return cmp;
}
