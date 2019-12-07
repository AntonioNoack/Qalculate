/* mpz_sizeinbase(x, base) -- return an approximation to the number of
   character the integer X would have printed in base BASE.  The
   approximation is never too small.

Copyright 1991, 1993-1995, 2001, 2002 Free Software Foundation, Inc.
  */

  #ifndef sizeinbasex
  #define sizeinbasex

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"
#include "../gmp-impl.c"

inline size_t
mpz_sizeinbase (mpz_srcptr x, int base) __GMP_NOTHROW
{
  size_t  result;
  MPN_SIZEINBASE (result, PTR(x), ABSIZ(x), base);
  return result;
}

#endif