/* mpn_cnd_swap

   Contributed to the GNU project by Niels Möller

Copyright 2013, 2015 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

void
mpn_cnd_swap (mp_limb_t cnd, volatile mp_limb_t *ap, volatile mp_limb_t *bp,
	      mp_size_t n)
{
  volatile mp_limb_t mask = - (mp_limb_t) (cnd != 0);
  mp_size_t i;
  for (i = 0; i < n; i++)
    {
      mp_limb_t a, b, t;
      a = ap[i];
      b = bp[i];
      t = (a ^ b) & mask;
      ap[i] = a ^ t;
      bp[i] = b ^ t;
    }
}
