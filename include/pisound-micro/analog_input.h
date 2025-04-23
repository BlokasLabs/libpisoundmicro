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

#ifndef PISOUND_MICRO_ANALOG_INPUT_H
#define PISOUND_MICRO_ANALOG_INPUT_H

#ifndef PISOUND_MICRO_H
#	error Please #include <pisound-micro.h>!
#endif

/**
 * @file analog_input.h
 * @brief Analog Input element class header.
 */

namespace upisnd
{

/// @brief Analog input element class.
/// @ingroup cpp
class UPISND_API AnalogInput : public Element
{
public:
	using Element::Element;

	/// @brief Set up an analog input element.
	static AnalogInput setup(ElementName name, upisnd_pin_t pin);

	/// @brief This is for quick access to the value, otherwise, it's recommended to keep the
	///
	/// ValueFd returned by Element::openValueFd, to avoid file open and close overhead.
	/// Inspect the `errno` to check for errors.
	int get() const;

	/// @brief Retrieves the Analog Input options. @see ::upisnd_element_analog_input_get_opts
	int getOpts(upisnd_analog_input_opts_t &opts) const;
	/// @brief Sets the Analog Input options. @see ::upisnd_element_analog_input_set_opts
	int setOpts(const upisnd_analog_input_opts_t &opts);
};

} // namespace upisnd

#endif // PISOUND_MICRO_ANALOG_INPUT_H
