/* mpn_sec_add_1, mpn_sec_sub_1

   Contributed to the GNU project by Niels Möller

Copyright 2013, 2014 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

#if OPERATION_sec_add_1
#define FNAME mpn_sec_add_1
#define FNAME_itch mpn_sec_add_1_itch
#define OP_N mpn_add_n
#endif
#if OPERATION_sec_sub_1
#define FNAME mpn_sec_sub_1
#define FNAME_itch mpn_sec_sub_1_itch
#define OP_N mpn_sub_n
#endif

/* It's annoying to that we need scratch space */
mp_size_t
FNAME_itch (mp_size_t n)
{
  return n;
}

mp_limb_t
FNAME (mp_ptr rp, mp_srcptr ap, mp_size_t n, mp_limb_t b, mp_ptr scratch)
{
  scratch[0] = b;
  MPN_ZERO (scratch + 1, n-1);
  return OP_N (rp, ap, scratch, n);
}
