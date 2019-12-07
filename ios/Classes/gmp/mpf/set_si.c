/* mpf_set_si() -- Assign a float from a signed int.

Copyright 1993-1995, 2000-2002, 2004, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

void
mpf_set_si (mpf_ptr dest, long val)
{
  mp_size_t size;
  mp_limb_t vl;

  vl = (mp_limb_t) ABS_CAST (unsigned long int, val);

  dest->_mp_d[0] = vl & GMP_NUMB_MASK;
  size = vl != 0;

#if BITS_PER_ULONG > GMP_NUMB_BITS
  vl >>= GMP_NUMB_BITS;
  dest->_mp_d[1] = vl;
  size += (vl != 0);
#endif

  dest->_mp_exp = size;
  dest->_mp_size = val >= 0 ? size : -size;
}
