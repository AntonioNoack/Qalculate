/* mpf_dump -- Dump a float to stdout.

   THIS IS AN INTERNAL FUNCTION WITH A MUTABLE INTERFACE.  IT IS NOT SAFE TO
   CALL THIS FUNCTION DIRECTLY.  IN FACT, IT IS ALMOST GUARANTEED THAT THIS
   FUNCTION WILL CHANGE OR DISAPPEAR IN A FUTURE GNU MP RELEASE.


Copyright 1993-1995, 2000, 2001 Free Software Foundation, Inc.
  */

#include <stdio.h>
#include <string.h> /* for strlen */
#include "../gmp.h"
#include "../gmp-impl.h"

void
mpf_dump (mpf_srcptr u)
{
  mp_exp_t exp;
  char *str;

  str = mpf_get_str (0, &exp, 10, 0, u);
  if (str[0] == '-')
    printf ("-0.%se%ld\n", str + 1, exp);
  else
    printf ("0.%se%ld\n", str, exp);
  (*__gmp_free_func) (str, strlen (str) + 1);
}
