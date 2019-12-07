
#include "../gmp.h"
#include "../gmp-impl.h"

#ifndef Xpopham
#define Xpopham

#if OPERATION_popcount
#define FNAME mpn_popcount
#define POPHAM(u,v) u
#endif

#if OPERATION_hamdist
#define FNAME mpn_hamdist
#define POPHAM(u,v) u ^ v
#endif

#define OPERATION_popcount 1
#include "popham.c"
#undef OPERATION_popcount

#define OPERATION_hamdist 1
#include "popham.c"
#undef OPERATION_hamdist

#endif