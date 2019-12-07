/* mpf_get_default_prec -- return default precision in bits.

Copyright 2001 Free Software Foundation, Inc.
  */

#ifndef GET_DFL_PREC_C
#define GET_DFL_PREC_C

#include "../gmp.h"
#include "../gmp-impl.h"
#include "set_dfl_prec.c"

mp_bitcnt_t
mpf_get_default_prec (void) __GMP_NOTHROW
{
  return __GMPF_PREC_TO_BITS (__gmp_default_fp_limb_precision);
}

#endif