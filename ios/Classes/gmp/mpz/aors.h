/* mpz_add, mpz_sub -- add or subtract integers.

Copyright 1991, 1993, 1994, 1996, 2000, 2001, 2011, 2012 Free Software
Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

#undef FUNCTION

#ifdef OPERATION_add
#define FUNCTION     mpz_add
#define VARIATION
#endif
#ifdef OPERATION_sub
#define FUNCTION     mpz_sub
#define VARIATION    -
#endif

#ifndef FUNCTION
Error, need OPERATION_add or OPERATION_sub
#endif


void
FUNCTION (mpz_ptr w, mpz_srcptr u, mpz_srcptr v)
{
  mp_srcptr up, vp;
  mp_ptr wp;
  mp_size_t usize, vsize, wsize;
  mp_size_t abs_usize;
  mp_size_t abs_vsize;

  usize = SIZ(u);
  vsize = VARIATION SIZ(v);
  abs_usize = ABS (usize);
  abs_vsize = ABS (vsize);

  if (abs_usize < abs_vsize)
    {
      /* Swap U and V. */
      MPZ_SRCPTR_SWAP (u, v);
      MP_SIZE_T_SWAP (usize, vsize);
      MP_SIZE_T_SWAP (abs_usize, abs_vsize);
    }

  /* True: ABS_USIZE >= ABS_VSIZE.  */

  /* If not space for w (and possible carry), increase space.  */
  wsize = abs_usize + 1;
  wp = MPZ_REALLOC (w, wsize);

  /* These must be after realloc (u or v may be the same as w).  */
  up = PTR(u);
  vp = PTR(v);

  if ((usize ^ vsize) < 0)
    {
      /* U and V have different sign.  Need to compare them to determine
	 which operand to subtract from which.  */

      /* This test is right since ABS_USIZE >= ABS_VSIZE.  */
      if (abs_usize != abs_vsize)
	{
	  mpn_sub (wp, up, abs_usize, vp, abs_vsize);
	  wsize = abs_usize;
	  MPN_NORMALIZE (wp, wsize);
	  if (usize < 0)
	    wsize = -wsize;
	}
      else if (mpn_cmp (up, vp, abs_usize) < 0)
	{
	  mpn_sub_n (wp, vp, up, abs_usize);
	  wsize = abs_usize;
	  MPN_NORMALIZE (wp, wsize);
	  if (usize >= 0)
	    wsize = -wsize;
	}
      else
	{
	  mpn_sub_n (wp, up, vp, abs_usize);
	  wsize = abs_usize;
	  MPN_NORMALIZE (wp, wsize);
	  if (usize < 0)
	    wsize = -wsize;
	}
    }
  else
    {
      /* U and V have same sign.  Add them.  */
      mp_limb_t cy_limb = mpn_add (wp, up, abs_usize, vp, abs_vsize);
      wp[abs_usize] = cy_limb;
      wsize = abs_usize + cy_limb;
      if (usize < 0)
	wsize = -wsize;
    }

  SIZ(w) = wsize;
}
