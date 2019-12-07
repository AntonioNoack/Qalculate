/* mpf_init2() -- Make a new multiple precision number with value 0.

Copyright 1993-1995, 2000, 2001, 2004 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

void
mpf_init2 (mpf_ptr r, mp_bitcnt_t prec_in_bits)
{
  mp_size_t prec;

  prec = __GMPF_BITS_TO_PREC (prec_in_bits);
  r->_mp_size = 0;
  r->_mp_exp = 0;
  r->_mp_prec = prec;
  r->_mp_d = __GMP_ALLOCATE_FUNC_LIMBS (prec + 1);
}
