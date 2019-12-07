/* mpn_bsqrt, a^{1/2} (mod 2^n).

Copyright 2009, 2010, 2012, 2015 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"


void
mpn_bsqrt (mp_ptr rp, mp_srcptr ap, mp_bitcnt_t nb, mp_ptr tp)
{
  mp_ptr sp;
  mp_size_t n;

  ASSERT (nb > 0);

  n = nb / GMP_NUMB_BITS;
  sp = tp + n;

  mpn_bsqrtinv (tp, ap, nb, sp);
  mpn_mullo_n (rp, tp, ap, n);
}
