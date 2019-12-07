/* mpf_set_ui() -- Assign a float from an unsigned int.

Copyright 1993-1995, 2001, 2002, 2004 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

void
mpf_set_ui (mpf_ptr f, unsigned long val)
{
  mp_size_t size;

  f->_mp_d[0] = val & GMP_NUMB_MASK;
  size = val != 0;

#if BITS_PER_ULONG > GMP_NUMB_BITS
  val >>= GMP_NUMB_BITS;
  f->_mp_d[1] = val;
  size += (val != 0);
#endif

  f->_mp_exp = f->_mp_size = size;
}
