/* mpn_sec_tabselect.

Copyright 2007-2009, 2011, 2013 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"


/* Select entry `which' from table `tab', which has nents entries, each `n'
   limbs.  Store the selected entry at rp.  Reads entire table to avoid
   side-channel information leaks.  O(n*nents).  */
void
mpn_sec_tabselect (volatile mp_limb_t *rp, volatile const mp_limb_t *tab,
		   mp_size_t n, mp_size_t nents, mp_size_t which)
{
  mp_size_t k, i;
  mp_limb_t mask;
  volatile const mp_limb_t *tp;

  for (k = 0; k < nents; k++)
    {
      mask = -(mp_limb_t) (which == k);
      tp = tab + n * k;
      for (i = 0; i < n; i++)
	{
	  rp[i] = (rp[i] & ~mask) | (tp[i] & mask);
	}
    }
}
