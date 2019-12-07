/* mpz_tdiv_q -- divide two integers and produce a quotient.

Copyright 1991, 1993, 1994, 1996, 2000, 2001, 2005, 2010, 2012 Free Software
Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"
#include "../gmp-impl.c"

void
mpz_tdiv_q (mpz_ptr quot, mpz_srcptr num, mpz_srcptr den)
{
  mp_size_t ql;
  mp_size_t ns, ds, nl, dl;
  mp_ptr np, dp, qp;
  TMP_DECL;

  ns = SIZ (num);
  ds = SIZ (den);
  nl = ABS (ns);
  dl = ABS (ds);
  ql = nl - dl + 1;

  if (UNLIKELY (dl == 0))
    DIVIDE_BY_ZERO;

  if (ql <= 0)
    {
      SIZ (quot) = 0;
      return;
    }

  qp = MPZ_REALLOC (quot, ql);

  TMP_MARK;
  np = PTR (num);
  dp = PTR (den);

  /* Copy denominator to temporary space if it overlaps with the quotient.  */
  if (dp == qp)
    {
      mp_ptr tp;
      tp = TMP_ALLOC_LIMBS (dl);
      MPN_COPY (tp, dp, dl);
      dp = tp;
    }
  /* Copy numerator to temporary space if it overlaps with the quotient.  */
  if (np == qp)
    {
      mp_ptr tp;
      tp = TMP_ALLOC_LIMBS (nl + 1);
      MPN_COPY (tp, np, nl);
      /* Overlap dividend and scratch.  */
      mpn_div_q (qp, tp, nl, dp, dl, tp);
    }
  else
    {
      mp_ptr tp;
      tp = TMP_ALLOC_LIMBS (nl + 1);
      mpn_div_q (qp, np, nl, dp, dl, tp);
    }

  ql -=  qp[ql - 1] == 0;

  SIZ (quot) = (ns ^ ds) >= 0 ? ql : -ql;
  TMP_FREE;
}
