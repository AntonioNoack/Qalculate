/* mpn_copyi

Copyright 2009 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

void
mpn_copyi (mp_ptr rp, mp_srcptr up, mp_size_t n)
{
  mp_size_t i;

  up += n;
  rp += n;
  for (i = -n; i != 0; i++)
    rp[i] = up[i];
}
