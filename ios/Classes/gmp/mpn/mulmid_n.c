/* mpn_mulmid_n -- balanced middle product

   Contributed by David Harvey.

   THE FUNCTION IN THIS FILE IS INTERNAL WITH A MUTABLE INTERFACE.  IT IS ONLY
   SAFE TO REACH IT THROUGH DOCUMENTED INTERFACES.  IN FACT, IT IS ALMOST
   GUARANTEED THAT IT'LL CHANGE OR DISAPPEAR IN A FUTURE GNU MP RELEASE. */


#include "../gmp.h"
#include "../gmp-impl.h"


void
mpn_mulmid_n (mp_ptr rp, mp_srcptr ap, mp_srcptr bp, mp_size_t n)
{
  ASSERT (n >= 1);
  ASSERT (! MPN_OVERLAP_P (rp, n + 2, ap, 2*n - 1));
  ASSERT (! MPN_OVERLAP_P (rp, n + 2, bp, n));

  if (n < MULMID_TOOM42_THRESHOLD)
    {
      mpn_mulmid_basecase (rp, ap, 2*n - 1, bp, n);
    }
  else
    {
      mp_ptr scratch;
      TMP_DECL;
      TMP_MARK;
      scratch = TMP_ALLOC_LIMBS (mpn_toom42_mulmid_itch (n));
      mpn_toom42_mulmid (rp, ap, bp, n, scratch);
      TMP_FREE;
    }
}
