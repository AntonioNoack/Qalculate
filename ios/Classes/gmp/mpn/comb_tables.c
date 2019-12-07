/* Const tables shared among combinatoric functions.

   THE CONTENTS OF THIS FILE ARE FOR INTERNAL USE AND ARE ALMOST CERTAIN TO
   BE SUBJECT TO INCOMPATIBLE CHANGES IN FUTURE GNU MP RELEASES.

Copyright 2012 Free Software Foundation, Inc.
  */

  #ifndef COMB_TABS_C
  #define COMB_TABS_C

#include "../gmp.h"
#include "../gmp-impl.h"

/* Entry i contains (i!/2^t) where t is chosen such that the parenthesis
   is an odd integer. */
const mp_limb_t __gmp_oddfac_table[] = { ONE_LIMB_ODD_FACTORIAL_TABLE, ONE_LIMB_ODD_FACTORIAL_EXTTABLE };

/* Entry i contains ((2i+1)!!/2^t) where t is chosen such that the parenthesis
   is an odd integer. */
const mp_limb_t __gmp_odd2fac_table[] = { ONE_LIMB_ODD_DOUBLEFACTORIAL_TABLE };

/* Entry i contains 2i-popc(2i). */
const unsigned char __gmp_fac2cnt_table[] = { TABLE_2N_MINUS_POPC_2N };

const mp_limb_t __gmp_limbroots_table[] = { NTH_ROOT_NUMB_MASK_TABLE };

#endif
