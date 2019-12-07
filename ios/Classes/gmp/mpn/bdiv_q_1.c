/* mpn_bdiv_q_1, mpn_pi1_bdiv_q_1 -- schoolbook Hensel division by 1-limb
   divisor, returning quotient only.

   THE FUNCTIONS IN THIS FILE ARE FOR INTERNAL USE ONLY.  THEY'RE ALMOST
   CERTAIN TO BE SUBJECT TO INCOMPATIBLE CHANGES OR DISAPPEAR COMPLETELY IN
   FUTURE GNU MP RELEASES.

Copyright 2000-2003, 2005, 2009 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"

mp_limb_t
mpn_pi1_bdiv_q_1 (mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t d,
		  mp_limb_t di, int shift)
{
  mp_size_t  i;
  mp_limb_t  c, h, l, u, u_next, dummy;

  ASSERT (n >= 1);
  ASSERT (d != 0);
  ASSERT (MPN_SAME_OR_SEPARATE_P (rp, up, n));
  ASSERT_MPN (up, n);
  ASSERT_LIMB (d);

  d <<= GMP_NAIL_BITS;

  if (shift != 0)
    {
      c = 0;

      u = up[0];
      rp--;
      for (i = 1; i < n; i++)
	{
	  u_next = up[i];
	  u = ((u >> shift) | (u_next << (GMP_NUMB_BITS-shift))) & GMP_NUMB_MASK;

	  SUBC_LIMB (c, l, u, c);

	  l = (l * di) & GMP_NUMB_MASK;
	  rp[i] = l;

	  umul_ppmm (h, dummy, l, d);
	  c += h;
	  u = u_next;
	}

      u = u >> shift;
      l = u - c;
      l = (l * di) & GMP_NUMB_MASK;
      rp[i] = l;
    }
  else
    {
      u = up[0];
      l = (u * di) & GMP_NUMB_MASK;
      rp[0] = l;
      c = 0;

      for (i = 1; i < n; i++)
	{
	  umul_ppmm (h, dummy, l, d);
	  c += h;

	  u = up[i];
	  SUBC_LIMB (c, l, u, c);

	  l = (l * di) & GMP_NUMB_MASK;
	  rp[i] = l;
	}
    }

  return c;
}

mp_limb_t
mpn_bdiv_q_1 (mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t d)
{
  mp_limb_t di;
  int shift;

  ASSERT (n >= 1);
  ASSERT (d != 0);
  ASSERT (MPN_SAME_OR_SEPARATE_P (rp, up, n));
  ASSERT_MPN (up, n);
  ASSERT_LIMB (d);

  if ((d & 1) == 0)
    {
      count_trailing_zeros (shift, d);
      d >>= shift;
    }
  else
    shift = 0;

  binvert_limb (di, d);
  return mpn_pi1_bdiv_q_1 (rp, up, n, d, di, shift);
}
