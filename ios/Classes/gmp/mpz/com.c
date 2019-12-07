/* mpz_com(mpz_ptr dst, mpz_ptr src) -- Assign the bit-complemented value of
   SRC to DST.

Copyright 1991, 1993, 1994, 1996, 2001, 2003, 2012 Free Software Foundation,
Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpz_com (mpz_ptr dst, mpz_srcptr src)
{
  mp_size_t size = SIZ (src);
  mp_srcptr src_ptr;
  mp_ptr dst_ptr;

  if (size >= 0)
    {
      /* As with infinite precision: one's complement, two's complement.
	 But this can be simplified using the identity -x = ~x + 1.
	 So we're going to compute (~~x) + 1 = x + 1!  */

      if (UNLIKELY (size == 0))
	{
	  /* special case, as mpn_add_1 wants size!=0 */
	  PTR (dst)[0] = 1;
	  SIZ (dst) = -1;
	}
      else
	{
	  mp_limb_t cy;

	  dst_ptr = MPZ_REALLOC (dst, size + 1);

	  src_ptr = PTR (src);

	  cy = mpn_add_1 (dst_ptr, src_ptr, size, (mp_limb_t) 1);
	  dst_ptr[size] = cy;
	  size += cy;

	  /* Store a negative size, to indicate ones-extension.  */
	  SIZ (dst) = -size;
      }
    }
  else
    {
      /* As with infinite precision: two's complement, then one's complement.
	 But that can be simplified using the identity -x = ~(x - 1).
	 So we're going to compute ~~(x - 1) = x - 1!  */
      size = -size;

      dst_ptr = MPZ_REALLOC (dst, size);

      src_ptr = PTR (src);

      mpn_sub_1 (dst_ptr, src_ptr, size, (mp_limb_t) 1);
      size -= dst_ptr[size - 1] == 0;

      /* Store a positive size, to indicate zero-extension.  */
      SIZ (dst) = size;
    }
}
