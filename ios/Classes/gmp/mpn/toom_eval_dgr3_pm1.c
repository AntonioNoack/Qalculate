/* mpn_toom_eval_dgr3_pm1 -- Evaluate a degree 3 polynomial in +1 and -1

   Contributed to the GNU project by Niels Möller

   THE FUNCTION IN THIS FILE IS INTERNAL WITH A MUTABLE INTERFACE.  IT IS ONLY
   SAFE TO REACH IT THROUGH DOCUMENTED INTERFACES.  IN FACT, IT IS ALMOST
   GUARANTEED THAT IT WILL CHANGE OR DISAPPEAR IN A FUTURE GNU MP RELEASE.

Copyright 2009 Free Software Foundation, Inc.
  */


#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

#undef pt
#undef n
#undef xp

int
mpn_toom_eval_dgr3_pm1 (mp_ptr xp1, mp_ptr xm1,
			mp_srcptr xp, mp_size_t n, mp_size_t x3n, mp_ptr tp)
{
  int neg;

  ASSERT (x3n > 0);
  ASSERT (x3n <= n);

  xp1[n] = mpn_add_n (xp1, xp, xp + 2*n, n);
  tp[n] = mpn_add (tp, xp + n, n, xp + 3*n, x3n);

  neg = (mpn_cmp (xp1, tp, n + 1) < 0) ? ~0 : 0;

#if HAVE_NATIVE_mpn_add_n_sub_n
  if (neg)
    mpn_add_n_sub_n (xp1, xm1, tp, xp1, n + 1);
  else
    mpn_add_n_sub_n (xp1, xm1, xp1, tp, n + 1);
#else
  if (neg)
    mpn_sub_n (xm1, tp, xp1, n + 1);
  else
    mpn_sub_n (xm1, xp1, tp, n + 1);

  mpn_add_n (xp1, xp1, tp, n + 1);
#endif

  ASSERT (xp1[n] <= 3);
  ASSERT (xm1[n] <= 1);

  return neg;
}
