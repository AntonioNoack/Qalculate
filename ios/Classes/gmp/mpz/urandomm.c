/* mpz_urandomm (rop, state, n) -- Generate a uniform pseudorandom
   integer in the range 0 to N-1, using STATE as the random state
   previously initialized by a call to gmp_randinit().

Copyright 2000, 2002, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h" /* for count_leading_zeros */
#include "../gmp-impl.c"


#define MAX_URANDOMM_ITER  80

void
mpz_urandomm (mpz_ptr rop, gmp_randstate_t rstate, mpz_srcptr n)
{
  mp_ptr rp, np, nlast;
  mp_size_t nbits, size;
  int count;
  int pow2;
  int cmp;
  TMP_DECL;

  size = ABSIZ (n);
  if (UNLIKELY (size == 0))
    DIVIDE_BY_ZERO;

  nlast = &PTR (n)[size - 1];

  /* Detect whether n is a power of 2.  */
  pow2 = POW2_P (*nlast);
  if (pow2 != 0)
    for (np = PTR (n); np < nlast; np++)
      if (*np != 0)
	{
	  pow2 = 0;		/* Mark n as `not a power of two'.  */
	  break;
	}

  count_leading_zeros (count, *nlast);
  nbits = size * GMP_NUMB_BITS - (count - GMP_NAIL_BITS) - pow2;
  if (nbits == 0)		/* nbits == 0 means that n was == 1.  */
    {
      SIZ (rop) = 0;
      return;
    }

  TMP_MARK;
  np = PTR (n);
  if (rop == n)
    {
      mp_ptr tp;
      tp = TMP_ALLOC_LIMBS (size);
      MPN_COPY (tp, np, size);
      np = tp;
    }

  /* Here the allocated size can be one too much if n is a power of
     (2^GMP_NUMB_BITS) but it's convenient for using mpn_cmp below.  */
  rp = MPZ_REALLOC (rop, size);
  /* Clear last limb to prevent the case in which size is one too much.  */
  rp[size - 1] = 0;

  count = MAX_URANDOMM_ITER;	/* Set iteration count limit.  */
  do
    {
      _gmp_rand (rp, rstate, nbits);
      MPN_CMP (cmp, rp, np, size);
    }
  while (cmp >= 0 && --count != 0);

  if (count == 0)
    /* Too many iterations; return result mod n == result - n */
    mpn_sub_n (rp, rp, np, size);

  MPN_NORMALIZE (rp, size);
  SIZ (rop) = size;
  TMP_FREE;
}
