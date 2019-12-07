/* mpn_copyd

Copyright 2009 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

void
mpn_copyd (mp_ptr rp, mp_srcptr up, mp_size_t n)
{
  mp_size_t i;

  for (i = n - 1; i >= 0; i--)
    rp[i] = up[i];
}
