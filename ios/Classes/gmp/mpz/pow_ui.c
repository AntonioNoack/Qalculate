/* mpz_pow_ui -- mpz raised to ulong.

Copyright 2001, 2008 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_pow_ui (mpz_ptr r, mpz_srcptr b, unsigned long int e)
{
  /* We test some small exponents here, mainly to avoid the overhead of
     mpz_n_pow_ui for small bases and exponents.  */
  switch (e)
    {
    case 0:
      mpz_set_ui (r, 1);
      break;
    case 1:
      mpz_set (r, b);
      break;
    case 2:
      mpz_mul (r, b, b);
      break;
    default:
      mpz_n_pow_ui (r, PTR(b), (mp_size_t) SIZ(b), e);
    }
}
