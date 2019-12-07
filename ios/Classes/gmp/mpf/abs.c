/* mpf_abs -- Compute the absolute value of a float.

Copyright 1993-1995, 2001 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

void
mpf_abs (mpf_ptr r, mpf_srcptr u)
{
  mp_size_t size;

  size = ABS (u->_mp_size);
  if (r != u)
    {
      mp_size_t prec;
      mp_ptr rp, up;

      prec = r->_mp_prec + 1;	/* lie not to lose precision in assignment */
      rp = r->_mp_d;
      up = u->_mp_d;

      if (size > prec)
	{
	  up += size - prec;
	  size = prec;
	}

      MPN_COPY (rp, up, size);
      r->_mp_exp = u->_mp_exp;
    }
  r->_mp_size = size;
}
