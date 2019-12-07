/* mpn_zero

Copyright 2009 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpn_zero (mp_ptr rp, mp_size_t n)
{
  mp_size_t i;

  rp += n;
  for (i = -n; i != 0; i++)
    rp[i] = 0;
}
