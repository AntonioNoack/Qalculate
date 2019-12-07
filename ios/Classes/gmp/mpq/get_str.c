/* mpq_get_str -- mpq to string conversion.

Copyright 2001, 2002, 2006, 2011 Free Software Foundation, Inc.
  */

#include <stdio.h>
#include <string.h>
#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"
#include "../gmp-impl.c"

char *
mpq_get_str (char *str, int base, mpq_srcptr q)
{
  size_t  str_alloc, len;

  if (base > 62 || base < -36)
    return NULL;

  str_alloc = 0;
  if (str == NULL)
    {
      /* This is an overestimate since we don't bother checking how much of
	 the high limbs of num and den are used.  +2 for rounding up the
	 chars per bit of num and den.  +3 for sign, slash and '\0'.  */
      DIGITS_IN_BASE_PER_LIMB (str_alloc, ABSIZ(NUM(q)) + SIZ(DEN(q)), ABS(base));
      str_alloc += 6;

      str = (char *) (*__gmp_allocate_func) (str_alloc);
    }

  mpz_get_str (str, base, mpq_numref(q));
  len = strlen (str);
  if (! MPZ_EQUAL_1_P (mpq_denref (q)))
    {
      str[len++] = '/';
      mpz_get_str (str+len, base, mpq_denref(q));
      len += strlen (str+len);
    }

  ASSERT (len == strlen(str));
  ASSERT (str_alloc == 0 || len+1 <= str_alloc);
  ASSERT (len+1 <=  /* size recommended to applications */
	  mpz_sizeinbase (mpq_numref(q), ABS(base)) +
	  mpz_sizeinbase (mpq_denref(q), ABS(base)) + 3);

  if (str_alloc != 0)
    __GMP_REALLOCATE_FUNC_MAYBE_TYPE (str, str_alloc, len+1, char);

  return str;
}
