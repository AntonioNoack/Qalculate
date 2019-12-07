/* mpf_set_default_prec --

Copyright 1993-1995, 2001 Free Software Foundation, Inc.
  */

#ifndef SET_DFL_PREC_C
#define SET_DFL_PREC_C

#include "../gmp.h"
#include "../gmp-impl.h"

#ifdef QAL

extern mp_size_t __gmp_default_fp_limb_precision;

#else

mp_size_t __gmp_default_fp_limb_precision = __GMPF_BITS_TO_PREC (53);

#endif

inline void mpf_set_default_prec (mp_bitcnt_t prec_in_bits) __GMP_NOTHROW
{
  __gmp_default_fp_limb_precision = __GMPF_BITS_TO_PREC (prec_in_bits);
}

#endif /* SET_DFL_PREC_C */
