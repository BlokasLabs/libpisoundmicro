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

#ifndef PISOUND_MICRO_H
#define PISOUND_MICRO_H

/**
 * @file pisound-micro.h
 * @brief The header to include in your projects: `#include <pisound-micro.h>`
 * @see pisound-micro/types.h
 * @see pisound-micro/api.h
 * @see pisound-micro/api_cpp.h
 */

#ifdef UPISND_BUILDING_LIB
#	define UPISND_API __attribute__((visibility("default")))
#else
#	define UPISND_API
#endif

#include "pisound-micro/types.h"
#include "pisound-micro/api.h"
#include "pisound-micro/api_cpp.h"

/**
 * \mainpage
 *
 * This is the reference documentation for the Pisound Micro I/O library.
 *
 * * \ref c
 * * \ref cpp
 * * \ref python
 *
 * See also https://blokas.io/pisound-micro/docs/.
 */

#endif // PISOUND_MICRO_H
