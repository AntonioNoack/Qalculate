/* mpn_trialdiv -- find small factors of an mpn number using trial division.

   Contributed to the GNU project by Torbjorn Granlund.

   THE FUNCTION IN THIS FILE IS INTERNAL WITH A MUTABLE INTERFACE.  IT IS ONLY
   SAFE TO REACH IT THROUGH DOCUMENTED INTERFACES.  IN FACT, IT IS ALMOST
   GUARANTEED THAT IT WILL CHANGE OR DISAPPEAR IN A FUTURE GNU MP RELEASE.

Copyright 2009, 2010, 2012, 2013 Free Software Foundation, Inc.
  */

/*
   This function finds the first (smallest) factor represented in
   trialdivtab.h.  It does not stop the factoring effort just because it has
   reached some sensible limit, such as the square root of the input number.

   The caller can limit the factoring effort by passing NPRIMES.  The function
   will then divide until that limit, or perhaps a few primes more.  A position
   which only mpn_trialdiv can make sense of is returned in the WHERE
   parameter.  It can be used for restarting the factoring effort; the first
   call should pass 0 here.

   Input:        1. A non-negative number T = {tp,tn}
                 2. NPRIMES as described above,
                 3. *WHERE as described above.
   Output:       1. *WHERE updated as described above.
                 2. Return value is non-zero if we found a factor, else zero
                    To get the actual prime factor, compute the mod B inverse
                    of the return value.
*/

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

struct gmp_primes_dtab {
  mp_limb_t binv;
  mp_limb_t lim;
};

struct gmp_primes_ptab {
  mp_limb_t ppp;	/* primes, multiplied together */
  mp_limb_t cps[7];	/* ppp values pre-computed for mpn_mod_1s_4p */
  gmp_uint_least32_t idx:24;	/* index of  first primes in dtab */
  gmp_uint_least32_t np :8;	/* number of primes related to this entry */
};


static const struct gmp_primes_dtab gmp_primes_dtab[] =
{
#define WANT_dtab
#define P(p,inv,lim) {inv,lim}
#include "../trialdivtab.h"
#undef WANT_dtab
#undef P
  {0,0}
};

static const struct gmp_primes_ptab gmp_primes_ptab[] =
{
#define WANT_ptab
#include "../trialdivtab.h"
#undef WANT_ptab
};

#define PTAB_LINES (sizeof (gmp_primes_ptab) / sizeof (gmp_primes_ptab[0]))

/* FIXME: We could optimize out one of the outer loop conditions if we
   had a final ptab entry with a huge np field.  */
mp_limb_t
mpn_trialdiv (mp_srcptr tp, mp_size_t tn, mp_size_t nprimes, int *where)
{
  mp_limb_t ppp;
  const mp_limb_t *cps;
  const struct gmp_primes_dtab *dp;
  long i, j, idx, np;
  mp_limb_t r, q;

  ASSERT (tn >= 1);

  for (i = *where; i < PTAB_LINES; i++)
    {
      ppp = gmp_primes_ptab[i].ppp;
      cps = gmp_primes_ptab[i].cps;

      r = mpn_mod_1s_4p (tp, tn, ppp << cps[1], cps);

      idx = gmp_primes_ptab[i].idx;
      np = gmp_primes_ptab[i].np;

      /* Check divisibility by individual primes.  */
      dp = &gmp_primes_dtab[idx] + np;
      for (j = -np; j < 0; j++)
	{
	  q = r * dp[j].binv;
	  if (q <= dp[j].lim)
	    {
	      *where = i;
	      return dp[j].binv;
	    }
	}

      nprimes -= np;
      if (nprimes <= 0)
	return 0;
    }
  return 0;
}