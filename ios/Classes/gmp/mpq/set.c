/* mpq_set(dest,src) -- Set DEST to SRC.

Copyright 1991, 1994, 1995, 2001, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpq_set (mpq_ptr dest, mpq_srcptr src)
{
  mp_size_t num_size, den_size;
  mp_size_t abs_num_size;
  mp_ptr dp;

  num_size = SIZ(NUM(src));
  abs_num_size = ABS (num_size);
  dp = MPZ_NEWALLOC (NUM(dest), abs_num_size);
  SIZ(NUM(dest)) = num_size;
  MPN_COPY (dp, PTR(NUM(src)), abs_num_size);

  den_size = SIZ(DEN(src));
  dp = MPZ_NEWALLOC (DEN(dest), den_size);
  SIZ(DEN(dest)) = den_size;
  MPN_COPY (dp, PTR(DEN(src)), den_size);
}
