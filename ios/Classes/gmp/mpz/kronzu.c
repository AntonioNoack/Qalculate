/* mpz_kronecker_ui -- mpz+ulong Kronecker/Jacobi symbol.

Copyright 1999-2002 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"
#include "../gmp-impl.c"


int
mpz_kronecker_ui (mpz_srcptr a, unsigned long b)
{
  mp_srcptr  a_ptr;
  mp_size_t  a_size;
  mp_limb_t  a_rem;
  int        result_bit1;

  a_size = SIZ(a);
  if (a_size == 0)
    return JACOBI_0U (b);

  if (b > GMP_NUMB_MAX)
    {
      mp_limb_t  blimbs[2];
      mpz_t      bz;
      ALLOC(bz) = numberof (blimbs);
      PTR(bz) = blimbs;
      mpz_set_ui (bz, b);
      return mpz_kronecker (a, bz);
    }

  a_ptr = PTR(a);
  if ((b & 1) != 0)
    {
      result_bit1 = JACOBI_ASGN_SU_BIT1 (a_size, b);
    }
  else
    {
      mp_limb_t  a_low = a_ptr[0];
      int        twos;

      if (b == 0)
	return JACOBI_LS0 (a_low, a_size);   /* (a/0) */

      if (! (a_low & 1))
	return 0;  /* (even/even)=0 */

      /* (a/2)=(2/a) for a odd */
      count_trailing_zeros (twos, b);
      b >>= twos;
      result_bit1 = (JACOBI_TWOS_U_BIT1 (twos, a_low)
		     ^ JACOBI_ASGN_SU_BIT1 (a_size, b));
    }

  if (b == 1)
    return JACOBI_BIT1_TO_PN (result_bit1);  /* (a/1)=1 for any a */

  a_size = ABS(a_size);

  /* (a/b) = (a mod b / b) */
  JACOBI_MOD_OR_MODEXACT_1_ODD (result_bit1, a_rem, a_ptr, a_size, b);
  return mpn_jacobi_base (a_rem, (mp_limb_t) b, result_bit1);
}
