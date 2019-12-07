/* mpn_and_n, mpn_ior_n, etc -- mpn logical operations.

Copyright 2009 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

#define func __MPN(and_n)
#define call mpn_and_n
void
func (mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n)
{
  call (rp, up, vp, n);
}


#define func __MPN(andn_n)
#define call mpn_andn_n
void
func (mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n)
{
  call (rp, up, vp, n);
}


#define func __MPN(nand_n)
#define call mpn_nand_n
void
func (mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n)
{
  call (rp, up, vp, n);
}


#define func __MPN(ior_n)
#define call mpn_ior_n
void
func (mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n)
{
  call (rp, up, vp, n);
}


#define func __MPN(iorn_n)
#define call mpn_iorn_n
void
func (mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n)
{
  call (rp, up, vp, n);
}


#define func __MPN(nior_n)
#define call mpn_nior_n
void
func (mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n)
{
  call (rp, up, vp, n);
}


#define func __MPN(xor_n)
#define call mpn_xor_n
void
func (mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n)
{
  call (rp, up, vp, n);
}


#define func __MPN(xnor_n)
#define call mpn_xnor_n
void
func (mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n)
{
  call (rp, up, vp, n);
}

