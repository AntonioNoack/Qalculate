/* mpq_neg -- negate a rational.

Copyright 2000, 2001, 2012 Free Software Foundation, Inc.
  */

#define __GMP_FORCE_mpq_neg 1

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"


void
mpq_neg (mpq_ptr dst, mpq_srcptr src)
{
  mp_size_t  num_size = SIZ(NUM(src));

  if (src != dst)
    {
      mp_size_t  size;
      mp_ptr dp;

      size = ABS(num_size);
      dp = MPZ_NEWALLOC (NUM(dst), size);
      MPN_COPY (dp, PTR(NUM(src)), size);

      size = SIZ(DEN(src));
      dp = MPZ_NEWALLOC (DEN(dst), size);
      SIZ(DEN(dst)) = size;
      MPN_COPY (dp, PTR(DEN(src)), size);
    }

  SIZ(NUM(dst)) = -num_size;
}
