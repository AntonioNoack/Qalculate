/* mpf_inp_str(dest_float, stream, base) -- Input a number in base
   BASE from stdio stream STREAM and store the result in DEST_FLOAT.

Copyright 1996, 2000-2002, 2005 Free Software Foundation, Inc.
  */

#include <stdio.h>
#include <ctype.h>
#include "../gmp.h"
#include "../gmp-impl.h"

size_t
mpf_inp_str (mpf_ptr rop, FILE *stream, int base)
{
  char *str;
  size_t alloc_size, str_size;
  int c;
  int res;
  size_t nread;

  if (stream == 0)
    stream = stdin;

  alloc_size = 100;
  str = (char *) (*__gmp_allocate_func) (alloc_size);
  str_size = 0;
  nread = 0;

  /* Skip whitespace.  */
  do
    {
      c = getc (stream);
      nread++;
    }
  while (isspace (c));

  for (;;)
    {
      if (str_size >= alloc_size)
	{
	  size_t old_alloc_size = alloc_size;
	  alloc_size = alloc_size * 3 / 2;
	  str = (char *) (*__gmp_reallocate_func) (str, old_alloc_size, alloc_size);
	}
      if (c == EOF || isspace (c))
	break;
      str[str_size++] = c;
      c = getc (stream);
    }
  ungetc (c, stream);
  nread--;

  if (str_size >= alloc_size)
    {
      size_t old_alloc_size = alloc_size;
      alloc_size = alloc_size * 3 / 2;
      str = (char *) (*__gmp_reallocate_func) (str, old_alloc_size, alloc_size);
    }
  str[str_size] = 0;

  res = mpf_set_str (rop, str, base);
  (*__gmp_free_func) (str, alloc_size);

  if (res == -1)
    return 0;			/* error */

  return str_size + nread;
}
