/* mpz_primorial_ui(RESULT, N) -- Set RESULT to N# the product of primes <= N.

Contributed to the GNU project by Marco Bodrato.

Copyright 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

/* TODO: Remove duplicated constants / macros / static functions...
 */

/*************************************************************/
/* Section macros: common macros, for swing/fac/bin (&sieve) */
/*************************************************************/

#define FACTOR_LIST_STORE(P, PR, MAX_PR, VEC, I)		\
  do {								\
    if ((PR) > (MAX_PR)) {					\
      (VEC)[(I)++] = (PR);					\
      (PR) = (P);						\
    } else							\
      (PR) *= (P);						\
  } while (0)

#define LOOP_ON_SIEVE_CONTINUE(prime,end,sieve)			\
    __max_i = (end);						\
								\
    do {							\
      ++__i;							\
      if (((sieve)[__index] & __mask) == 0)			\
	{							\
	  (prime) = id_to_n(__i)

#define LOOP_ON_SIEVE_BEGIN(prime,start,end,off,sieve)		\
  do {								\
    mp_limb_t __mask, __index, __max_i, __i;			\
								\
    __i = (start)-(off);					\
    __index = __i / GMP_LIMB_BITS;				\
    __mask = CNST_LIMB(1) << (__i % GMP_LIMB_BITS);		\
    __i += (off);						\
								\
    LOOP_ON_SIEVE_CONTINUE(prime,end,sieve)

#define LOOP_ON_SIEVE_STOP					\
	}							\
      __mask = __mask << 1 | __mask >> (GMP_LIMB_BITS-1);	\
      __index += __mask & 1;					\
    }  while (__i <= __max_i)					\

#define LOOP_ON_SIEVE_END					\
    LOOP_ON_SIEVE_STOP;						\
  } while (0)

/*********************************************************/
/* Section sieve: sieving functions and tools for primes */
/*********************************************************/

#if 0
static mp_limb_t
bit_to_n (mp_limb_t bit) { return (bit*3+4)|1; }
#endif

/* id_to_n (x) = bit_to_n (x-1) = (id*3+1)|1*/
#define id_to_n(id) ((id)*3+1+((id)&1))

/* n_to_bit (n) = ((n-1)&(-CNST_LIMB(2)))/3U-1 */
#define n_to_bit(n) ((((n)-5)|1)/3U)
// static mp_limb_t n_to_bit (mp_limb_t n) { return ((n-5)|1)/3U; }

#if WANT_ASSERT
static mp_size_t
primesieve_size (mp_limb_t n) { return n_to_bit(n) / GMP_LIMB_BITS + 1; }
#endif

/*********************************************************/
/* Section primorial: implementation                     */
/*********************************************************/

void
mpz_primorial_ui (mpz_ptr x, unsigned long n)
{
  static const mp_limb_t table[] = { 1, 1, 2, 6, 6 };

  ASSERT (n <= GMP_NUMB_MAX);

  if (n < numberof (table))
    {
      PTR (x)[0] = table[n];
      SIZ (x) = 1;
    }
  else
    {
      mp_limb_t *sieve, *factors;
      mp_size_t size;
      mp_limb_t prod;
      mp_limb_t j;
      TMP_DECL;

      size = 1 + n / GMP_NUMB_BITS + n / (2*GMP_NUMB_BITS);
      ASSERT (size >= primesieve_size (n));
      sieve = MPZ_NEWALLOC (x, size);
      size = (gmp_primesieve (sieve, n) + 1) / log_n_max (n) + 1;

      TMP_MARK;
      factors = TMP_ALLOC_LIMBS (size);

      j = 0;

      prod = table[numberof (table)-1];

      /* Store primes from 5 to n */
      {
	mp_limb_t prime, max_prod;

	max_prod = GMP_NUMB_MAX / n;

	LOOP_ON_SIEVE_BEGIN (prime, n_to_bit(numberof (table)), n_to_bit (n), 0, sieve);
	FACTOR_LIST_STORE (prime, prod, max_prod, factors, j);
	LOOP_ON_SIEVE_END;
      }

      if (LIKELY (j != 0))
	{
	  factors[j++] = prod;
	  mpz_prodlimbs (x, factors, j);
	}
      else
	{
	  PTR (x)[0] = prod;
	  SIZ (x) = 1;
	}

      TMP_FREE;
    }
}
