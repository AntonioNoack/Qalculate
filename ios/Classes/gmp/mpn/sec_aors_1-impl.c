/* mpn_sec_add_1, mpn_sec_sub_1

   Contributed to the GNU project by Niels Möller

Copyright 2013, 2014 Free Software Foundation, Inc.
  */


#include "../gmp.h"
#include "../gmp-impl.h"

/* It's annoying to that we need scratch space */
mp_size_t
mpn_sec_add_1_itch (mp_size_t n)
{
  return n;
}

mp_limb_t
mpn_sec_add_1 (mp_ptr rp, mp_srcptr ap, mp_size_t n, mp_limb_t b, mp_ptr scratch)
{
  scratch[0] = b;
  MPN_ZERO (scratch + 1, n-1);
  return mpn_add_n (rp, ap, scratch, n);
}

/* It's annoying to that we need scratch space */
mp_size_t
mpn_sec_sub_1_itch (mp_size_t n)
{
  return n;
}

mp_limb_t
mpn_sec_sub_1 (mp_ptr rp, mp_srcptr ap, mp_size_t n, mp_limb_t b, mp_ptr scratch)
{
  scratch[0] = b;
  MPN_ZERO (scratch + 1, n-1);
  return mpn_sub_n (rp, ap, scratch, n);
}