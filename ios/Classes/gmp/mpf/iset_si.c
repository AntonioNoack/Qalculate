/* mpf_init_set_si() -- Initialize a float and assign it from a signed int.

Copyright 1993-1995, 2000, 2001, 2003, 2004, 2012 Free Software Foundation,
Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "get_dfl_prec.c"

void
mpf_init_set_si (mpf_ptr r, long int val)
{
  mp_size_t prec = __gmp_default_fp_limb_precision;
  mp_size_t size;
  mp_limb_t vl;

  r->_mp_prec = prec;
  r->_mp_d = __GMP_ALLOCATE_FUNC_LIMBS (prec + 1);

  vl = (mp_limb_t) ABS_CAST (unsigned long int, val);

  r->_mp_d[0] = vl & GMP_NUMB_MASK;
  size = vl != 0;

#if BITS_PER_ULONG > GMP_NUMB_BITS
  vl >>= GMP_NUMB_BITS;
  r->_mp_d[1] = vl;
  size += (vl != 0);
#endif

  r->_mp_exp = size;
  r->_mp_size = val >= 0 ? size : -size;
}
