/* mpz_init_set (src_integer) -- Make a new multiple precision number with
   a value copied from SRC_INTEGER.

Copyright 1991, 1993, 1994, 1996, 2000-2002, 2012 Free Software Foundation,
Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_init_set (mpz_ptr w, mpz_srcptr u)
{
  mp_ptr wp, up;
  mp_size_t usize, size;

  usize = SIZ (u);
  size = ABS (usize);

  ALLOC (w) = MAX (size, 1);
  PTR (w) = __GMP_ALLOCATE_FUNC_LIMBS (ALLOC (w));

  wp = PTR (w);
  up = PTR (u);

  MPN_COPY (wp, up, size);
  SIZ (w) = usize;

#ifdef __CHECKER__
  /* let the low limb look initialized, for the benefit of mpz_get_ui etc */
  if (size == 0)
    wp[0] = 0;
#endif
}
