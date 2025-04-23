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
#include <unistd.h>
#include <errno.h>

namespace upisnd
{

ValueFd::ValueFd()
	:m_fd(-1)
{
}

ValueFd::ValueFd(int fd)
	:m_fd(fd)
{
}

ValueFd::~ValueFd()
{
	close();
}

ValueFd::ValueFd(const ValueFd &rhs)
	:m_fd(rhs.m_fd >= 0 ? fcntl(rhs.m_fd, F_DUPFD_CLOEXEC, 0) : -1)
{
}

ValueFd& ValueFd::operator=(const ValueFd &rhs)
{
	if (this != &rhs)
	{
		close();
		m_fd = rhs.m_fd >= 0 ? fcntl(rhs.m_fd, F_DUPFD_CLOEXEC, 0) : -1;
	}
	return *this;
}

ValueFd::ValueFd(ValueFd &&rhs)
	:m_fd(rhs.m_fd)
{
	rhs.m_fd = -1;
}

ValueFd& ValueFd::operator=(ValueFd &&rhs)
{
	if (this != &rhs)
	{
		close();
		m_fd = rhs.m_fd;
		rhs.m_fd = -1;
	}
	return *this;
}

bool ValueFd::isValid() const
{
	return m_fd >= 0;
}

int ValueFd::take()
{
	int fd = m_fd;
	m_fd = -1;
	return fd;
}

int ValueFd::get() const
{
	return m_fd;
}

int ValueFd::close()
{
	if (m_fd >= 0)
	{
		int fd = m_fd;
		m_fd = -1;
		return ::close(fd);
	}
	return 0;
}

int ValueFd::write(int value)
{
	return upisnd_value_write(m_fd, value);
}

int ValueFd::read(int *err) const
{
	int r = upisnd_value_read(m_fd);
	if (err) *err = errno;
	return r;
}

} // namespace upisnd
