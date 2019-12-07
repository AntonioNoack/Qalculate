/* mpf_set_z -- Assign a float from an integer.

Copyright 1996, 2001, 2004 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

void
mpf_set_z (mpf_ptr r, mpz_srcptr u)
{
  mp_ptr rp, up;
  mp_size_t size, asize;
  mp_size_t prec;

  prec = PREC (r) + 1;
  size = SIZ (u);
  asize = ABS (size);
  rp = PTR (r);
  up = PTR (u);

  EXP (r) = asize;

  if (asize > prec)
    {
      up += asize - prec;
      asize = prec;
    }

  SIZ (r) = size >= 0 ? asize : -asize;
  MPN_COPY (rp, up, asize);
}
