/* mpq_clears() -- Clear multiple mpq_t variables.

Copyright 2009, 2014, 2015 Free Software Foundation, Inc.
  */

#include <stdarg.h>
#include "../gmp.h"
#include "../gmp-impl.h"

void
mpq_clears (mpq_ptr x, ...)
{
  va_list  ap;

  va_start (ap, x);

  while (x != NULL)
    {
      __GMP_FREE_FUNC_LIMBS (PTR(NUM(x)), ALLOC(NUM(x)));
      __GMP_FREE_FUNC_LIMBS (PTR(DEN(x)), ALLOC(DEN(x)));
      x = va_arg (ap, mpq_ptr);
    }

  va_end (ap);
}
