/* mpz_perfect_power_p(arg) -- Return non-zero if ARG is a perfect power,
   zero otherwise.

Copyright 1998-2001, 2005, 2008, 2009 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

int
mpz_perfect_power_p (mpz_srcptr u)
{
  return mpn_perfect_power_p (PTR (u), SIZ (u));
}
