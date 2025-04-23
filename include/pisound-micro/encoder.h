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

#ifndef PISOUND_MICRO_ENCODER_H
#define PISOUND_MICRO_ENCODER_H

#ifndef PISOUND_MICRO_H
#	error Please #include <pisound-micro.h>!
#endif

/**
 * @file encoder.h
 * @brief Encoder element class header.
 */

namespace upisnd
{

/// The Encoder element class.
/// @ingroup cpp
class UPISND_API Encoder : public Element
{
public:
	using Element::Element;

	/// @brief Set up an encoder element.
	/// @see ::upisnd_setup_encoder
	static Encoder setup(ElementName name, upisnd_pin_t pin_a, upisnd_pin_pull_e pull_a, upisnd_pin_t pin_b, upisnd_pin_pull_e pull_b);

	/// @brief This is for quick access to the value, otherwise, it's recommended to keep the
	///
	/// ValueFd returned by Element::openValueFd, to avoid file open and close overhead.
	/// Inspect the `errno` to check for errors.
	int get() const;

	/// Retrieves the Encoder options. @see ::upisnd_element_encoder_get_opts
	int getOpts(upisnd_encoder_opts_t &opts) const;

	/// Sets the Encoder options. @see ::upisnd_element_encoder_set_opts
	int setOpts(const upisnd_encoder_opts_t &opts);

	/// Retrieves the 2nd pin of the Encoder. @see ::upisnd_element_encoder_get_pin_b
	upisnd_pin_t getPinB() const;

	/// Retrieves the pull-up/pull-down configuration of the 1st pin of the Encoder. @see ::upisnd_element_gpio_get_pull
	upisnd_pin_pull_e getPinPull() const;

	/// Retrieves the pull-up/pull-down configuration of the 2nd pin of the Encoder. @see ::upisnd_element_encoder_get_pin_b_pull
	upisnd_pin_pull_e getPinBPull() const;
};

} // namespace upisnd

#endif // PISOUND_MICRO_ENCODER_H
