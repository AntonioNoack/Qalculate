/* mpq_inits() -- Initialize multiple mpq_t variables and set them to 0.

Copyright 2009, 2015 Free Software Foundation, Inc.
  */

#include <stdarg.h>
#include "../gmp.h"
#include "../gmp-impl.h"

void
mpq_inits (mpq_ptr x, ...)
{
  va_list  ap;

  va_start (ap, x);

  while (x != NULL)
    {
      mpq_init (x);
      x = va_arg (ap, mpq_ptr);
    }

  va_end (ap);
}
