/* mpz_mul_2exp -- Multiply a bignum by 2**CNT

Copyright 1991, 1993, 1994, 1996, 2001, 2002, 2012 Free Software Foundation,
Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_mul_2exp (mpz_ptr r, mpz_srcptr u, mp_bitcnt_t cnt)
{
  mp_size_t un, rn;
  mp_size_t limb_cnt;
  mp_ptr rp;
  mp_srcptr up;
  mp_limb_t rlimb;

  un = ABSIZ (u);
  limb_cnt = cnt / GMP_NUMB_BITS;
  rn = un + limb_cnt;

  if (un == 0)
    rn = 0;
  else
    {
      rp = MPZ_REALLOC (r, rn + 1);
      up = PTR(u);

      cnt %= GMP_NUMB_BITS;
      if (cnt != 0)
	{
	  rlimb = mpn_lshift (rp + limb_cnt, up, un, cnt);
	  rp[rn] = rlimb;
	  rn += (rlimb != 0);
	}
      else
	{
	  MPN_COPY_DECR (rp + limb_cnt, up, un);
	}

      /* Zero all whole limbs at low end.  Do it here and not before calling
	 mpn_lshift, not to lose for U == R.  */
      MPN_ZERO (rp, limb_cnt);
    }

  SIZ(r) = SIZ(u) >= 0 ? rn : -rn;
}
