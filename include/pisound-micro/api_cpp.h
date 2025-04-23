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

#ifndef PISOUND_MICRO_API_CPP_H
#define PISOUND_MICRO_API_CPP_H

/** @defgroup cpp C++ API
 *
 * The C++ API reference documentation for libpisoundmicro.
 *
 *
 * # Quick Start
 *
 * First, install the development package:
 *
 * ```bash
 * sudo apt install libpisoundmicro-dev
 * ```
 *
 * Then, let's create a simple program to read the GPIO value of pin B03:
 *
 * ```cpp
#include <pisound-micro.h>
#include <iostream>

int main(int argc, char **argv)
{
	// Initialize the libpisoundmicro library. The libInit destructor will automatically
	// deinitialize it.
	upisnd::LibInitializer libInit;
	if (libInit.getResult() != 0)
	{
		std::cerr << "Failed to initialize libpisoundmicro: "
				  << libInit.getResult() << std::endl;
		return 1;
	}

	// Setup a gpio input Element with B03 pin and pull-up enabled.
	upisnd::Gpio gpio = upisnd::Gpio::setupInput(
		upisnd::ElementName::randomized(),
		UPISND_PIN_B03,
		UPISND_PIN_PULL_UP
		);
	if (!gpio.isValid())
	{
		std::cerr << "Failed to setup GPIO." << std::endl;
		return 1;
	}

	// Read the value.
	if (gpio.get())
		std::cout << "B03 is high." << std::endl;
	else
		std::cout << "B03 is low." << std::endl;

	// The pisound-micro resources will get automatically relased upon
	// destruction of the gpio and libInit objects.
	return 0;
}
```
 * Save the code as `example.cpp` and compile it:
 * ```
 * g++ example.cpp -lpisoundmicro -o example
 * ```
 *
 * And run it:
 *
 * ```
 * ./example
 * ```
 *
 * You should see it output either \"\a B03 \a is \a high.\" or \"\a B03 \a is \a low.\", depending on the B03 pin state.
 *
 * See the below documentation for more details.
 */

#ifndef PISOUND_MICRO_H
#	error Please #include <pisound-micro.h>!
#endif

/**
 * @file api_cpp.h
 * @brief Includes the C++ headers.
 */

#ifdef __cplusplus

namespace upisnd
{
	class Name;
	class RandomizedName;
	class ValueFd;
	class Element;
	class Encoder;
	class AnalogInput;
	class Gpio;
	class Activity;
}

/** \namespace upisnd
 * @brief The C++ API namespace. See \file api.h for C API.
 */

#include "lib_initializer.h"
#include "value_fd.h"
#include "name.h"
#include "element.h"
#include "encoder.h"
#include "analog_input.h"
#include "gpio.h"
#include "activity.h"

#endif // __cplusplus

#endif // PISOUND_MICRO_API_CPP_H
