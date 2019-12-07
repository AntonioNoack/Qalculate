/* mpn_add_n -- Add equal length limb vectors.

Copyright 1992-1994, 1996, 2000, 2002, 2009 Free Software Foundation, Inc.
  */

#ifndef adaddnc156132
#define adaddnc156132

#include "../gmp.h"
#include "../gmp-impl.h"


#if GMP_NAIL_BITS == 0

mp_limb_t
mpn_add_n (mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n)
{
  mp_limb_t ul, vl, sl, rl, cy, cy1, cy2;

  ASSERT (n >= 1);
  ASSERT (MPN_SAME_OR_INCR_P (rp, up, n));
  ASSERT (MPN_SAME_OR_INCR_P (rp, vp, n));

  cy = 0;
  do
    {
      ul = *up++;
      vl = *vp++;
      sl = ul + vl;
      cy1 = sl < ul;
      rl = sl + cy;
      cy2 = rl < sl;
      cy = cy1 | cy2;
      *rp++ = rl;
    }
  while (--n != 0);

  return cy;
}

#endif

#if GMP_NAIL_BITS >= 1

mp_limb_t
mpn_add_n (mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n)
{
  mp_limb_t ul, vl, rl, cy;

  ASSERT (n >= 1);
  ASSERT (MPN_SAME_OR_INCR_P (rp, up, n));
  ASSERT (MPN_SAME_OR_INCR_P (rp, vp, n));

  cy = 0;
  do
    {
      ul = *up++;
      vl = *vp++;
      rl = ul + vl + cy;
      cy = rl >> GMP_NUMB_BITS;
      *rp++ = rl & GMP_NUMB_MASK;
    }
  while (--n != 0);

  return cy;
}

#endif

#endif
