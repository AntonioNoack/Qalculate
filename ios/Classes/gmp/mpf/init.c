/* mpf_init() -- Make a new multiple precision number with value 0.

Copyright 1993-1995, 2000, 2001, 2004 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "set_dfl_prec.c"

void
mpf_init (mpf_ptr r)
{
  mp_size_t prec = __gmp_default_fp_limb_precision;
  r->_mp_size = 0;
  r->_mp_exp = 0;
  r->_mp_prec = prec;
  r->_mp_d = __GMP_ALLOCATE_FUNC_LIMBS (prec + 1);
}
