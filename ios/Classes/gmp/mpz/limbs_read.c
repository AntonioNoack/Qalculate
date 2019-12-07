/* mpz_limbs_read -- Read access to the mpn-style limb array.

Copyright 2013 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

mp_srcptr
mpz_limbs_read (mpz_srcptr x)
{
  return PTR(x);
}
