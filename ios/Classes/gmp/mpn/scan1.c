/* mpn_scan1 -- Scan from a given bit position for the next set bit.

Copyright 1994, 1996, 2001, 2002, 2004 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"

/* Argument constraints:
   1. U must sooner or later have a limb != 0.
 */

mp_bitcnt_t
mpn_scan1 (mp_srcptr up, mp_bitcnt_t starting_bit)
{
  mp_size_t starting_word;
  mp_limb_t alimb;
  int cnt;
  mp_srcptr p;

  /* Start at the word implied by STARTING_BIT.  */
  starting_word = starting_bit / GMP_NUMB_BITS;
  p = up + starting_word;
  alimb = *p++;

  /* Mask off any bits before STARTING_BIT in the first limb.  */
  alimb &= - (mp_limb_t) 1 << (starting_bit % GMP_NUMB_BITS);

  while (alimb == 0)
    alimb = *p++;

  count_trailing_zeros (cnt, alimb);
  return (p - up - 1) * GMP_NUMB_BITS + cnt;
}
