/* mpn_div_qr_2n_pi1

   Contributed to the GNU project by Torbjorn Granlund and Niels Möller

   THIS FILE CONTAINS INTERNAL FUNCTIONS WITH MUTABLE INTERFACES.  IT IS ONLY
   SAFE TO REACH THEM THROUGH DOCUMENTED INTERFACES.  IN FACT, IT IS ALMOST
   GUARANTEED THAT THEY'LL CHANGE OR DISAPPEAR IN A FUTURE GNU MP RELEASE.
 */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"


/* 3/2 loop, for normalized divisor */
mp_limb_t
mpn_div_qr_2n_pi1 (mp_ptr qp, mp_ptr rp, mp_srcptr np, mp_size_t nn,
		   mp_limb_t d1, mp_limb_t d0, mp_limb_t di)
{
  mp_limb_t qh;
  mp_size_t i;
  mp_limb_t r1, r0;

  ASSERT (nn >= 2);
  ASSERT (d1 & GMP_NUMB_HIGHBIT);

  np += nn - 2;
  r1 = np[1];
  r0 = np[0];

  qh = 0;
  if (r1 >= d1 && (r1 > d1 || r0 >= d0))
    {
#if GMP_NAIL_BITS == 0
      sub_ddmmss (r1, r0, r1, r0, d1, d0);
#else
      r0 = r0 - d0;
      r1 = r1 - d1 - (r0 >> GMP_LIMB_BITS - 1);
      r0 &= GMP_NUMB_MASK;
#endif
      qh = 1;
    }

  for (i = nn - 2 - 1; i >= 0; i--)
    {
      mp_limb_t n0, q;
      n0 = np[-1];
      udiv_qr_3by2 (q, r1, r0, r1, r0, n0, d1, d0, di);
      np--;
      qp[i] = q;
    }

  rp[1] = r1;
  rp[0] = r0;

  return qh;
}
