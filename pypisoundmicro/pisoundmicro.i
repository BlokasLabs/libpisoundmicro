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

%module pypisoundmicro

%include <std_string.i>

%typemap(in) upisnd::ElementName (void *argp, int res) {
	if (PyUnicode_Check($input)) {
		$1 = PyUnicode_AsUTF8($input);
	} else {
		res = SWIG_ConvertPtr($input, &argp, $descriptor(upisnd::ElementName*), 0);
		if (!SWIG_IsOK(res)) {
			SWIG_exception_fail(SWIG_ArgError(res), "in method '" "$symname" "', argument " "$argnum"" of type '" "$1_type""'"); 
		}
		if (!argp) {
			SWIG_exception_fail(SWIG_ValueError, "invalid null reference " "in method '" "$symname" "', argument " "$argnum"" of type '" "$1_type""'");
		} else {
			$1_type * temp = reinterpret_cast< $1_type * >(argp);
			$1 = *temp;
			if (SWIG_IsNewObj(res)) delete temp;
		}
	}
}

%{
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

#define SWIG_FILE_WITH_INIT
#include <pisound-micro.h>
%}

%include <stdint.i>

%ignore upisnd::ElementName::formatted;
%rename(__str__) upisnd::ElementName::operator const char*() const;

%include <pisound-micro.h>

%pythoncode %{
_upisnd_initializer = LibInitializer()
%}
