/* mpf_get_prec(x) -- Return the precision in bits of x.

Copyright 1996, 2000, 2001 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

mp_bitcnt_t
mpf_get_prec (mpf_srcptr x) __GMP_NOTHROW
{
  return __GMPF_PREC_TO_BITS (x->_mp_prec);
}
