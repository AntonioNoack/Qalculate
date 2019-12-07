/* mpz_divexact -- finds quotient when known that quot * den == num && den != 0.

Contributed to the GNU project by Niels Möller.

Copyright 1991, 1993-1998, 2000-2002, 2005-2007, 2009, 2012 Free Software
Foundation, Inc.
  */


#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"
#include "../gmp-impl.c"

void
mpz_divexact (mpz_ptr quot, mpz_srcptr num, mpz_srcptr den)
{
  mp_ptr qp;
  mp_size_t qn;
  mp_srcptr np, dp;
  mp_size_t nn, dn;
  TMP_DECL;

#if WANT_ASSERT
  {
    mpz_t  rem;
    mpz_init (rem);
    mpz_tdiv_r (rem, num, den);
    ASSERT (SIZ(rem) == 0);
    mpz_clear (rem);
  }
#endif

  nn = ABSIZ (num);
  dn = ABSIZ (den);

  if (nn < dn)
    {
      /* This special case avoids segfaults below when the function is
	 incorrectly called with |N| < |D|, N != 0.  It also handles the
	 well-defined case N = 0.  */
      SIZ(quot) = 0;
      return;
    }

  qn = nn - dn + 1;

  TMP_MARK;

  if (quot == num || quot == den)
    qp = TMP_ALLOC_LIMBS (qn);
  else
    qp = MPZ_REALLOC (quot, qn);

  np = PTR(num);
  dp = PTR(den);

  mpn_divexact (qp, np, nn, dp, dn);
  MPN_NORMALIZE (qp, qn);

  if (qp != PTR(quot))
    MPN_COPY (MPZ_REALLOC (quot, qn), qp, qn);

  SIZ(quot) = (SIZ(num) ^ SIZ(den)) >= 0 ? qn : -qn;

  TMP_FREE;
}
