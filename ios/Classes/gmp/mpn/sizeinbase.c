/* mpn_sizeinbase -- approximation to chars required for an mpn.

   THE FUNCTIONS IN THIS FILE ARE FOR INTERNAL USE ONLY.  THEY'RE ALMOST
   CERTAIN TO BE SUBJECT TO INCOMPATIBLE CHANGES OR DISAPPEAR COMPLETELY IN
   FUTURE GNU MP RELEASES.

Copyright 1991, 1993-1995, 2001, 2002, 2011, 2012 Free Software Foundation,
Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"


/* Same as mpz_sizeinbase, meaning exact for power-of-2 bases, and either
   exact or 1 too big for other bases.  */

size_t
mpn_sizeinbase (mp_srcptr xp, mp_size_t xsize, int base)
{
  size_t  result;
  MPN_SIZEINBASE (result, xp, xsize, base);
  return result;
}
