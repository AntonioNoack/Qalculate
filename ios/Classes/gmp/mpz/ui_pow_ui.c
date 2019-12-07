/* mpz_ui_pow_ui -- ulong raised to ulong.

Copyright 2001, 2002 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"


void
mpz_ui_pow_ui (mpz_ptr r, unsigned long b, unsigned long e)
{
#if GMP_NAIL_BITS != 0
  if (b > GMP_NUMB_MAX)
    {
      mp_limb_t bb[2];
      bb[0] = b & GMP_NUMB_MASK;
      bb[1] = b >> GMP_NUMB_BITS;
      mpz_n_pow_ui (r, bb, (mp_size_t) 2, e);
    }
  else
#endif
    {
#ifdef _LONG_LONG_LIMB
      /* i386 gcc 2.95.3 doesn't recognise blimb can be eliminated when
	 mp_limb_t is an unsigned long, so only use a separate blimb when
	 necessary.  */
      mp_limb_t  blimb = b;
      mpz_n_pow_ui (r, &blimb, (mp_size_t) (b != 0), e);
#else
      mpz_n_pow_ui (r, &b,     (mp_size_t) (b != 0), e);
#endif
    }
}
