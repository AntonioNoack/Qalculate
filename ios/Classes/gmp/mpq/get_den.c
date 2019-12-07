/* mpq_get_den(den,rat_src) -- Set DEN to the denominator of RAT_SRC.

Copyright 1991, 1994, 1995, 2001, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpq_get_den (mpz_ptr den, mpq_srcptr src)
{
  mp_size_t size = SIZ(DEN(src));
  mp_ptr dp;

  dp = MPZ_NEWALLOC (den, size);
  SIZ(den) = size;
  MPN_COPY (dp, PTR(DEN(src)), size);
}
