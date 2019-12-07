/* mpz_set_ui(integer, val) -- Assign INTEGER with a small value VAL.

Copyright 1991, 1993-1995, 2001, 2002, 2004, 2012 Free Software Foundation,
Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_set_ui (mpz_ptr dest, unsigned long int val)
{
  mp_size_t size;

  PTR (dest)[0] = val & GMP_NUMB_MASK;
  size = val != 0;

#if BITS_PER_ULONG > GMP_NUMB_BITS  /* avoid warnings about shift amount */
  if (val > GMP_NUMB_MAX)
    {
      MPZ_REALLOC (dest, 2);
      PTR (dest)[1] = val >> GMP_NUMB_BITS;
      size = 2;
    }
#endif

  SIZ (dest) = size;
}
