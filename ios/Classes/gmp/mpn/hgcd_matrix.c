/* hgcd_matrix.c.

   THE FUNCTIONS IN THIS FILE ARE INTERNAL WITH MUTABLE INTERFACES.  IT IS ONLY
   SAFE TO REACH THEM THROUGH DOCUMENTED INTERFACES.  IN FACT, IT IS ALMOST
   GUARANTEED THAT THEY'LL CHANGE OR DISAPPEAR IN A FUTURE GNU MP RELEASE.

Copyright 2003-2005, 2008, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"
#include "../gmp-impl.c"

/* For input of size n, matrix elements are of size at most ceil(n/2)
   - 1, but we need two limbs extra. */
void
mpn_hgcd_matrix_init (struct hgcd_matrix *mat, mp_size_t n, mp_ptr p)
{
  mp_size_t s = (n+1)/2 + 1;
  mat->alloc = s;
  mat->n = 1;
  MPN_ZERO (p, 4 * s);
  mat->p[0][0] = p;
  mat->p[0][1] = p + s;
  mat->p[1][0] = p + 2 * s;
  mat->p[1][1] = p + 3 * s;

  mat->p[0][0][0] = mat->p[1][1][0] = 1;
}

/* Update column COL, adding in Q * column (1-COL). Temporary storage:
 * qn + n <= mat->alloc, where n is the size of the largest element in
 * column 1 - COL. */
void
mpn_hgcd_matrix_update_q (struct hgcd_matrix *mat, mp_srcptr qp, mp_size_t qn,
			  unsigned col, mp_ptr tp)
{
  ASSERT (col < 2);

  if (qn == 1)
    {
      mp_limb_t q = qp[0];
      mp_limb_t c0, c1;

      c0 = mpn_addmul_1 (mat->p[0][col], mat->p[0][1-col], mat->n, q);
      c1 = mpn_addmul_1 (mat->p[1][col], mat->p[1][1-col], mat->n, q);

      mat->p[0][col][mat->n] = c0;
      mat->p[1][col][mat->n] = c1;

      mat->n += (c0 | c1) != 0;
    }
  else
    {
      unsigned row;

      /* Carries for the unlikely case that we get both high words
	 from the multiplication and carries from the addition. */
      mp_limb_t c[2];
      mp_size_t n;

      /* The matrix will not necessarily grow in size by qn, so we
	 need normalization in order not to overflow mat. */

      for (n = mat->n; n + qn > mat->n; n--)
	{
	  ASSERT (n > 0);
	  if (mat->p[0][1-col][n-1] > 0 || mat->p[1][1-col][n-1] > 0)
	    break;
	}

      ASSERT (qn + n <= mat->alloc);

      for (row = 0; row < 2; row++)
	{
	  if (qn <= n)
	    mpn_mul (tp, mat->p[row][1-col], n, qp, qn);
	  else
	    mpn_mul (tp, qp, qn, mat->p[row][1-col], n);

	  ASSERT (n + qn >= mat->n);
	  c[row] = mpn_add (mat->p[row][col], tp, n + qn, mat->p[row][col], mat->n);
	}

      n += qn;

      if (c[0] | c[1])
	{
	  mat->p[0][col][n] = c[0];
	  mat->p[1][col][n] = c[1];
	  n++;
	}
      else
	{
	  n -= (mat->p[0][col][n-1] | mat->p[1][col][n-1]) == 0;
	  ASSERT (n >= mat->n);
	}
      mat->n = n;
    }

  ASSERT (mat->n < mat->alloc);
}

/* Multiply mat by M1 from the right. Since the M1 elements fit in
   GMP_NUMB_BITS - 1 bits, mat grows by at most one limb. Needs
   temporary space mat->n */
void
mpn_hgcd_matrix_mul_1 (struct hgcd_matrix *mat, const struct hgcd_matrix1 *M1,
		       mp_ptr tp)
{
  mp_size_t n0, n1;

  /* Could avoid copy by some swapping of pointers. */
  MPN_COPY (tp, mat->p[0][0], mat->n);
  n0 = mpn_hgcd_mul_matrix1_vector (M1, mat->p[0][0], tp, mat->p[0][1], mat->n);
  MPN_COPY (tp, mat->p[1][0], mat->n);
  n1 = mpn_hgcd_mul_matrix1_vector (M1, mat->p[1][0], tp, mat->p[1][1], mat->n);

  /* Depends on zero initialization */
  mat->n = MAX(n0, n1);
  ASSERT (mat->n < mat->alloc);
}

/* Multiply mat by M1 from the right. Needs 3*(mat->n + M1->n) + 5 limbs
   of temporary storage (see mpn_matrix22_mul_itch). */
void
mpn_hgcd_matrix_mul (struct hgcd_matrix *mat, const struct hgcd_matrix *M1,
		     mp_ptr tp)
{
  mp_size_t n;

  /* About the new size of mat:s elements. Since M1's diagonal elements
     are > 0, no element can decrease. The new elements are of size
     mat->n + M1->n, one limb more or less. The computation of the
     matrix product produces elements of size mat->n + M1->n + 1. But
     the true size, after normalization, may be three limbs smaller.

     The reason that the product has normalized size >= mat->n + M1->n -
     2 is subtle. It depends on the fact that mat and M1 can be factored
     as products of (1,1; 0,1) and (1,0; 1,1), and that we can't have
     mat ending with a large power and M1 starting with a large power of
     the same matrix. */

  /* FIXME: Strassen multiplication gives only a small speedup. In FFT
     multiplication range, this function could be sped up quite a lot
     using invariance. */
  ASSERT (mat->n + M1->n < mat->alloc);

  ASSERT ((mat->p[0][0][mat->n-1] | mat->p[0][1][mat->n-1]
	   | mat->p[1][0][mat->n-1] | mat->p[1][1][mat->n-1]) > 0);

  ASSERT ((M1->p[0][0][M1->n-1] | M1->p[0][1][M1->n-1]
	   | M1->p[1][0][M1->n-1] | M1->p[1][1][M1->n-1]) > 0);

  mpn_matrix22_mul (mat->p[0][0], mat->p[0][1],
		    mat->p[1][0], mat->p[1][1], mat->n,
		    M1->p[0][0], M1->p[0][1],
		    M1->p[1][0], M1->p[1][1], M1->n, tp);

  /* Index of last potentially non-zero limb, size is one greater. */
  n = mat->n + M1->n;

  n -= ((mat->p[0][0][n] | mat->p[0][1][n] | mat->p[1][0][n] | mat->p[1][1][n]) == 0);
  n -= ((mat->p[0][0][n] | mat->p[0][1][n] | mat->p[1][0][n] | mat->p[1][1][n]) == 0);
  n -= ((mat->p[0][0][n] | mat->p[0][1][n] | mat->p[1][0][n] | mat->p[1][1][n]) == 0);

  ASSERT ((mat->p[0][0][n] | mat->p[0][1][n] | mat->p[1][0][n] | mat->p[1][1][n]) > 0);

  mat->n = n + 1;
}

/* Multiplies the least significant p limbs of (a;b) by mat^-1.
   Temporary space needed: 2 * (p + mat->n)*/
mp_size_t
mpn_hgcd_matrix_adjust (const struct hgcd_matrix *mat,
			mp_size_t n, mp_ptr ap, mp_ptr bp,
			mp_size_t p, mp_ptr tp)
{
  /* mat^-1 (a;b) = (r11, -r01; -r10, r00) (a ; b)
     = (r11 a - r01 b; - r10 a + r00 b */

  mp_ptr t0 = tp;
  mp_ptr t1 = tp + p + mat->n;
  mp_limb_t ah, bh;
  mp_limb_t cy;

  ASSERT (p + mat->n  < n);

  /* First compute the two values depending on a, before overwriting a */

  if (mat->n >= p)
    {
      mpn_mul (t0, mat->p[1][1], mat->n, ap, p);
      mpn_mul (t1, mat->p[1][0], mat->n, ap, p);
    }
  else
    {
      mpn_mul (t0, ap, p, mat->p[1][1], mat->n);
      mpn_mul (t1, ap, p, mat->p[1][0], mat->n);
    }

  /* Update a */
  MPN_COPY (ap, t0, p);
  ah = mpn_add (ap + p, ap + p, n - p, t0 + p, mat->n);

  if (mat->n >= p)
    mpn_mul (t0, mat->p[0][1], mat->n, bp, p);
  else
    mpn_mul (t0, bp, p, mat->p[0][1], mat->n);

  cy = mpn_sub (ap, ap, n, t0, p + mat->n);
  ASSERT (cy <= ah);
  ah -= cy;

  /* Update b */
  if (mat->n >= p)
    mpn_mul (t0, mat->p[0][0], mat->n, bp, p);
  else
    mpn_mul (t0, bp, p, mat->p[0][0], mat->n);

  MPN_COPY (bp, t0, p);
  bh = mpn_add (bp + p, bp + p, n - p, t0 + p, mat->n);
  cy = mpn_sub (bp, bp, n, t1, p + mat->n);
  ASSERT (cy <= bh);
  bh -= cy;

  if (ah > 0 || bh > 0)
    {
      ap[n] = ah;
      bp[n] = bh;
      n++;
    }
  else
    {
      /* The subtraction can reduce the size by at most one limb. */
      if (ap[n-1] == 0 && bp[n-1] == 0)
	n--;
    }
  ASSERT (ap[n-1] > 0 || bp[n-1] > 0);
  return n;
}
