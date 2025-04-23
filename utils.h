// SPDX-License-Identifier: LGPL-2.1-only
//
// libpisoundmicro - a utility library for Pisound Micro I/O expander capabilities.
// Copyright (c) 2017-2025 Vilniaus Blokas UAB, https://blokas.io/
//
// This file is part of libpisoundmicro.
//
// libpisoundmicro is free software: you can redistribute it and/or modify it under the terms of the
// GNU Lesser General Public License as published by the Free Software Foundation, version 2.1 of the License.
//
// libpisoundmicro is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with libpisoundmicro. If not, see <https://www.gnu.org/licenses/>.

#ifndef PISOUND_MICRO_UTILS_H
#define PISOUND_MICRO_UTILS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int upisnd_base64_encode(char *dst, size_t dst_length, const void *d, size_t length, bool pad);

typedef uint32_t upisnd_xoshiro128_star_star_seed_t[4];
uint32_t upisnd_xoshiro128_star_star_next(upisnd_xoshiro128_star_star_seed_t seed);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // PISOUND_MICRO_UTILS_H
