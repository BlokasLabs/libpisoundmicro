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

#ifndef PISOUND_MICRO_API_INTERNAL_H
#define PISOUND_MICRO_API_INTERNAL_H

#ifdef UPISND_INTERNAL

#include "../pisound-micro.h"

#ifdef __cplusplus
extern "C" {
#endif

UPISND_API struct upisnd_ctx_t *upisnd_init_internal(const char *sysfs_base_path);
UPISND_API struct upisnd_ctx_t *upisnd_set_active_ctx(struct upisnd_ctx_t *ctx);

UPISND_API int upisnd_set_adc_offset(int16_t offset);
UPISND_API int16_t upisnd_get_adc_offset(void);

UPISND_API int upisnd_set_adc_gain(uint16_t gain);
UPISND_API uint16_t upisnd_get_adc_gain(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // UPISND_INTERNAL

#endif // PISOUND_MICRO_API_INTERNAL_H
