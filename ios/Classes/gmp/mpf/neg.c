/* mpf_neg -- Negate a float.

Copyright 1993-1995, 2001 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

void
mpf_neg (mpf_ptr r, mpf_srcptr u)
{
  mp_size_t size;

  size = -u->_mp_size;
  if (r != u)
    {
      mp_size_t prec;
      mp_size_t asize;
      mp_ptr rp, up;

      prec = r->_mp_prec + 1;	/* lie not to lose precision in assignment */
      asize = ABS (size);
      rp = r->_mp_d;
      up = u->_mp_d;

      if (asize > prec)
	{
	  up += asize - prec;
	  asize = prec;
	}

      MPN_COPY (rp, up, asize);
      r->_mp_exp = u->_mp_exp;
      size = size >= 0 ? asize : -asize;
    }
  r->_mp_size = size;
}
