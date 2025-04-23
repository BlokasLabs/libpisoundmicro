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

#ifndef PISOUND_MICRO_VALUE_FD_H
#define PISOUND_MICRO_VALUE_FD_H

#ifndef PISOUND_MICRO_H
#	error Please #include <pisound-micro.h>!
#endif

/**
 * @file value_fd.h
 * @brief ValueFd class header.
 */

namespace upisnd
{

/** @brief A wrapper around a file descriptor that can be used to read/write the Element's value.
 *
 * Takes care of managing the lifetime of the fd.
 *
 * You may retrieve the fd value using get() or take ownership of it using take(). If you take
 * the ownership, the ValueFd object will immediately forget about the fd, and you'll be responsible
 * to `close` it yourself.
 */
class UPISND_API ValueFd
{
public:
	ValueFd();

	/// Creates a ValueFd from an existing fd, takes ownership of it, so it will close it in the destructor.
	explicit ValueFd(int fd); // Takes ownership of the fd.
	~ValueFd();

	/// Copy constructor, duplicates the fd using F_DUPFD_CLOEXEC fcntl call.
	ValueFd(const ValueFd &rhs);

	/// Copy assignment operator, first it closes any valid fd it may have had, then duplicates the rhs fd using F_DUPFD_CLOEXEC fcntl call.
	ValueFd& operator=(const ValueFd &rhs);

	/// Move constructor, moves take ownership of the fd.
	ValueFd(ValueFd &&rhs);

	/// Move assignment operator, first it closes any valid fd it may have had, then assumes ownership of the rhs fd, invalidating the rhs object.
	ValueFd& operator=(ValueFd &&rhs);

	/// Returns true if the object holds a valid fd.
	bool isValid() const;

	/// Returns the fd value, and won't close it in the destructor, transferring ownership.
	int take();

	/// Returns the fd value for your use, but keeps ownership, and will close it upon destruction.
	int get() const;

	/// Closes the fd and forgets about it.
	int close();

	/// Outputs a decimal number to the fd.
	int write(int value);

	/// Reads a decimal number from the fd and returns it as integer.
	/// @param err If not NULL, will be set to the `errno` value if the read failed, 0 otherwise.
	int read(int *err = NULL) const;

private:
	int m_fd;
};

} // namespace upisnd

#endif // PISOUND_MICRO_VALUE_FD_H
