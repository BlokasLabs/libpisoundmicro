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

#ifndef PISOUND_MICRO_NAME_H
#define PISOUND_MICRO_NAME_H

#ifndef PISOUND_MICRO_H
#	error Please #include <pisound-micro.h>!
#endif

/**
 * @file name.h
 * @brief ElementName class header.
 */

namespace upisnd
{

/// @brief A helper class for formatting Element names.
/// @ingroup cpp
class UPISND_API ElementName
{
public:
	/// Implicit conversion to const char *.
	operator const char *() const;

	/// Implicit conversion from const char *, so plain strings can be used as ElementName argument in APIs.
	ElementName(const char *str);

	/// Returns an ElementName initialized from the provided string.
	static ElementName regular(const char *name);

	/// Returns a randomized ElementName. You may optionally specify a prefix to be prepended to the name.
	/// @see ::upisnd_generate_random_element_name
	static ElementName randomized(const char *prefix = NULL);

	/// Returns a formatted ElementName, following the printf() semantics.
	static ElementName formatted(const char *fmt, ...);

	ElementName(const ElementName &rhs);

private:
	ElementName();

	char m_name[UPISND_MAX_ELEMENT_NAME_LENGTH];
};

} // namespace upisnd

#endif // PISOUND_MICRO_NAME_H
