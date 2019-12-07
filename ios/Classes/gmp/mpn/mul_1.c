/* mpn_mul_1 -- Multiply a limb vector with a single limb and store the
   product in a second limb vector.

Copyright 1991-1994, 1996, 2000-2002 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"


#if GMP_NAIL_BITS == 0

mp_limb_t
mpn_mul_1 (mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t vl)
{
  mp_limb_t ul, cl, hpl, lpl;

  ASSERT (n >= 1);
  ASSERT (MPN_SAME_OR_INCR_P (rp, up, n));

  cl = 0;
  do
    {
      ul = *up++;
      umul_ppmm (hpl, lpl, ul, vl);

      lpl += cl;
      cl = (lpl < cl) + hpl;

      *rp++ = lpl;
    }
  while (--n != 0);

  return cl;
}

#endif

#if GMP_NAIL_BITS >= 1

mp_limb_t
mpn_mul_1 (mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t vl)
{
  mp_limb_t shifted_vl, ul, lpl, hpl, prev_hpl, xw, cl, xl;

  ASSERT (n >= 1);
  ASSERT (MPN_SAME_OR_INCR_P (rp, up, n));
  ASSERT_MPN (up, n);
  ASSERT_LIMB (vl);

  shifted_vl = vl << GMP_NAIL_BITS;
  cl = 0;
  prev_hpl = 0;
  do
    {
      ul = *up++;

      umul_ppmm (hpl, lpl, ul, shifted_vl);
      lpl >>= GMP_NAIL_BITS;
      xw = prev_hpl + lpl + cl;
      cl = xw >> GMP_NUMB_BITS;
      xl = xw & GMP_NUMB_MASK;
      *rp++ = xl;
      prev_hpl = hpl;
    }
  while (--n != 0);

  return prev_hpl + cl;
}

#endif
