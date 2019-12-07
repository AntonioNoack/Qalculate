/* mpz_random2 -- Generate a positive random mpz_t of specified size, with
   long runs of consecutive ones and zeros in the binary representation.
   Meant for testing of other MP routines.

Copyright 1991, 1993, 1994, 1996, 2001, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_random2 (mpz_ptr x, mp_size_t size)
{
  mp_size_t abs_size;
  mp_ptr xp;

  abs_size = ABS (size);
  if (abs_size != 0)
    {
      xp = MPZ_REALLOC (x, abs_size);

      mpn_random2 (xp, abs_size);
    }

  SIZ (x) = size;
}
