/* mpf_init_set_d -- Initialize a float and assign it from a double.

Copyright 1993-1995, 2000, 2001, 2004 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "set_dfl_prec.c"

void
mpf_init_set_d (mpf_ptr r, double val)
{
  mp_size_t prec = __gmp_default_fp_limb_precision;
  r->_mp_prec = prec;
  r->_mp_d = __GMP_ALLOCATE_FUNC_LIMBS (prec + 1);

  mpf_set_d (r, val);
}
