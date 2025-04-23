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

#include <cstring>
#include <cstdio>

namespace upisnd
{

ElementName::ElementName()
{
	m_name[0] = '\0';
}

ElementName::operator const char*() const
{
	return m_name;
}

ElementName ElementName::regular(const char *str)
{
	ElementName ret;
	strncpy(ret.m_name, str, sizeof(ret.m_name));
	ret.m_name[sizeof(ret.m_name)-1] = '\0';
	return ret;
}

ElementName ElementName::randomized(const char *prefix)
{
	ElementName ret;
	int n = upisnd_generate_random_element_name(ret.m_name, sizeof(ret.m_name), prefix);
	return n > 0 && n < sizeof(ret.m_name) ? ret : ElementName();
}

ElementName ElementName::formatted(const char *fmt, ...)
{
	ElementName ret;
	va_list ap;
	va_start(ap, fmt);
	int n = vsnprintf(ret.m_name, sizeof(ret.m_name), fmt, ap);
	va_end(ap);
	return n > 0 && n < sizeof(ret.m_name) ? ret : ElementName();
}

ElementName::ElementName(const ElementName &rhs)
{
	strncpy(m_name, rhs.m_name, sizeof(m_name)-1);
	m_name[sizeof(m_name)-1] = '\0';
}

ElementName::ElementName(const char *str)
{
	strncpy(m_name, str, sizeof(m_name)-1);
	m_name[sizeof(m_name)-1] = '\0';
}

} // namespace upisnd
