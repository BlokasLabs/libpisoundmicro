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

#ifndef PISOUND_MICRO_ACTIVITY_H
#define PISOUND_MICRO_ACTIVITY_H

#ifndef PISOUND_MICRO_H
#	error Please #include <pisound-micro.h>!
#endif

/**
 * @file activity.h
 * @brief Activity element class header.
 */

namespace upisnd
{

/// @brief Activity element class.
/// @ingroup cpp
class UPISND_API Activity : public Element
{
public:
	using Element::Element;

	/// @brief Set up an activity element.
	/// @see upisnd_setup_activity
	static Activity setup(ElementName name, upisnd_pin_t pin, upisnd_activity_e activity);

	/// @brief Get the activity of the element.
	upisnd_activity_e getActivity() const;
};

} // namespace upisnd

#endif // PISOUND_MICRO_ACTIVITY_H
