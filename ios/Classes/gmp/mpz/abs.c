/* mpz_abs(dst, src) -- Assign the absolute value of SRC to DST.

Copyright 1991, 1993-1995, 2001, 2012 Free Software Foundation, Inc.
  */

#define __GMP_FORCE_mpz_abs 1

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_abs (mpz_ptr w, mpz_srcptr u)
{
  mp_ptr wp;
  mp_srcptr up;
  mp_size_t size;

  size = ABSIZ (u);

  if (u != w)
    {
      wp = MPZ_NEWALLOC (w, size);

      up = PTR (u);

      MPN_COPY (wp, up, size);
    }

  SIZ (w) = size;
}
