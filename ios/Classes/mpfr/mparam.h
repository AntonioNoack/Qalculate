/* Various Thresholds of MPFR, not exported.  -*- mode: C -*-

Copyright 2005-2019 Free Software Foundation, Inc.

This file is part of the GNU MPFR Library.

The GNU MPFR Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

The GNU MPFR Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the GNU MPFR Library; see the file COPYING.LESSER.  If not, see
https://www.gnu.org/licenses/ or write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef __MPFR_IMPL_H__
# error "MPFR Internal not included"
#endif

/* Threshold when testing coverage */
#if defined(MPFR_TUNE_COVERAGE)
#define MPFR_TUNE_CASE "src/generic/coverage/mparam.h"
#include "generic/coverage/mparam.h"

/* Note: the different macros used here are those defined by gcc,
   for example with gcc -dM -E -xc /dev/null
   As of gcc 4.2, you can also use: -march=native or -mtune=native */

#else
#define MPFR_TUNE_CASE "default"
#endif

/****************************************************************
 * Default values of Threshold.                                 *
 * Must be included in any case: it checks, for every constant, *
 * if it has been defined, and it sets it to a default value if *
 * it was not previously defined.                               *
 ****************************************************************/
#include "generic/mparam.h"
