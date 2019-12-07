/* mpf_init_set_str -- Initialize a float and assign it from a string.

Copyright 1995, 1996, 2000, 2001, 2004 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "set_dfl_prec.c"

int
mpf_init_set_str (mpf_ptr r, const char *s, int base)
{
  mp_size_t prec = __gmp_default_fp_limb_precision;
  r->_mp_size = 0;
  r->_mp_exp = 0;
  r->_mp_prec = prec;
  r->_mp_d = __GMP_ALLOCATE_FUNC_LIMBS (prec + 1);

  return mpf_set_str (r, s, base);
}
