/* mpz_sqrt(root, u) --  Set ROOT to floor(sqrt(U)).

Copyright 1991, 1993, 1994, 1996, 2000, 2001, 2005, 2012 Free Software
Foundation, Inc.
  */

#include <stdio.h> /* for NULL */
#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_sqrt (mpz_ptr root, mpz_srcptr op)
{
  mp_size_t op_size, root_size;
  mp_ptr root_ptr, op_ptr;

  op_size = SIZ (op);
  if (UNLIKELY (op_size <= 0))
    {
      if (op_size < 0)
	SQRT_OF_NEGATIVE;
      SIZ(root) = 0;
      return;
    }

  /* The size of the root is accurate after this simple calculation.  */
  root_size = (op_size + 1) / 2;
  SIZ (root) = root_size;

  op_ptr = PTR (op);

  if (root == op)
    {
      /* Allocate temp space for the root, which we then copy to the
	 shared OP/ROOT variable.  */
      TMP_DECL;
      TMP_MARK;

      root_ptr = TMP_ALLOC_LIMBS (root_size);
      mpn_sqrtrem (root_ptr, NULL, op_ptr, op_size);

      MPN_COPY (op_ptr, root_ptr, root_size);

      TMP_FREE;
    }
  else
    {
      root_ptr = MPZ_NEWALLOC (root, root_size);

      mpn_sqrtrem (root_ptr, NULL, op_ptr, op_size);
    }
}
