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

#ifndef PISOUND_MICRO_GPIO_H
#define PISOUND_MICRO_GPIO_H

#ifndef PISOUND_MICRO_H
#	error Please #include <pisound-micro.h>!
#endif

/**
 * @file gpio.h
 * @brief Gpio element class header.
 */

namespace upisnd
{

/// The GPIO element class.
/// @ingroup cpp
class UPISND_API Gpio : public Element
{
public:
	using Element::Element;

	/// @brief Set up a GPIO element as input.
	/// @see upisnd_setup_gpio_input
	static Gpio setupInput(ElementName name, upisnd_pin_t pin, upisnd_pin_pull_e pull);
	/// @brief Set up a GPIO element as output.
	/// @see upisnd_setup_gpio_output
	static Gpio setupOutput(ElementName name, upisnd_pin_t pin, bool high);

	/// @brief Get the direction of the GPIO element.
	upisnd_pin_direction_e getDirection() const;
	/// @brief Get the pull of the GPIO Input element.
	/// @return Returns #UPISND_PIN_PULL_INVALID if the element is not an input.
	upisnd_pin_pull_e getPull() const;

	/// @brief This is for quick access to the value, otherwise, it's recommended to keep the
	/// ValueFd returned by Element::openValueFd, to avoid file open and close overhead.
	/// @return Negative return value indicates an error.
	int get() const;

	/// @brief Set the output value of the GPIO output element.
	/// @return Negative return value indicates an error.
	int set(bool high);
};

} // namespace upisnd

#endif // PISOUND_MICRO_GPIO_H
