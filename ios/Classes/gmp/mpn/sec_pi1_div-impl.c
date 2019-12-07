/* mpn_sec_pi1_div_qr, mpn_sec_pi1_div_r -- Compute Q = floor(U / V), U = U
   mod V.  Side-channel silent under the assumption that the used instructions
   are side-channel silent.

   Contributed to the GNU project by Torbjörn Granlund.

   THE FUNCTIONS IN THIS FILE ARE INTERNAL WITH MUTABLE INTERFACES.  IT IS ONLY
   SAFE TO REACH THEM THROUGH DOCUMENTED INTERFACES.  IN FACT, IT IS ALMOST
   GUARANTEED THAT THEY WILL CHANGE OR DISAPPEAR IN A FUTURE GNU MP RELEASE.

Copyright 2011-2013 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"

/* This side-channel silent division algorithm reduces the partial remainder by
   GMP_NUMB_BITS/2 bits at a time, compared to GMP_NUMB_BITS for the main
   division algorithm.  We actually do not insist on reducing by exactly
   GMP_NUMB_BITS/2, but may leave a partial remainder that is D*B^i to 3D*B^i
   too large (B is the limb base, D is the divisor, and i is the induction
   variable); the subsequent step will handle the extra partial remainder bits.

   With that partial remainder reduction, each step generates a quotient "half
   limb".  The outer loop generates two quotient half limbs, an upper (q1h) and
   a lower (q0h) which are stored sparsely in separate limb arrays.  These
   arrays are added at the end; using separate arrays avoids data-dependent
   carry propagation which could else pose a side-channel leakage problem.

   The quotient half limbs may be between -3 to 0 from the accurate value
   ("accurate" being the one which corresponds to a reduction to a principal
   partial remainder).  Too small quotient half limbs correspond to too large
   remainders, which we reduce later, as described above.

   In order to keep quotients from getting too big, corresponding to a negative
   partial remainder, we use an inverse which is slightly smaller than usually.
*/

#define OPERATION_sec_pi1_div_qr 1
#include "sec_pi1_div.c"
#undef OPERATION_sec_pi1_div_qr
#undef RETTYPE
#undef Q
#undef FNAME
#define OPERATION_sec_pi1_div_r 1
#include "sec_pi1_div.c"
#undef OPERATION_sec_pi1_div_r
#undef RETTYPE
#undef Q
#undef FNAME
