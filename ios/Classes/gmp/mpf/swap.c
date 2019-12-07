/* mpf_swap (U, V) -- Swap U and V.

Copyright 1997, 1998, 2000, 2001, 2013 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpf_swap (mpf_ptr u, mpf_ptr v) __GMP_NOTHROW
{
  mp_ptr tptr;
  mp_size_t tprec;
  mp_size_t tsiz;
  mp_exp_t  texp;

  tprec = PREC(u);
  PREC(u) = PREC(v);
  PREC(v) = tprec;

  tsiz = SIZ(u);
  SIZ(u) = SIZ(v);
  SIZ(v) = tsiz;

  texp = EXP(u);
  EXP(u) = EXP(v);
  EXP(v) = texp;

  tptr = PTR(u);
  PTR(u) = PTR(v);
  PTR(v) = tptr;
}
