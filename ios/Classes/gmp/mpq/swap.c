/* mpq_swap (U, V) -- Swap U and V.

Copyright 1997, 1998, 2000, 2001 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"

void
mpq_swap (mpq_ptr u, mpq_ptr v) __GMP_NOTHROW
{
  mp_ptr up, vp;
  mp_size_t usize, vsize;
  mp_size_t ualloc, valloc;

  ualloc = ALLOC(NUM(u));
  valloc = ALLOC(NUM(v));
  ALLOC(NUM(v)) = ualloc;
  ALLOC(NUM(u)) = valloc;

  usize = SIZ(NUM(u));
  vsize = SIZ(NUM(v));
  SIZ(NUM(v)) = usize;
  SIZ(NUM(u)) = vsize;

  up = PTR(NUM(u));
  vp = PTR(NUM(v));
  PTR(NUM(v)) = up;
  PTR(NUM(u)) = vp;


  ualloc = ALLOC(DEN(u));
  valloc = ALLOC(DEN(v));
  ALLOC(DEN(v)) = ualloc;
  ALLOC(DEN(u)) = valloc;

  usize = SIZ(DEN(u));
  vsize = SIZ(DEN(v));
  SIZ(DEN(v)) = usize;
  SIZ(DEN(u)) = vsize;

  up = PTR(DEN(u));
  vp = PTR(DEN(v));
  PTR(DEN(v)) = up;
  PTR(DEN(u)) = vp;
}
