/* mpz_init_set_str(string, base) -- Convert the \0-terminated string STRING in
   base BASE to a multiple precision integer.  Allow white space in the string.
   If BASE == 0 determine the base in the C standard way, i.e.  0xhh...h means
   base 16, 0oo...o means base 8, otherwise assume base 10.

Copyright 1991, 1993-1995, 2000-2002, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

int
mpz_init_set_str (mpz_ptr x, const char *str, int base)
{
  ALLOC (x) = 1;
  PTR (x) = __GMP_ALLOCATE_FUNC_LIMBS (1);

  /* if str has no digits mpz_set_str leaves x->_mp_size unset */
  SIZ (x) = 0;

#ifdef __CHECKER__
  /* let the low limb look initialized, for the benefit of mpz_get_ui etc */
  PTR (x)[0] = 0;
#endif

  return mpz_set_str (x, str, base);
}
