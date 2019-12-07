/* mpf_clears() -- Clear multiple mpf_t variables.

Copyright 2009, 2014, 2015 Free Software Foundation, Inc.
  */

#include <stdarg.h>
#include "../gmp.h"
#include "../gmp-impl.h"

void
mpf_clears (mpf_ptr x, ...)
{
  va_list  ap;

  va_start (ap, x);

  while (x != NULL)
    {
      __GMP_FREE_FUNC_LIMBS (PTR(x), PREC(x) + 1);
      x = va_arg (ap, mpf_ptr);
    }

  va_end (ap);
}
