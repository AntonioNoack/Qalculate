/* _mpz_realloc -- make the mpz_t have NEW_ALLOC digits allocated.

Copyright 1991, 1993-1995, 2000, 2001, 2008 Free Software Foundation, Inc.
  */

#include <stdlib.h>
#include <stdio.h>
#include "../gmp.h"
#include "../gmp-impl.h"

void *_mpz_realloc (mpz_ptr m, mp_size_t new_alloc){
  mp_ptr mp;

  /* Never allocate zero space. */
  new_alloc = MAX (new_alloc, 1);

  if (sizeof (mp_size_t) == sizeof (int))
    {
      if (UNLIKELY (new_alloc > ULONG_MAX / GMP_NUMB_BITS))
	{
	  fprintf (stderr, "gmp: overflow in mpz type\n");
	  abort ();
	}
    }
  else
    {
      if (UNLIKELY (new_alloc > INT_MAX))
	{
	  fprintf (stderr, "gmp: overflow in mpz type\n");
	  abort ();
	}
    }

  mp = __GMP_REALLOCATE_FUNC_LIMBS (PTR(m), ALLOC(m), new_alloc);
  PTR(m) = mp;
  ALLOC(m) = new_alloc;

  /* Don't create an invalid number; if the current value doesn't fit after
     reallocation, clear it to 0.  */
  if (ABSIZ(m) > new_alloc)
    SIZ(m) = 0;

  return (void *) mp;
}
