/* mpn_redc_n.  Set rp[] <- up[]/R^n mod mp[].  Clobber up[].
   mp[] is n limbs; up[] is 2n limbs, the inverse ip[] is n limbs.

   THIS IS AN INTERNAL FUNCTION WITH A MUTABLE INTERFACE.  IT IS ONLY
   SAFE TO REACH THIS FUNCTION THROUGH DOCUMENTED INTERFACES.

Copyright 2009, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

/*
  TODO

  * We assume mpn_mulmod_bnm1 is always faster than plain mpn_mul_n (or a
    future mpn_mulhi) for the range we will be called.  Follow up that
    assumption.

  * Decrease scratch usage.

  * Consider removing the residue canonicalisation.
*/

void
mpn_redc_n (mp_ptr rp, mp_ptr up, mp_srcptr mp, mp_size_t n, mp_srcptr ip)
{
  mp_ptr xp, yp, scratch;
  mp_limb_t cy;
  mp_size_t rn;
  TMP_DECL;
  TMP_MARK;

  ASSERT (n > 8);

  rn = mpn_mulmod_bnm1_next_size (n);

  scratch = TMP_ALLOC_LIMBS (n + rn + mpn_mulmod_bnm1_itch (rn, n, n));

  xp = scratch;
  mpn_mullo_n (xp, up, ip, n);

  yp = scratch + n;
  mpn_mulmod_bnm1 (yp, rn, xp, n, mp, n, scratch + n + rn);

  ASSERT_ALWAYS (2 * n > rn);				/* could handle this */

  cy = mpn_sub_n (yp + rn, yp, up, 2*n - rn);		/* undo wrap around */
  MPN_DECR_U (yp + 2*n - rn, rn, cy);

  cy = mpn_sub_n (rp, up + n, yp + n, n);
  if (cy != 0)
    mpn_add_n (rp, rp, mp, n);

  TMP_FREE;
}
