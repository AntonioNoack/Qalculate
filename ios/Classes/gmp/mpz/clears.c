/* mpz_clears() -- Clear multiple mpz_t variables.

Copyright 2009, 2014, 2015 Free Software Foundation, Inc.
  */

#include <stdarg.h>
#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_clears (mpz_ptr x, ...)
{
  va_list  ap;

  va_start (ap, x);

  while (x != NULL)
    {
      __GMP_FREE_FUNC_LIMBS (PTR (x), ALLOC(x));
      x = va_arg (ap, mpz_ptr);
    }

  va_end (ap);
}
