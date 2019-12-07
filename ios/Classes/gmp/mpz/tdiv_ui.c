/* mpz_tdiv_ui(dividend, divisor_limb) -- Return DIVDEND mod DIVISOR_LIMB.

Copyright 1991, 1993, 1994, 1996-1998, 2001, 2002, 2004, 2005, 2012 Free
Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"
#include "../gmp-impl.c"

unsigned long int
mpz_tdiv_ui (mpz_srcptr dividend, unsigned long int divisor)
{
  mp_size_t ns, nn;
  mp_ptr np;
  mp_limb_t rl;

  if (UNLIKELY (divisor == 0))
    DIVIDE_BY_ZERO;

  ns = SIZ(dividend);
  if (ns == 0)
    {
      return 0;
    }

  nn = ABS(ns);
  np = PTR(dividend);

#if BITS_PER_ULONG > GMP_NUMB_BITS  /* avoid warnings about shift amount */
  if (divisor > GMP_NUMB_MAX)
    {
      mp_limb_t dp[2], rp[2];
      mp_ptr qp;
      mp_size_t rn;
      TMP_DECL;

      if (nn == 1)		/* tdiv_qr requirements; tested above for 0 */
	{
	  rl = np[0];
	  return rl;
	}

      TMP_MARK;
      dp[0] = divisor & GMP_NUMB_MASK;
      dp[1] = divisor >> GMP_NUMB_BITS;
      qp = TMP_ALLOC_LIMBS (nn - 2 + 1);
      mpn_tdiv_qr (qp, rp, (mp_size_t) 0, np, nn, dp, (mp_size_t) 2);
      TMP_FREE;
      rl = rp[0] + (rp[1] << GMP_NUMB_BITS);
      rn = 2 - (rp[1] == 0);  rn -= (rp[rn - 1] == 0);
    }
  else
#endif
    {
      rl = mpn_mod_1 (np, nn, (mp_limb_t) divisor);
    }

  return rl;
}
