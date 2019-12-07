/* mpz_set (dest_integer, src_integer) -- Assign DEST_INTEGER from SRC_INTEGER.

Copyright 1991, 1993-1995, 2000, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"


void
mpz_set (mpz_ptr w, mpz_srcptr u)
{
  mp_ptr wp, up;
  mp_size_t usize, size;

  usize = SIZ(u);
  size = ABS (usize);

  wp = MPZ_REALLOC (w, size);

  up = PTR(u);

  MPN_COPY (wp, up, size);
  SIZ(w) = usize;
}
