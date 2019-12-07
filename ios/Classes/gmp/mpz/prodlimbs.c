/* mpz_prodlimbs(RESULT, V, LEN) -- Set RESULT to V[0]*V[1]*...*V[LEN-1].

Contributed to the GNU project by Marco Bodrato.

THE FUNCTION IN THIS FILE IS INTERNAL WITH A MUTABLE INTERFACE.
IT IS ONLY SAFE TO REACH IT THROUGH DOCUMENTED INTERFACES.
IN FACT, IT IS ALMOST GUARANTEED THAT IT WILL CHANGE OR
DISAPPEAR IN A FUTURE GNU MP RELEASE.

Copyright 2010-2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

/*********************************************************/
/* Section list-prod: product of a list -> mpz_t         */
/*********************************************************/

/* FIXME: should be tuned */
#ifndef RECURSIVE_PROD_THRESHOLD
#define RECURSIVE_PROD_THRESHOLD (MUL_TOOM22_THRESHOLD)
#endif

/* Computes the product of the j>1 limbs pointed by factors, puts the
 * result in x. It assumes that all limbs are non-zero. Above
 * Karatsuba's threshold it uses a binary splitting strategy, to gain
 * speed by the asymptotically fast multiplication algorithms.
 *
 * The list in  {factors, j} is overwritten.
 * Returns the size of the result
 */

mp_size_t
mpz_prodlimbs (mpz_ptr x, mp_ptr factors, mp_size_t j)
{
  mp_limb_t cy;
  mp_size_t size, i;
  mp_ptr    prod;

  ASSERT (j > 1);
  ASSERT (RECURSIVE_PROD_THRESHOLD > 3);

  if (BELOW_THRESHOLD (j, RECURSIVE_PROD_THRESHOLD)) {
    j--;
    size = 1;

    for (i = 1; i < j; i++)
      {
	cy = mpn_mul_1 (factors, factors, size, factors[i]);
	factors[size] = cy;
	size += cy != 0;
      };

    prod = MPZ_NEWALLOC (x, size + 1);

    cy = mpn_mul_1 (prod, factors, size, factors[i]);
    prod[size] = cy;
    return SIZ (x) = size + (cy != 0);
  } else {
    mpz_t x1, x2;
    TMP_DECL;

    i = j >> 1;
    j -= i;
    TMP_MARK;

    MPZ_TMP_INIT (x2, j);

    PTR (x1) = factors + i;
    ALLOC (x1) = j;
    j = mpz_prodlimbs (x2, factors + i, j);
    i = mpz_prodlimbs (x1, factors, i);
    size = i + j;
    prod = MPZ_NEWALLOC (x, size);
    if (i >= j)
      cy = mpn_mul (prod, PTR(x1), i, PTR(x2), j);
    else
      cy = mpn_mul (prod, PTR(x2), j, PTR(x1), i);
    TMP_FREE;

    return SIZ (x) = size - (cy == 0);
  }
}
