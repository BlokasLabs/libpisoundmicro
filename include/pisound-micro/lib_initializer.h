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

#ifndef PISOUND_MICRO_LIB_INITIALIZER_H
#define PISOUND_MICRO_LIB_INITIALIZER_H

#ifndef PISOUND_MICRO_H
#	error Please #include <pisound-micro.h>!
#endif

/**
 * @file lib_initializer.h
 * @brief LibInitializer class header.
 */

namespace upisnd
{

/** @brief Simply calls ::upisnd_init and ::upisnd_uninit in its constructor and destructor, for convenience.
 *
 * You may have a global `upisnd::LibInitializer` variable, or place it inside your main function,
 * prior to calling any other libpisoundmicro functions or instantiating other classes.
 *
 * To know if the initialization succeeded, you can check the result of `getResult()` method, it keeps
 * the value returned by ::upisnd_init.
 *
 * @see upisnd_init
 * @see upisnd_uninit
 *
 * @ingroup cpp
 */
class UPISND_API LibInitializer
{
public:
	LibInitializer();
	LibInitializer(const LibInitializer &) = delete;
	LibInitializer &operator=(const LibInitializer &) = delete;
	LibInitializer(LibInitializer &&) = delete;

	int getResult() const;

	~LibInitializer();
private:
	const int m_result;
};

} // namespace upisnd

#endif // PISOUND_MICRO_LIB_INITIALIZER_H
