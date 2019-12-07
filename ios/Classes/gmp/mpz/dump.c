/* mpz_dump - Dump an integer to stdout.

   THIS IS AN INTERNAL FUNCTION WITH A MUTABLE INTERFACE.  IT IS NOT SAFE TO
   CALL THIS FUNCTION DIRECTLY.  IN FACT, IT IS ALMOST GUARANTEED THAT THIS
   FUNCTION WILL CHANGE OR DISAPPEAR IN A FUTURE GNU MP RELEASE.


Copyright 1999-2001 Free Software Foundation, Inc.
  */

#include <stdio.h>
#include <string.h> /* for strlen */
#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_dump (mpz_srcptr u)
{
  char *str;

  str = mpz_get_str (0, 10, u);
  printf ("%s\n", str);
  (*__gmp_free_func) (str, strlen (str) + 1);
}
