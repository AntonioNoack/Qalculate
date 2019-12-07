/* mpz_tstbit -- test a specified bit.

Copyright 2000, 2002 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"


/* For negatives the effective twos complement is achieved by negating the
   limb tested, either with a ones or twos complement.  Twos complement
   ("-") is used if there's only zero limbs below the one being tested.
   Ones complement ("~") is used if there's a non-zero below.  Note that "-"
   is correct even if the limb examined is 0 (and the true beginning of twos
   complement is further up).

   Testing the limbs below p is unavoidable on negatives, but will usually
   need to examine only *(p-1).  The search is done from *(p-1) down to
   *u_ptr, since that might give better cache locality, and because a
   non-zero limb is perhaps a touch more likely in the middle of a number
   than at the low end.

   Bits past the end of available data simply follow sign of u.  Notice that
   the limb_index >= abs_size test covers u=0 too.  */

int
mpz_tstbit (mpz_srcptr u, mp_bitcnt_t bit_index) __GMP_NOTHROW
{
  mp_srcptr      u_ptr      = PTR(u);
  mp_size_t      size       = SIZ(u);
  unsigned       abs_size   = ABS(size);
  mp_size_t      limb_index = bit_index / GMP_NUMB_BITS;
  mp_srcptr      p          = u_ptr + limb_index;
  mp_limb_t      limb;

  if (limb_index >= abs_size)
    return (size < 0);

  limb = *p;
  if (size < 0)
    {
      limb = -limb;     /* twos complement */

      while (p != u_ptr)
	{
	  p--;
	  if (*p != 0)
	    {
	      limb--;	/* make it a ones complement instead */
	      break;
	    }
	}
    }

  return (limb >> (bit_index % GMP_NUMB_BITS)) & 1;
}
