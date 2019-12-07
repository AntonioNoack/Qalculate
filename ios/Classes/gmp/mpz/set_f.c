/* mpz_set_f (dest_integer, src_float) -- Assign DEST_INTEGER from SRC_FLOAT.

Copyright 1996, 2001, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"


void
mpz_set_f (mpz_ptr w, mpf_srcptr u)
{
  mp_ptr    wp, up;
  mp_size_t size;
  mp_exp_t  exp;

  /* abs(u)<1 truncates to zero */
  exp = EXP (u);
  if (exp <= 0)
    {
      SIZ(w) = 0;
      return;
    }

  wp = MPZ_REALLOC (w, exp);
  up = PTR(u);

  size = SIZ (u);
  SIZ(w) = (size >= 0 ? exp : -exp);
  size = ABS (size);

  if (exp > size)
    {
      /* pad with low zeros to get a total "exp" many limbs */
      mp_size_t  zeros = exp - size;
      MPN_ZERO (wp, zeros);
      wp += zeros;
    }
  else
    {
      /* exp<=size, truncate to the high "exp" many limbs */
      up += (size - exp);
      size = exp;
    }

  MPN_COPY (wp, up, size);
}
