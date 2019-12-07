/* mpz_swap (dest_integer, src_integer) -- Swap U and V.

Copyright 1997, 1998, 2001, 2012, 2015 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_swap (mpz_ptr u, mpz_ptr v) __GMP_NOTHROW
{
  MP_SIZE_T_SWAP (ALLOC(u), ALLOC(v));
  MP_SIZE_T_SWAP (SIZ(u), SIZ(v));
  MP_PTR_SWAP (PTR(v), PTR(u));
}
