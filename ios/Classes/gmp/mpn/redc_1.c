/* mpn_redc_1.  Set rp[] <- up[]/R^n mod mp[].  Clobber up[].
   mp[] is n limbs; up[] is 2n limbs.

   THIS IS AN INTERNAL FUNCTION WITH A MUTABLE INTERFACE.  IT IS ONLY
   SAFE TO REACH THIS FUNCTION THROUGH DOCUMENTED INTERFACES.

Copyright (C) 2000-2002, 2004, 2008, 2009, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

mp_limb_t
mpn_redc_1 (mp_ptr rp, mp_ptr up, mp_srcptr mp, mp_size_t n, mp_limb_t invm)
{
  mp_size_t j;
  mp_limb_t cy;

  ASSERT (n > 0);
  ASSERT_MPN (up, 2*n);

  for (j = n - 1; j >= 0; j--)
    {
      cy = mpn_addmul_1 (up, mp, n, (up[0] * invm) & GMP_NUMB_MASK);
      ASSERT (up[0] == 0);
      up[0] = cy;
      up++;
    }

  cy = mpn_add_n (rp, up, up - n, n);
  return cy;
}
