/* mpz_init2 -- initialize mpz, with requested size in bits.

Copyright 2001, 2002, 2008 Free Software Foundation, Inc.
  */

#include <stdlib.h>
#include <stdio.h>
#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_init2 (mpz_ptr x, mp_bitcnt_t bits)
{
  mp_size_t  new_alloc;

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

  PTR(x) = __GMP_ALLOCATE_FUNC_LIMBS (new_alloc);
  ALLOC(x) = new_alloc;
  SIZ(x) = 0;

#ifdef __CHECKER__
  /* let the low limb look initialized, for the benefit of mpz_get_ui etc */
  PTR(x)[0] = 0;
#endif
}
