/* mpz_hamdist -- calculate hamming distance.

Copyright 1994, 1996, 2001, 2002, 2009-2011 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"


mp_bitcnt_t
mpz_hamdist (mpz_srcptr u, mpz_srcptr v) __GMP_NOTHROW
{
  mp_srcptr      up, vp;
  mp_size_t      usize, vsize;
  mp_bitcnt_t    count;

  usize = SIZ(u);
  vsize = SIZ(v);

  up = PTR(u);
  vp = PTR(v);

  if (usize >= 0)
    {
      if (vsize < 0)
	return ~ (mp_bitcnt_t) 0;

      /* positive/positive */

      if (usize < vsize)
	MPN_SRCPTR_SWAP (up,usize, vp,vsize);

      count = 0;
      if (vsize != 0)
	count = mpn_hamdist (up, vp, vsize);

      usize -= vsize;
      if (usize != 0)
	count += mpn_popcount (up + vsize, usize);

      return count;
    }
  else
    {
      mp_limb_t  ulimb, vlimb;
      mp_size_t  old_vsize, step;

      if (vsize >= 0)
	return ~ (mp_bitcnt_t) 0;

      /* negative/negative */

      usize = -usize;
      vsize = -vsize;

      /* skip common low zeros */
      for (;;)
	{
	  ASSERT (usize > 0);
	  ASSERT (vsize > 0);

	  usize--;
	  vsize--;

	  ulimb = *up++;
	  vlimb = *vp++;

	  if (ulimb != 0)
	    break;

	  if (vlimb != 0)
	    {
	      MPN_SRCPTR_SWAP (up,usize, vp,vsize);
	      ulimb = vlimb;
	      vlimb = 0;
	      break;
	    }
	}

      /* twos complement first non-zero limbs (ulimb is non-zero, but vlimb
	 might be zero) */
      ulimb = -ulimb;
      vlimb = -vlimb;
      popc_limb (count, (ulimb ^ vlimb) & GMP_NUMB_MASK);

      if (vlimb == 0)
	{
	  mp_bitcnt_t  twoscount;

	  /* first non-zero of v */
	  old_vsize = vsize;
	  do
	    {
	      ASSERT (vsize > 0);
	      vsize--;
	      vlimb = *vp++;
	    }
	  while (vlimb == 0);

	  /* part of u corresponding to skipped v zeros */
	  step = old_vsize - vsize - 1;
	  count += step * GMP_NUMB_BITS;
	  step = MIN (step, usize);
	  if (step != 0)
	    {
	      count -= mpn_popcount (up, step);
	      usize -= step;
	      up += step;
	    }

	  /* First non-zero vlimb as twos complement, xor with ones
	     complement ulimb.  Note -v^(~0^u) == (v-1)^u. */
	  vlimb--;
	  if (usize != 0)
	    {
	      usize--;
	      vlimb ^= *up++;
	    }
	  popc_limb (twoscount, vlimb);
	  count += twoscount;
	}

      /* Overlapping part of u and v, if any.  Ones complement both, so just
	 plain hamdist. */
      step = MIN (usize, vsize);
      if (step != 0)
	{
	  count += mpn_hamdist (up, vp, step);
	  usize -= step;
	  vsize -= step;
	  up += step;
	  vp += step;
	}

      /* Remaining high part of u or v, if any, ones complement but xor
	 against all ones in the other, so plain popcount. */
      if (usize != 0)
	{
	remaining:
	  count += mpn_popcount (up, usize);
	}
      else if (vsize != 0)
	{
	  up = vp;
	  usize = vsize;
	  goto remaining;
	}
      return count;
    }
}