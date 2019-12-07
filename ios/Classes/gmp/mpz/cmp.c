/* mpz_cmp(u,v) -- Compare U, V.  Return positive, zero, or negative
   based on if U > V, U == V, or U < V.

Copyright 1991, 1993, 1994, 1996, 2001, 2002, 2011 Free Software Foundation,
Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

int
mpz_cmp (mpz_srcptr u, mpz_srcptr v) __GMP_NOTHROW
{
  mp_size_t  usize, vsize, dsize, asize;
  mp_srcptr  up, vp;
  int        cmp;

  usize = SIZ(u);
  vsize = SIZ(v);
  dsize = usize - vsize;
  if (dsize != 0)
    return dsize;

  asize = ABS (usize);
  up = PTR(u);
  vp = PTR(v);
  MPN_CMP (cmp, up, vp, asize);
  return (usize >= 0 ? cmp : -cmp);
}
