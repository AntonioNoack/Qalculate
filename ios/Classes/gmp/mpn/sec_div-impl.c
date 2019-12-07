/* mpn_sec_div_qr, mpn_sec_div_r -- Compute Q = floor(U / V), U = U mod V.
   Side-channel silent under the assumption that the used instructions are
   side-channel silent.

   Contributed to the GNU project by Torbjörn Granlund.

Copyright 2011-2014 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"

#ifndef secdivimpl
#define secdivimpl

#define OPERATION_sec_div_qr 1
#include "sec_div.c"
#undef OPERATION_sec_div_qr
#undef FNAME
#undef RETTYPE
#define OPERATION_sec_div_r 1
#include "sec_div.c"
#undef FNAME
#undef RETTYPE


#endif
