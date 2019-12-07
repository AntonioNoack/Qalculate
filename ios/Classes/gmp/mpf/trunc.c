/* mpf_trunc -- truncate an mpf to an integer.

Copyright 2001 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"


/* Notice the use of prec+1 ensures mpf_trunc is equivalent to mpf_set if u
   is already an integer.  */

void
mpf_trunc (mpf_ptr r, mpf_srcptr u)
{
  mp_ptr     rp;
  mp_srcptr  up;
  mp_size_t  size, asize, prec;
  mp_exp_t   exp;

  exp = EXP(u);
  size = SIZ(u);
  if (size == 0 || exp <= 0)
    {
      /* u is only a fraction */
      SIZ(r) = 0;
      EXP(r) = 0;
      return;
    }

  up = PTR(u);
  EXP(r) = exp;
  asize = ABS (size);
  up += asize;

  /* skip fraction part of u */
  asize = MIN (asize, exp);

  /* don't lose precision in the copy */
  prec = PREC(r) + 1;

  /* skip excess over target precision */
  asize = MIN (asize, prec);

  up -= asize;
  rp = PTR(r);
  SIZ(r) = (size >= 0 ? asize : -asize);
  if (rp != up)
    MPN_COPY_INCR (rp, up, asize);
}
