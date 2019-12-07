/* mpn_lshiftc -- Shift left low level with complement.

Copyright 1991, 1993, 1994, 1996, 2000-2002, 2009 Free Software Foundation,
Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

/* Shift U (pointed to by up and n limbs long) cnt bits to the left
   and store the n least significant limbs of the result at rp.
   Return the bits shifted out from the most significant limb.

   Argument constraints:
   1. 0 < cnt < GMP_NUMB_BITS.
   2. If the result is to be written over the input, rp must be >= up.
*/

mp_limb_t
mpn_lshiftc (mp_ptr rp, mp_srcptr up, mp_size_t n, unsigned int cnt)
{
  mp_limb_t high_limb, low_limb;
  unsigned int tnc;
  mp_size_t i;
  mp_limb_t retval;

  ASSERT (n >= 1);
  ASSERT (cnt >= 1);
  ASSERT (cnt < GMP_NUMB_BITS);
  ASSERT (MPN_SAME_OR_DECR_P (rp, up, n));

  up += n;
  rp += n;

  tnc = GMP_NUMB_BITS - cnt;
  low_limb = *--up;
  retval = low_limb >> tnc;
  high_limb = (low_limb << cnt);

  for (i = n - 1; i != 0; i--)
    {
      low_limb = *--up;
      *--rp = (~(high_limb | (low_limb >> tnc))) & GMP_NUMB_MASK;
      high_limb = low_limb << cnt;
    }
  *--rp = (~high_limb) & GMP_NUMB_MASK;

  return retval;
}
