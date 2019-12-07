/* mpq_set_den(dest,den) -- Set the denominator of DEST from DEN.

Copyright 1991, 1994-1996, 2000, 2001, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpq_set_den (mpq_ptr dest, mpz_srcptr den)
{
  mp_size_t size = SIZ (den);
  mp_size_t abs_size = ABS (size);
  mp_ptr dp;

  dp = MPZ_NEWALLOC (DEN(dest), abs_size);

  SIZ(DEN(dest)) = size;
  MPN_COPY (dp, PTR(den), abs_size);
}
