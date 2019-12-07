/* mpq_set_z (dest,src) -- Set DEST to SRC.

Copyright 1996, 2001, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpq_set_z (mpq_ptr dest, mpz_srcptr src)
{
  mp_size_t num_size;
  mp_size_t abs_num_size;
  mp_ptr dp;

  num_size = SIZ (src);
  abs_num_size = ABS (num_size);
  dp = MPZ_NEWALLOC (NUM(dest), abs_num_size);
  SIZ(NUM(dest)) = num_size;
  MPN_COPY (dp, PTR(src), abs_num_size);

  PTR(DEN(dest))[0] = 1;
  SIZ(DEN(dest)) = 1;
}
