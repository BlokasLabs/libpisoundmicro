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

namespace upisnd
{

template <> upisnd_element_type_e Element::classType<Encoder>()     { return UPISND_ELEMENT_TYPE_ENCODER;      }
template <> upisnd_element_type_e Element::classType<AnalogInput>() { return UPISND_ELEMENT_TYPE_ANALOG_INPUT; }
template <> upisnd_element_type_e Element::classType<Gpio>()        { return UPISND_ELEMENT_TYPE_GPIO;         }
template <> upisnd_element_type_e Element::classType<Activity>()    { return UPISND_ELEMENT_TYPE_ACTIVITY;     }

Element::Element()
	:m_ref(NULL)
{
}

Element::Element(upisnd_element_ref_t el)
	:m_ref(upisnd_element_add_ref(el))
{
}

Element::Element(upisnd_element_ref_t el, bool)
	:m_ref(el)
{
}

Element::Element(const Element &rhs)
	:m_ref(upisnd_element_add_ref(rhs.m_ref))
{
}

Element& Element::operator=(const Element &rhs)
{
	if (this != &rhs)
	{
		if (rhs.ref())
			upisnd_element_add_ref(rhs.ref());
		upisnd_element_unref(&m_ref);
		m_ref = rhs.ref();
	}
	return *this;
}

Element::Element(Element &&rhs)
{
	m_ref = rhs.ref();
	rhs.m_ref = NULL;
}

Element& Element::operator=(Element &&rhs)
{
	if (this != &rhs)
	{
		upisnd_element_unref(&m_ref);
		m_ref = rhs.ref();
		rhs.m_ref = NULL;
	}
	return *this;
}

Element::~Element()
{
	upisnd_element_unref(&m_ref);
}

Element Element::get(ElementName name)
{
	return Element(upisnd_element_get(name), false);
}

Element Element::setup(ElementName name, upisnd_setup_t setup)
{
	return Element(upisnd_setup(name, setup), false);
}

bool Element::isValid() const
{
	return ref() != NULL;
}

void Element::release()
{
	upisnd_element_unref(&m_ref);
}

const char *Element::getName() const
{
	return upisnd_element_get_name(ref());
}

upisnd_element_type_e Element::getType() const
{
	return upisnd_element_get_type(ref());
}

upisnd_pin_t Element::getPin() const
{
	return upisnd_element_get_pin(ref());
}

ValueFd Element::openValueFd(int flags) const
{
	return ValueFd(upisnd_element_open_value_fd(ref(), flags));
}

} // namespace upisnd
