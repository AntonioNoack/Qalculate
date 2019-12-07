/* mpq_set_num(dest,num) -- Set the numerator of DEST from NUM.

Copyright 1991, 1994, 1995, 2001, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpq_set_num (mpq_ptr dest, mpz_srcptr num)
{
  mp_size_t size = SIZ (num);
  mp_size_t abs_size = ABS (size);
  mp_ptr dp;

  dp = MPZ_NEWALLOC (NUM(dest), abs_size);

  SIZ(NUM(dest)) = size;
  MPN_COPY (dp, PTR(num), abs_size);
}
