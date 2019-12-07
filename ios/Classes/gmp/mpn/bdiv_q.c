/* mpn_bdiv_q -- Hensel division with precomputed inverse, returning quotient.

   Contributed to the GNU project by Torbjorn Granlund.

   THE FUNCTIONS IN THIS FILE ARE INTERNAL WITH MUTABLE INTERFACES.  IT IS ONLY
   SAFE TO REACH THEM THROUGH DOCUMENTED INTERFACES.  IN FACT, IT IS ALMOST
   GUARANTEED THAT THEY WILL CHANGE OR DISAPPEAR IN A FUTURE GMP RELEASE.

Copyright 2006, 2007, 2009 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"


/* Computes Q = N / D mod B^n. */

void
mpn_bdiv_q (mp_ptr qp,
	    mp_srcptr np, mp_size_t nn,
	    mp_srcptr dp, mp_size_t dn,
	    mp_ptr tp)
{
  mp_limb_t di;

  if (BELOW_THRESHOLD (dn, DC_BDIV_Q_THRESHOLD))
    {
      MPN_COPY (tp, np, nn);
      binvert_limb (di, dp[0]);  di = -di;
      mpn_sbpi1_bdiv_q (qp, tp, nn, dp, dn, di);
    }
  else if (BELOW_THRESHOLD (dn, MU_BDIV_Q_THRESHOLD))
    {
      MPN_COPY (tp, np, nn);
      binvert_limb (di, dp[0]);  di = -di;
      mpn_dcpi1_bdiv_q (qp, tp, nn, dp, dn, di);
    }
  else
    {
      mpn_mu_bdiv_q (qp, np, nn, dp, dn, tp);
    }
  return;
}

mp_size_t
mpn_bdiv_q_itch (mp_size_t nn, mp_size_t dn)
{
  if (BELOW_THRESHOLD (dn, MU_BDIV_Q_THRESHOLD))
    return nn;
  else
    return mpn_mu_bdiv_q_itch (nn, dn);
}
