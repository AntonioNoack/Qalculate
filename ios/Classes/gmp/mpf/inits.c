/* mpf_inits() -- Initialize multiple mpf_t variables and set them to 0.

Copyright 2009, 2015 Free Software Foundation, Inc.
  */

#include <stdarg.h>
#include "../gmp.h"
#include "../gmp-impl.h"

void
mpf_inits (mpf_ptr x, ...)
{
  va_list  ap;

  va_start (ap, x);

  while (x != NULL)
    {
      mpf_init (x);
      x = va_arg (ap, mpf_ptr);
    }

  va_end (ap);
}
