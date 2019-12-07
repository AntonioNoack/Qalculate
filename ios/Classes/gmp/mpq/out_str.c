/* mpq_out_str(stream,base,integer) */

/*
Copyright 2000, 2001 Free Software Foundation, Inc.
  */

#include <stdio.h>
#include "../gmp.h"
#include "../gmp-impl.h"


size_t
mpq_out_str (FILE *stream, int base, mpq_srcptr q)
{
  size_t  written;

  if (stream == NULL)
    stream = stdout;

  written = mpz_out_str (stream, base, mpq_numref (q));

  if (mpz_cmp_ui (mpq_denref (q), 1) != 0)
    {
      putc ('/', stream);
      written += 1 + mpz_out_str (stream, base, mpq_denref (q));
    }

  return ferror (stream) ? 0 : written;
}
