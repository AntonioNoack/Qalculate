/* mpn_mulmid_basecase -- classical middle product algorithm

   Contributed by David Harvey.

   THE FUNCTION IN THIS FILE IS INTERNAL WITH A MUTABLE INTERFACE.  IT IS ONLY
   SAFE TO REACH IT THROUGH DOCUMENTED INTERFACES.  IN FACT, IT IS ALMOST
   GUARANTEED THAT IT'LL CHANGE OR DISAPPEAR IN A FUTURE GNU MP RELEASE. */


#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"

/* Middle product of {up,un} and {vp,vn}, write result to {rp,un-vn+3}.
   Must have un >= vn >= 1.

   Neither input buffer may overlap with the output buffer. */

void
mpn_mulmid_basecase (mp_ptr rp,
                     mp_srcptr up, mp_size_t un,
                     mp_srcptr vp, mp_size_t vn)
{
  mp_limb_t lo, hi;  /* last two limbs of output */
  mp_limb_t cy;

  ASSERT (un >= vn);
  ASSERT (vn >= 1);
  ASSERT (! MPN_OVERLAP_P (rp, un - vn + 3, up, un));
  ASSERT (! MPN_OVERLAP_P (rp, un - vn + 3, vp, vn));

  up += vn - 1;
  un -= vn - 1;

  /* multiply by first limb, store result */
  lo = mpn_mul_1 (rp, up, un, vp[0]);
  hi = 0;

  /* accumulate remaining rows */
  for (vn--; vn; vn--)
    {
      up--, vp++;
      cy = mpn_addmul_1 (rp, up, un, vp[0]);
      add_ssaaaa (hi, lo, hi, lo, CNST_LIMB(0), cy);
    }

  /* store final limbs */
#if GMP_NAIL_BITS != 0
  hi = (hi << GMP_NAIL_BITS) + (lo >> GMP_NUMB_BITS);
  lo &= GMP_NUMB_MASK;
#endif

  rp[un] = lo;
  rp[un + 1] = hi;
}