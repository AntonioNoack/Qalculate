/* mpz_realloc2 -- change allocated data size.

Copyright 2001, 2002, 2008 Free Software Foundation, Inc.
  */

#include <stdlib.h>
#include <stdio.h>
#include "../gmp.h"
#include "../gmp-impl.h"

void
mpz_realloc2 (mpz_ptr m, mp_bitcnt_t bits)
{
  mp_size_t new_alloc;

  bits -= (bits != 0);		/* Round down, except if 0 */
  new_alloc = 1 + bits / GMP_NUMB_BITS;

  if (sizeof (unsigned long) > sizeof (int)) /* param vs _mp_size field */
    {
      if (UNLIKELY (new_alloc > INT_MAX))
	{
	  fprintf (stderr, "gmp: overflow in mpz type\n");
	  abort ();
	}
    }

  PTR(m) = __GMP_REALLOCATE_FUNC_LIMBS (PTR(m), ALLOC(m), new_alloc);
  ALLOC(m) = new_alloc;

  /* Don't create an invalid number; if the current value doesn't fit after
     reallocation, clear it to 0.  */
  if (ABSIZ(m) > new_alloc)
    SIZ(m) = 0;
}
