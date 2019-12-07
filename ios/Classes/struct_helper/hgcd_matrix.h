//
// Created by antonio on 11/29/19.
//

#ifndef CALCULATE_HGCD_MATRIX_H
#define CALCULATE_HGCD_MATRIX_H

struct hgcd_matrix1 {
  mp_limb_t u[2][2];
};

struct hgcd_matrix {
  mp_size_t alloc;		/* for sanity checking only */
  mp_size_t n;
  mp_ptr p[2][2];
};

#endif //CALCULATE_HGCD_MATRIX_H
