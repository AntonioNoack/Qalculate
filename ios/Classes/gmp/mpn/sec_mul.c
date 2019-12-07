/* mpn_sec_mul.

   Contributed to the GNU project by Torbjörn Granlund.

Copyright 2013 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

void
mpn_sec_mul (mp_ptr rp,
	     mp_srcptr ap, mp_size_t an,
	     mp_srcptr bp, mp_size_t bn,
	     mp_ptr tp)
{
  mpn_mul_basecase (rp, ap, an, bp, bn);
}

mp_size_t
mpn_sec_mul_itch (mp_size_t an, mp_size_t bn)
{
  return 0;
}
