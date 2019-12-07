/* mpf_cmp -- Compare two floats.

Copyright 1993, 1994, 1996, 2001, 2015 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

int
mpf_cmp (mpf_srcptr u, mpf_srcptr v) __GMP_NOTHROW
{
  mp_srcptr up, vp;
  mp_size_t usize, vsize;
  mp_exp_t uexp, vexp;
  int cmp;
  int usign;

  usize = SIZ(u);
  vsize = SIZ(v);
  usign = usize >= 0 ? 1 : -1;

  /* 1. Are the signs different?  */
  if ((usize ^ vsize) >= 0)
    {
      /* U and V are both non-negative or both negative.  */
      if (usize == 0)
	/* vsize >= 0 */
	return -(vsize != 0);
      if (vsize == 0)
	/* usize >= 0 */
	return usize != 0;
      /* Fall out.  */
    }
  else
    {
      /* Either U or V is negative, but not both.  */
      return usign;
    }

  /* U and V have the same sign and are both non-zero.  */

  uexp = EXP(u);
  vexp = EXP(v);

  /* 2. Are the exponents different?  */
  if (uexp > vexp)
    return usign;
  if (uexp < vexp)
    return -usign;

  usize = ABS (usize);
  vsize = ABS (vsize);

  up = PTR (u);
  vp = PTR (v);

#define STRICT_MPF_NORMALIZATION 0
#if ! STRICT_MPF_NORMALIZATION
  /* Ignore zeroes at the low end of U and V.  */
  do {
    mp_limb_t tl;
    tl = up[0];
    MPN_STRIP_LOW_ZEROS_NOT_ZERO (up, usize, tl);
    tl = vp[0];
    MPN_STRIP_LOW_ZEROS_NOT_ZERO (vp, vsize, tl);
  } while (0);
#endif

  if (usize > vsize)
    {
      cmp = mpn_cmp (up + usize - vsize, vp, vsize);
      /* if (cmp == 0) */
      /*	return usign; */
      ++cmp;
    }
  else if (vsize > usize)
    {
      cmp = mpn_cmp (up, vp + vsize - usize, usize);
      /* if (cmp == 0) */
      /*	return -usign; */
    }
  else
    {
      cmp = mpn_cmp (up, vp, usize);
      if (cmp == 0)
	return 0;
    }
  return cmp > 0 ? usign : -usign;
}
