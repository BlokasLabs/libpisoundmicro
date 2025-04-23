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

AnalogInput AnalogInput::setup(ElementName name, upisnd_pin_t pin)
{
	return AnalogInput(upisnd_setup_analog_input(name, pin), false);
}

int AnalogInput::get() const
{
	ValueFd fd = openValueFd(O_RDONLY | O_CLOEXEC);
	int result = fd.read();
	if (errno != 0)
		return -errno;
	return result;
}

int AnalogInput::getOpts(upisnd_analog_input_opts_t &opts) const
{
	return upisnd_element_analog_input_get_opts(ref(), &opts);
}

int AnalogInput::setOpts(const upisnd_analog_input_opts_t &opts)
{
	return upisnd_element_analog_input_set_opts(ref(), &opts);
}

} // namespace upisnd
