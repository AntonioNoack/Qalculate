/* mpf_set_prec_raw(x,bits) -- Change the precision of x without changing
   allocation.  For proper operation, the original precision need to be reset
   sooner or later.

Copyright 1996, 2001 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

void
mpf_set_prec_raw (mpf_ptr x, mp_bitcnt_t prec_in_bits) __GMP_NOTHROW
{
  x->_mp_prec = __GMPF_BITS_TO_PREC (prec_in_bits);
}
