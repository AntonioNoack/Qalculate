/* mpq_abs -- absolute value of a rational.

Copyright 2000, 2001, 2012 Free Software Foundation, Inc.
  */

#define __GMP_FORCE_mpq_abs 1

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"


void
mpq_abs (mpq_ptr dst, mpq_srcptr src)
{
  mp_size_t  num_abs_size = ABSIZ(NUM(src));

  if (dst != src)
    {
      mp_size_t  den_size = SIZ(DEN(src));
      mp_ptr dp;

      dp = MPZ_NEWALLOC (NUM(dst), num_abs_size);
      MPN_COPY (dp, PTR(NUM(src)), num_abs_size);

      dp = MPZ_NEWALLOC (DEN(dst), den_size);
      SIZ(DEN(dst)) = den_size;
      MPN_COPY (dp, PTR(DEN(src)), den_size);
    }

  SIZ(NUM(dst)) = num_abs_size;
}
