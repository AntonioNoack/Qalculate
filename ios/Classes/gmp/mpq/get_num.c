 /* mpq_get_num(num,rat_src) -- Set NUM to the numerator of RAT_SRC.

Copyright 1991, 1994, 1995, 2001, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpq_get_num (mpz_ptr num, mpq_srcptr src)
{
  mp_size_t size = SIZ(NUM(src));
  mp_size_t abs_size = ABS (size);
  mp_ptr dp;

  dp = MPZ_NEWALLOC (num, abs_size);
  SIZ(num) = size;

  MPN_COPY (dp, PTR(NUM(src)), abs_size);
}
