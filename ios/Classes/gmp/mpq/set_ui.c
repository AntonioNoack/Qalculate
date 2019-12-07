/* mpq_set_ui(dest,ulong_num,ulong_den) -- Set DEST to the rational number
   ULONG_NUM/ULONG_DEN.

Copyright 1991, 1994, 1995, 2001-2003 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpq_set_ui (mpq_t dest, unsigned long int num, unsigned long int den)
{
  if (GMP_NUMB_BITS < BITS_PER_ULONG)
    {
      if (num == 0)  /* Canonicalize 0/d to 0/1.  */
        den = 1;
      mpz_set_ui (mpq_numref (dest), num);
      mpz_set_ui (mpq_denref (dest), den);
      return;
    }

  if (num == 0)
    {
      /* Canonicalize 0/n to 0/1.  */
      den = 1;
      SIZ(NUM(dest)) = 0;
    }
  else
    {
      PTR(NUM(dest))[0] = num;
      SIZ(NUM(dest)) = 1;
    }

  PTR(DEN(dest))[0] = den;
  SIZ(DEN(dest)) = (den != 0);
}
