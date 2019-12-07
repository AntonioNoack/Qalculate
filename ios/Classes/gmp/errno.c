/* gmp_errno, __gmp_exception -- exception handling and reporting.

   THE FUNCTIONS IN THIS FILE, APART FROM gmp_errno, ARE FOR INTERNAL USE
   ONLY.  THEY'RE ALMOST CERTAIN TO BE SUBJECT TO INCOMPATIBLE CHANGES OR
   DISAPPEAR COMPLETELY IN FUTURE GNU MP RELEASES.

Copyright 2000, 2001, 2003 Free Software Foundation, Inc.
  */

#include <stdlib.h>
#include "gmp.h"
#include "gmp-impl.h"
#include "mp_bpl.c"

int gmp_errno = 0;


/* The deliberate divide by zero triggers an exception on most systems.  On
   those where it doesn't, for example power and powerpc, use abort instead.

   Enhancement: Perhaps raise(SIGFPE) (or the same with kill()) would be
   better than abort.  Perhaps it'd be possible to get the BSD style
   FPE_INTDIV_TRAP parameter in there too.  */

void
__gmp_exception (int error_bit)
{
  gmp_errno |= error_bit;
  __gmp_junk = 10 / __gmp_0;
  abort ();
}


/* These functions minimize the amount of code required in functions raising
   exceptions.  Since they're "noreturn" and don't take any parameters, a
   test and call might even come out as a simple conditional jump.  */
void
__gmp_sqrt_of_negative (void)
{
  __gmp_exception (GMP_ERROR_SQRT_OF_NEGATIVE);
}
void
__gmp_divide_by_zero (void)
{
  __gmp_exception (GMP_ERROR_DIVISION_BY_ZERO);
}