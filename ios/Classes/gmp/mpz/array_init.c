/* mpz_array_init (array, array_size, size_per_elem) --

Copyright 1991, 1993-1995, 2000-2002, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_array_init (mpz_ptr arr, mp_size_t arr_size, mp_size_t nbits)
{
  mp_ptr p;
  mp_size_t i;
  mp_size_t nlimbs;

  nlimbs = nbits / GMP_NUMB_BITS + 1;
  p = __GMP_ALLOCATE_FUNC_LIMBS (arr_size * nlimbs);

  for (i = 0; i < arr_size; i++)
    {
      ALLOC (&arr[i]) = nlimbs + 1; /* Yes, lie a little... */
      SIZ (&arr[i]) = 0;
      PTR (&arr[i]) = p + i * nlimbs;
    }
}
