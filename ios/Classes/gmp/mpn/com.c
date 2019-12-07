/* mpn_com - complement an mpn.

Copyright 2009 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

#ifndef mpncomc
#define mpncomc

#undef mpn_com
// #define mpn_com __MPN(com)

void
mpn_com (mp_ptr rp, mp_srcptr up, mp_size_t n)
{
  mp_limb_t ul;
  do {
      ul = *up++;
      *rp++ = ~ul & GMP_NUMB_MASK;
  } while (--n != 0);
}

#endif