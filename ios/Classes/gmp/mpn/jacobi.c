/* jacobi.c

   THE FUNCTIONS IN THIS FILE ARE INTERNAL WITH MUTABLE INTERFACES.  IT IS ONLY
   SAFE TO REACH THEM THROUGH DOCUMENTED INTERFACES.  IN FACT, IT IS ALMOST
   GUARANTEED THAT THEY'LL CHANGE OR DISAPPEAR IN A FUTURE GNU MP RELEASE.

Copyright 1996, 1998, 2000-2004, 2008, 2010, 2011 Free Software Foundation,
Inc.
  */

#ifndef JACOBI_XXX_C
#define JACOBI_XXX_C

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"
#include "../../struct_helper/hgcd_matrix.h"
#include "../gmp-impl.c"
#include "jacobi.h"

static void
jacobi_hook (void *p, mp_srcptr gp, mp_size_t gn,
	     mp_srcptr qp, mp_size_t qn, int d)
{
  unsigned *bitsp = (unsigned *) p;

  if (gp)
    {
      ASSERT (gn > 0);
      if (gn != 1 || gp[0] != 1)
	{
	  *bitsp = BITS_FAIL;
	  return;
	}
    }

  if (qp)
    {
      ASSERT (qn > 0);
      ASSERT (d >= 0);
      *bitsp = mpn_jacobi_update (*bitsp, d, qp[0] & 3);
    }
}

int
mpn_jacobi_n (mp_ptr ap, mp_ptr bp, mp_size_t n, unsigned bits)
{
  mp_size_t scratch;
  mp_size_t matrix_scratch;
  mp_ptr tp;

  TMP_DECL;

  ASSERT (n > 0);
  ASSERT ( (ap[n-1] | bp[n-1]) > 0);
  ASSERT ( (bp[0] | ap[0]) & 1);

  /* FIXME: Check for small sizes first, before setting up temporary
     storage etc. */
  scratch = MPN_GCD_SUBDIV_STEP_ITCH(n);

  if (ABOVE_THRESHOLD (n, GCD_DC_THRESHOLD))
    {
      mp_size_t hgcd_scratch;
      mp_size_t update_scratch;
      mp_size_t p = CHOOSE_P (n);
      mp_size_t dc_scratch;

      matrix_scratch = MPN_HGCD_MATRIX_INIT_ITCH (n - p);
      hgcd_scratch = mpn_hgcd_itch (n - p);
      update_scratch = p + n - 1;

      dc_scratch = matrix_scratch + MAX(hgcd_scratch, update_scratch);
      if (dc_scratch > scratch)
	scratch = dc_scratch;
    }

  TMP_MARK;
  tp = TMP_ALLOC_LIMBS(scratch);

  while (ABOVE_THRESHOLD (n, JACOBI_DC_THRESHOLD))
    {
      struct hgcd_matrix M;
      mp_size_t p = 2*n/3;
      mp_size_t matrix_scratch = MPN_HGCD_MATRIX_INIT_ITCH (n - p);
      mp_size_t nn;
      mpn_hgcd_matrix_init (&M, n - p, tp);

      nn = mpn_hgcd_jacobi (ap + p, bp + p, n - p, &M, &bits,
			    tp + matrix_scratch);
      if (nn > 0)
	{
	  ASSERT (M.n <= (n - p - 1)/2);
	  ASSERT (M.n + p <= (p + n - 1) / 2);
	  /* Temporary storage 2 (p + M->n) <= p + n - 1. */
	  n = mpn_hgcd_matrix_adjust (&M, p + nn, ap, bp, p, tp + matrix_scratch);
	}
      else
	{
	  /* Temporary storage n */
	  n = mpn_gcd_subdiv_step (ap, bp, n, 0, jacobi_hook, &bits, tp);
	  if (!n)
	    {
	      TMP_FREE;
	      return bits == BITS_FAIL ? 0 : mpn_jacobi_finish (bits);
	    }
	}
    }

  while (n > 2)
    {
      struct hgcd_matrix1 M;
      mp_limb_t ah, al, bh, bl;
      mp_limb_t mask;

      mask = ap[n-1] | bp[n-1];
      ASSERT (mask > 0);

      if (mask & GMP_NUMB_HIGHBIT)
	{
	  ah = ap[n-1]; al = ap[n-2];
	  bh = bp[n-1]; bl = bp[n-2];
	}
      else
	{
	  int shift;

	  count_leading_zeros (shift, mask);
	  ah = MPN_EXTRACT_NUMB (shift, ap[n-1], ap[n-2]);
	  al = MPN_EXTRACT_NUMB (shift, ap[n-2], ap[n-3]);
	  bh = MPN_EXTRACT_NUMB (shift, bp[n-1], bp[n-2]);
	  bl = MPN_EXTRACT_NUMB (shift, bp[n-2], bp[n-3]);
	}

      /* Try an mpn_nhgcd2 step */
      if (mpn_hgcd2_jacobi (ah, al, bh, bl, &M, &bits))
	{
	  n = mpn_matrix22_mul1_inverse_vector (&M, tp, ap, bp, n);
	  MP_PTR_SWAP (ap, tp);
	}
      else
	{
	  /* mpn_hgcd2 has failed. Then either one of a or b is very
	     small, or the difference is very small. Perform one
	     subtraction followed by one division. */
	  n = mpn_gcd_subdiv_step (ap, bp, n, 0, &jacobi_hook, &bits, tp);
	  if (!n)
	    {
	      TMP_FREE;
	      return bits == BITS_FAIL ? 0 : mpn_jacobi_finish (bits);
	    }
	}
    }

  if (bits >= 16)
    MP_PTR_SWAP (ap, bp);

  ASSERT (bp[0] & 1);

  if (n == 1)
    {
      mp_limb_t al, bl;
      al = ap[0];
      bl = bp[0];

      TMP_FREE;
      if (bl == 1)
	return 1 - 2*(bits & 1);
      else
	return mpn_jacobi_base (al, bl, bits << 1);
    }

  else
    {
      int res = mpn_jacobi_2 (ap, bp, bits & 1);
      TMP_FREE;
      return res;
    }
}

#endif
