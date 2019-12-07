/* mpz_inits() -- Initialize multiple mpz_t variables and set them to 0.

Copyright 2009, 2015 Free Software Foundation, Inc.
  */

#include <stdarg.h>
#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_inits (mpz_ptr x, ...)
{
  va_list  ap;

  va_start (ap, x);

  while (x != NULL)
    {
      ALLOC (x) = 1;
      PTR (x) = __GMP_ALLOCATE_FUNC_LIMBS (1);
      SIZ (x) = 0;

#ifdef __CHECKER__
      /* let the low limb look initialized, for the benefit of mpz_get_ui etc */
      PTR (x)[0] = 0;
#endif

      x = va_arg (ap, mpz_ptr);
    }

  va_end (ap);
}
