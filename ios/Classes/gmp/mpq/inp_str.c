/* mpq_inp_str -- read an mpq from a FILE.

Copyright 2001 Free Software Foundation, Inc.
  */

#include <stdio.h>
#include <ctype.h>
#include "../gmp.h"
#include "../gmp-impl.h"


size_t
mpq_inp_str (mpq_ptr q, FILE *fp, int base)
{
  size_t  nread;
  int     c;

  if (fp == NULL)
    fp = stdin;

  SIZ(DEN(q)) = 1;
  PTR(DEN(q))[0] = 1;

  nread = mpz_inp_str (mpq_numref(q), fp, base);
  if (nread == 0)
    return 0;

  c = getc (fp);
  nread++;

  if (c == '/')
    {
      c = getc (fp);
      nread++;

      nread = mpz_inp_str_nowhite (mpq_denref(q), fp, base, c, nread);
      if (nread == 0)
	{
	  SIZ(NUM(q)) = 0;
	  SIZ(DEN(q)) = 1;
	  PTR(DEN(q))[0] = 1;
	}
    }
  else
    {
      ungetc (c, fp);
      nread--;
    }

  return nread;
}
