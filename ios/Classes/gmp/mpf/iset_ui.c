/* mpf_init_set_ui() -- Initialize a float and assign it from an unsigned int.

Copyright 1993-1995, 2000, 2001, 2003, 2004 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "set_dfl_prec.c"

void
mpf_init_set_ui (mpf_ptr r, unsigned long int val)
{
  mp_size_t prec = __gmp_default_fp_limb_precision;
  mp_size_t size;

  r->_mp_prec = prec;
  r->_mp_d = __GMP_ALLOCATE_FUNC_LIMBS (prec + 1);
  r->_mp_d[0] = val & GMP_NUMB_MASK;
  size = (val != 0);

#if BITS_PER_ULONG > GMP_NUMB_BITS
  val >>= GMP_NUMB_BITS;
  r->_mp_d[1] = val;
  size += (val != 0);
#endif

  r->_mp_size = size;
  r->_mp_exp = size;
}
