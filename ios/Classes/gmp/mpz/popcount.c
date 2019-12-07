/* mpz_popcount(mpz_ptr op) -- Population count of OP.  If the operand is
   negative, return ~0 (a novel representation of infinity).

Copyright 1994, 1996, 2001 Free Software Foundation, Inc.
  */

#define __GMP_FORCE_mpz_popcount 1

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"
