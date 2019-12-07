
#ifndef GMP_IMPL_C
#define GMP_IMPL_C

#include "../config.h"
#include "config.h"
#include "gmp-impl.h"

static inline mp_size_t
mpn_mulmod_bnm1_itch (mp_size_t rn, mp_size_t an, mp_size_t bn) {
  mp_size_t n, itch;
  n = rn >> 1;
  itch = rn + 4 +
    (an > n ? (bn > n ? rn : n) : 0);
  return itch;
}

static inline mp_size_t
mpn_sqrmod_bnm1_itch (mp_size_t rn, mp_size_t an) {
  mp_size_t n, itch;
  n = rn >> 1;
  itch = rn + 3 +
    (an > n ? an : 0);
  return itch;
}

static inline void *_mpz_newalloc (mpz_ptr z, mp_size_t n){
  void * res = _mpz_realloc(z,n);
  /* If we are checking the code, force a random change to limbs. */
  ((mp_ptr) res)[0] = ~ ((mp_ptr) res)[ALLOC (z) - 1];
  return res;
}

static inline unsigned log_n_max (mp_limb_t n)
{
  unsigned log;
  for (log = 8; n > __gmp_limbroots_table[log - 1]; log--);
  return log;
}

static inline unsigned mpn_jacobi_init (unsigned a, unsigned b, unsigned s){
  ASSERT (b & 1);
  ASSERT (s <= 1);
  return ((a & 3) << 2) + (b & 2) + s;
}


static inline int mpn_jacobi_finish(unsigned bits){
  /* (a, b) = (1,0) or (0,1) */
  ASSERT ( (bits & 14) == 0);

  return 1-2*(bits & 1);
}

static inline unsigned mpn_jacobi_update (unsigned bits, unsigned denominator, unsigned q){
  /* FIXME: Could halve table size by not including the e bit in the
   * index, and instead xor when updating. Then the lookup would be
   * like
   *
   *   bits ^= table[((bits & 30) << 2) + (denominator << 2) + q];
   */

  ASSERT (bits < 26);
  ASSERT (denominator < 2);
  ASSERT (q < 4);

  /* For almost all calls, denominator is constant and quite often q
     is constant too. So use addition rather than or, so the compiler
     can put the constant part can into the offset of an indexed
     addressing instruction.

     With constant denominator, the below table lookup is compiled to

       C Constant q = 1, constant denominator = 1
       movzbl table+5(%eax,8), %eax

     or

       C q in %edx, constant denominator = 1
       movzbl table+4(%edx,%eax,8), %eax

     One could maintain the state preshifted 3 bits, to save a shift
     here, but at least on x86, that's no real saving.
  */
  return bits = jacobi_table[(bits << 3) + (denominator << 2) + q];
}

static inline mp_limb_t mpn_add_nc (mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n, mp_limb_t ci) {
  mp_limb_t co;
  co = mpn_add_n (rp, up, vp, n);
  co += mpn_add_1 (rp, rp, n, ci);
  return co;
}

static inline mp_limb_t mpn_sub_nc (mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n, mp_limb_t ci) {
  mp_limb_t co;
  co = mpn_sub_n (rp, up, vp, n);
  co += mpn_sub_1 (rp, rp, n, ci);
  return co;
}

static inline mp_size_t mpn_toom6h_mul_itch (mp_size_t an, mp_size_t bn) {
  mp_size_t estimatedN;
  estimatedN = (an + bn) / (size_t) 10 + 1;
  return mpn_toom6_mul_n_itch (estimatedN * 6);
}

static inline mp_size_t mpn_toom8h_mul_itch (mp_size_t an, mp_size_t bn) {
  mp_size_t estimatedN;
  estimatedN = (an + bn) / (size_t) 14 + 1;
  return mpn_toom8_mul_n_itch (estimatedN * 8);
}


static inline mp_size_t mpn_toom32_mul_itch (mp_size_t an, mp_size_t bn)
{
  mp_size_t n = 1 + (2 * an >= 3 * bn ? (an - 1) / (size_t) 3 : (bn - 1) >> 1);
  mp_size_t itch = 2 * n + 1;

  return itch;
}

static inline mp_size_t
mpn_toom42_mul_itch (mp_size_t an, mp_size_t bn)
{
  mp_size_t n = an >= 2 * bn ? (an + 3) >> 2 : (bn + 1) >> 1;
  return 6 * n + 3;
}


static inline mp_size_t
mpn_toom52_mul_itch (mp_size_t an, mp_size_t bn)
{
  mp_size_t n = 1 + (2 * an >= 5 * bn ? (an - 1) / (size_t) 5 : (bn - 1) >> 1);
  return 6*n + 4;
}


static inline mp_size_t
mpn_toom53_mul_itch (mp_size_t an, mp_size_t bn)
{
  mp_size_t n = 1 + (3 * an >= 5 * bn ? (an - 1) / (size_t) 5 : (bn - 1) / (size_t) 3);
  return 10 * n + 10;
}


static inline mp_size_t
mpn_toom43_mul_itch (mp_size_t an, mp_size_t bn)
{
  mp_size_t n = 1 + (3 * an >= 4 * bn ? (an - 1) >> 2 : (bn - 1) / (size_t) 3);

  return 6*n + 4;
}

static inline mp_size_t
mpn_toom62_mul_itch (mp_size_t an, mp_size_t bn)
{
  mp_size_t n = 1 + (an >= 3 * bn ? (an - 1) / (size_t) 6 : (bn - 1) >> 1);
  return 10 * n + 10;
}

static inline mp_size_t
mpn_toom63_mul_itch (mp_size_t an, mp_size_t bn)
{
  mp_size_t n = 1 + (an >= 2 * bn ? (an - 1) / (size_t) 6 : (bn - 1) / (size_t) 3);
  return 9 * n + 3;
}

static inline mp_size_t
mpn_toom54_mul_itch (mp_size_t an, mp_size_t bn)
{
  mp_size_t n = 1 + (4 * an >= 5 * bn ? (an - 1) / (size_t) 5 : (bn - 1) / (size_t) 4);
  return 9 * n + 3;
}

#endif