/* Compute {up,n}^(-1) mod B^n.

   Contributed to the GNU project by Torbjorn Granlund.

   THE FUNCTIONS IN THIS FILE ARE INTERNAL WITH MUTABLE INTERFACES.  IT IS ONLY
   SAFE TO REACH THEM THROUGH DOCUMENTED INTERFACES.  IN FACT, IT IS ALMOST
   GUARANTEED THAT THEY WILL CHANGE OR DISAPPEAR IN A FUTURE GMP RELEASE.

Copyright (C) 2004-2007, 2009, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"


/*
  r[k+1] = r[k] - r[k] * (u*r[k] - 1)
  r[k+1] = r[k] + r[k] - r[k]*(u*r[k])
*/

#if TUNE_PROGRAM_BUILD
#define NPOWS \
 ((sizeof(mp_size_t) > 6 ? 48 : 8*sizeof(mp_size_t)))
#else
#define NPOWS \
 ((sizeof(mp_size_t) > 6 ? 48 : 8*sizeof(mp_size_t)) - LOG2C (BINV_NEWTON_THRESHOLD))
#endif

mp_size_t
mpn_binvert_itch (mp_size_t n)
{
  mp_size_t itch_local = mpn_mulmod_bnm1_next_size (n);
  mp_size_t itch_out = mpn_mulmod_bnm1_itch (itch_local, n, (n + 1) >> 1);
  return itch_local + itch_out;
}

void
mpn_binvert (mp_ptr rp, mp_srcptr up, mp_size_t n, mp_ptr scratch)
{
  mp_ptr xp;
  mp_size_t rn, newrn;
  mp_size_t sizes[NPOWS], *sizp;
  mp_limb_t di;

  /* Compute the computation precisions from highest to lowest, leaving the
     base case size in 'rn'.  */
  sizp = sizes;
  for (rn = n; ABOVE_THRESHOLD (rn, BINV_NEWTON_THRESHOLD); rn = (rn + 1) >> 1)
    *sizp++ = rn;

  xp = scratch;

  /* Compute a base value of rn limbs.  */
  MPN_ZERO (xp, rn);
  xp[0] = 1;
  binvert_limb (di, up[0]);
  if (BELOW_THRESHOLD (rn, DC_BDIV_Q_THRESHOLD))
    mpn_sbpi1_bdiv_q (rp, xp, rn, up, rn, -di);
  else
    mpn_dcpi1_bdiv_q (rp, xp, rn, up, rn, -di);

  /* Use Newton iterations to get the desired precision.  */
  for (; rn < n; rn = newrn)
    {
      mp_size_t m;
      newrn = *--sizp;

      /* X <- UR. */
      m = mpn_mulmod_bnm1_next_size (newrn);
      mpn_mulmod_bnm1 (xp, m, up, newrn, rp, rn, xp + m);
      mpn_sub_1 (xp + m, xp, rn - (m - newrn), 1);

      /* R = R(X/B^rn) */
      mpn_mullo_n (rp + rn, rp, xp + rn, newrn - rn);
      mpn_neg (rp + rn, rp + rn, newrn - rn);
    }
}
