/* mpq_set_str -- string to mpq conversion.

Copyright 2001, 2002 Free Software Foundation, Inc.
  */

#include <stdio.h>
#include <string.h>
#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"


/* FIXME: Would like an mpz_set_mem (or similar) accepting a pointer and
   length so we wouldn't have to copy the numerator just to null-terminate
   it.  */

int
mpq_set_str (mpq_ptr q, const char *str, int base)
{
  const char  *slash;
  char        *num;
  size_t      numlen;
  int         ret;

  slash = strchr (str, '/');
  if (slash == NULL)
    {
      SIZ(DEN(q)) = 1;
      PTR(DEN(q))[0] = 1;

      return mpz_set_str (mpq_numref(q), str, base);
    }

  numlen = slash - str;
  num = __GMP_ALLOCATE_FUNC_TYPE (numlen+1, char);
  memcpy (num, str, numlen);
  num[numlen] = '\0';
  ret = mpz_set_str (mpq_numref(q), num, base);
  (*__gmp_free_func) (num, numlen+1);

  if (ret != 0)
    return ret;

  return mpz_set_str (mpq_denref(q), slash+1, base);
}
