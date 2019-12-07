/* mpf_sub_ui -- Subtract an unsigned integer from a float.

Copyright 1993, 1994, 1996, 2001 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpf_sub_ui (mpf_ptr sum, mpf_srcptr u, unsigned long int v)
{
  __mpf_struct vv;
  mp_limb_t vl;

  if (v == 0)
    {
      mpf_set (sum, u);
      return;
    }

  vl = v;
  vv._mp_size = 1;
  vv._mp_d = &vl;
  vv._mp_exp = 1;
  mpf_sub (sum, u, &vv);
}
