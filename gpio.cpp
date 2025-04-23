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

#include "include/pisound-micro.h"

#include <fcntl.h>
#include <errno.h>

namespace upisnd
{

Gpio Gpio::setupInput(ElementName name, upisnd_pin_t pin, upisnd_pin_pull_e pull)
{
	return Gpio(upisnd_setup_gpio_input(name, pin, pull), false);
}

Gpio Gpio::setupOutput(ElementName name, upisnd_pin_t pin, bool high)
{
	return Gpio(upisnd_setup_gpio_output(name, pin, high), false);
}

upisnd_pin_direction_e Gpio::getDirection() const
{
	return upisnd_element_gpio_get_direction(ref());
}

upisnd_pin_pull_e Gpio::getPull() const
{
	return upisnd_element_gpio_get_pull(ref());
}

int Gpio::get() const
{
	ValueFd fd = openValueFd(O_RDONLY | O_CLOEXEC);
	return fd.read();
}

int Gpio::set(bool high)
{
	ValueFd fd = openValueFd(O_WRONLY | O_CLOEXEC);
	return fd.isValid() ? fd.write(high ? 1 : 0) : -errno;
}

} // namespace upisnd
