/* __clz_tab -- support for longlong.h

   THE CONTENTS OF THIS FILE ARE FOR INTERNAL USE AND MAY CHANGE
   INCOMPATIBLY OR DISAPPEAR IN A FUTURE GNU MP RELEASE.

Copyright 1991, 1993, 1994, 1996, 1997, 2000, 2001 Free Software Foundation,
Inc.
  */
#ifndef mpclztabc
#define mpclztabc


#ifndef MP_CLZ_TAB_C
#define MP_CLZ_TAB_C

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

#ifdef COUNT_LEADING_ZEROS_NEED_CLZ_TAB
const
unsigned char __clz_tab[129] =
{
  1,2,3,3,4,4,4,4,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  9
};
#endif

#endif

#endif
