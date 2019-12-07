/* mpf_set -- Assign a float from another float.

Copyright 1993-1995, 2001, 2004 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

void
mpf_set (mpf_ptr r, mpf_srcptr u)
{
  mp_ptr rp, up;
  mp_size_t size, asize;
  mp_size_t prec;

  prec = r->_mp_prec + 1;		/* lie not to lose precision in assignment */
  size = u->_mp_size;
  asize = ABS (size);
  rp = r->_mp_d;
  up = u->_mp_d;

  if (asize > prec)
    {
      up += asize - prec;
      asize = prec;
    }

  r->_mp_exp = u->_mp_exp;
  r->_mp_size = size >= 0 ? asize : -asize;
  MPN_COPY_INCR (rp, up, asize);
}
