#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# SPDX-License-Identifier: LGPL-2.1-only
#
# libpisoundmicro - a utility library for Pisound Micro I/O expander capabilities.
# Copyright (c) 2017-2025 Vilniaus Blokas UAB, https://blokas.io/
#
# This file is part of libpisoundmicro.
#
# libpisoundmicro is free software: you can redistribute it and/or modify it under the terms of the
# GNU Lesser General Public License as published by the Free Software Foundation, version 2.1 of the License.
#
# libpisoundmicro is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
# for more details.
#
# You should have received a copy of the GNU Lesser General Public License along with libpisoundmicro. If not, see <https://www.gnu.org/licenses/>.

import os
from .swig import pypisoundmicro as psm
from ._utils import copy_doc
from typing import Union

@copy_doc(psm.ValueFd)
class ValueFd:
	"""A wrapper around a file descriptor for reading/writing element values."""

	_fd_obj: psm.ValueFd = None

	def __init__(self, fd : Union[psm.ValueFd, int]):
		"""Create a ValueFd from an existing file descriptor.
		
		Args:
			fd (int): The file descriptor to wrap
		"""
		if isinstance(fd, psm.ValueFd):
			self._fd_obj = fd
		elif isinstance(fd, int):
			self._fd_obj = psm.ValueFd(fd)
		else:
			raise TypeError("fd must be a psm.ValueFd or an int")

	def __del__(self):
		"""Close the file descriptor when the object is garbage collected."""
		self.close()

	@copy_doc(psm.ValueFd.isValid)
	def is_valid(self):
		return self._fd_obj.isValid() if self._fd_obj else False

	@copy_doc(psm.ValueFd.take)
	def take(self) -> int:
		if self._fd_obj is None:
			return -1

		fd = self._fd_obj.take()
		self._fd_obj = None
		return fd

	@copy_doc(psm.ValueFd.get)
	def get(self) -> int:
		return self._fd_obj.get() if self._fd_obj else -1

	@copy_doc(psm.ValueFd.close)
	def close(self):
		if hasattr(self, '_fd_obj') and self._fd_obj is not None:
			err = self._fd_obj.close()
			if err != 0:
				raise OSError(f"Failed to close file descriptor: {err}")
			self._fd_obj = None
		return 0

	@copy_doc(psm.ValueFd.write)
	def write(self, value):
		return self._fd_obj.write(value) if self._fd_obj else -1

	def read(self):
		"""
		Reads a decimal number from the fd and returns it as integer.

		:raises OSError: If the file descriptor is invalid or if an error occurs during reading.
		:return: The read value.
		"""
		if self._fd_obj is None:
			return (-1, os.EBADF)

		result, err = self._fd_obj.read()
		if err != 0:
			raise OSError(f"Failed to read from file descriptor: {err}")

		return result
