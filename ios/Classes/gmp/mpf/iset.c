/* mpf_init_set -- Initialize a float and assign it from another float.

Copyright 1993-1995, 2000, 2001, 2004 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "set_dfl_prec.c"

void
mpf_init_set (mpf_ptr r, mpf_srcptr s)
{
  mp_ptr rp, sp;
  mp_size_t ssize, size;
  mp_size_t prec;

  prec = __gmp_default_fp_limb_precision;
  r->_mp_d = __GMP_ALLOCATE_FUNC_LIMBS (prec + 1);
  r->_mp_prec = prec;

  prec++;		/* lie not to lose precision in assignment */
  ssize = s->_mp_size;
  size = ABS (ssize);

  rp = r->_mp_d;
  sp = s->_mp_d;

  if (size > prec)
    {
      sp += size - prec;
      size = prec;
    }

  r->_mp_exp = s->_mp_exp;
  r->_mp_size = ssize >= 0 ? size : -size;

  MPN_COPY (rp, sp, size);
}
