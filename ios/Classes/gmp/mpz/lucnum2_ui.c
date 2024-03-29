/* mpz_lucnum2_ui -- calculate Lucas numbers.

Copyright 2001, 2003, 2005, 2012 Free Software Foundation, Inc.
  */

#include <stdio.h>
#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"


void
mpz_lucnum2_ui (mpz_ptr ln, mpz_ptr lnsub1, unsigned long n)
{
  mp_ptr     lp, l1p, f1p;
  mp_size_t  size;
  mp_limb_t  c;
  TMP_DECL;

  ASSERT (ln != lnsub1);

  /* handle small n quickly, and hide the special case for L[-1]=-1 */
  if (n <= FIB_TABLE_LUCNUM_LIMIT)
    {
      mp_limb_t  f  = FIB_TABLE (n);
      mp_limb_t  f1 = FIB_TABLE ((int) n - 1);

      /* L[n] = F[n] + 2F[n-1] */
      PTR(ln)[0] = f + 2*f1;
      SIZ(ln) = 1;

      /* L[n-1] = 2F[n] - F[n-1], but allow for L[-1]=-1 */
      PTR(lnsub1)[0] = (n == 0 ? 1 : 2*f - f1);
      SIZ(lnsub1) = (n == 0 ? -1 : 1);

      return;
    }

  TMP_MARK;
  size = MPN_FIB2_SIZE (n);
  f1p = TMP_ALLOC_LIMBS (size);

  lp  = MPZ_REALLOC (ln,     size+1);
  l1p = MPZ_REALLOC (lnsub1, size+1);

  size = mpn_fib2_ui (l1p, f1p, n);

  /* L[n] = F[n] + 2F[n-1] */
#if HAVE_NATIVE_mpn_addlsh1_n
  c = mpn_addlsh1_n (lp, l1p, f1p, size);
#else
  c = mpn_lshift (lp, f1p, size, 1);
  c += mpn_add_n (lp, lp, l1p, size);
#endif
  lp[size] = c;
  SIZ(ln) = size + (c != 0);

  /* L[n-1] = 2F[n] - F[n-1] */
  c = mpn_lshift (l1p, l1p, size, 1);
  c -= mpn_sub_n (l1p, l1p, f1p, size);
  ASSERT ((mp_limb_signed_t) c >= 0);
  l1p[size] = c;
  SIZ(lnsub1) = size + (c != 0);

  TMP_FREE;
}
