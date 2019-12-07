/* mpz_init_set_ui(dest,val) -- Make a new multiple precision in DEST and
   assign VAL to the new number.

Copyright 1991, 1993-1995, 2000-2002, 2004, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_init_set_ui (mpz_ptr dest, unsigned long int val)
{
  mp_size_t size;

#if BITS_PER_ULONG > GMP_NUMB_BITS  /* avoid warnings about shift amount */
  if (val > GMP_NUMB_MAX)
    {
      ALLOC (dest) = 2;
      PTR (dest) = __GMP_ALLOCATE_FUNC_LIMBS (2);
      PTR (dest)[1] = val >> GMP_NUMB_BITS;
      size = 2;
    }
  else
#endif
    {
      ALLOC (dest) = 1;
      PTR (dest) = __GMP_ALLOCATE_FUNC_LIMBS (1);

      size = val != 0;
    }
  PTR (dest)[0] = val & GMP_NUMB_MASK;

  SIZ (dest) = size;
}
