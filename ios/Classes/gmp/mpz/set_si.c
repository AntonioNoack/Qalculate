/* mpz_set_si(dest,val) -- Assign DEST with a small value VAL.

Copyright 1991, 1993-1995, 2000-2002, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_set_si (mpz_ptr dest, signed long int val)
{
  mp_size_t size;
  mp_limb_t vl;

  vl = (mp_limb_t) ABS_CAST (unsigned long int, val);

  PTR (dest)[0] = vl & GMP_NUMB_MASK;
  size = vl != 0;

#if GMP_NAIL_BITS != 0
  if (vl > GMP_NUMB_MAX)
    {
      MPZ_REALLOC (dest, 2);
      PTR (dest)[1] = vl >> GMP_NUMB_BITS;
      size = 2;
    }
#endif

  SIZ (dest) = val >= 0 ? size : -size;
}
