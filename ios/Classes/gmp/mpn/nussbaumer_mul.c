/* mpn_nussbaumer_mul -- Multiply {ap,an} and {bp,bn} using
   Nussbaumer's negacyclic convolution.

   Contributed to the GNU project by Marco Bodrato.

   THE FUNCTION IN THIS FILE IS INTERNAL WITH A MUTABLE INTERFACE.  IT IS ONLY
   SAFE TO REACH IT THROUGH DOCUMENTED INTERFACES.  IN FACT, IT IS ALMOST
   GUARANTEED THAT IT WILL CHANGE OR DISAPPEAR IN A FUTURE GNU MP RELEASE.

Copyright 2009 Free Software Foundation, Inc.
  */


#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

/* Multiply {ap,an} by {bp,bn}, and put the result in {pp, an+bn} */
void
mpn_nussbaumer_mul (mp_ptr pp,
		    mp_srcptr ap, mp_size_t an,
		    mp_srcptr bp, mp_size_t bn)
{
  mp_size_t rn;
  mp_ptr tp;
  TMP_DECL;

  ASSERT (an >= bn);
  ASSERT (bn > 0);

  TMP_MARK;

  if ((ap == bp) && (an == bn))
    {
      rn = mpn_sqrmod_bnm1_next_size (2*an);
      tp = TMP_ALLOC_LIMBS (mpn_sqrmod_bnm1_itch (rn, an));
      mpn_sqrmod_bnm1 (pp, rn, ap, an, tp);
    }
  else
    {
      rn = mpn_mulmod_bnm1_next_size (an + bn);
      tp = TMP_ALLOC_LIMBS (mpn_mulmod_bnm1_itch (rn, an, bn));
      mpn_mulmod_bnm1 (pp, rn, ap, an, bp, bn, tp);
    }

  TMP_FREE;
}
