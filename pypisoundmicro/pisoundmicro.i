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
%feature("autodoc", "1");

%include <std_string.i>

// Typemap for handling upisnd_setup_t* parameters properly
%typemap(in) upisnd_setup_t* (upisnd_setup_t temp) {
	temp = (upisnd_setup_t) PyLong_AsLong($input);
	$1 = &temp;
}

%typemap(argout) upisnd_setup_t* {
	$result = SWIG_Python_AppendOutput($result, PyLong_FromLong(*$1));
}

// Typemaps for handling enum types
%typemap(in) upisnd_element_type_e {
	$1 = (upisnd_element_type_e) PyLong_AsLong($input);
}

%typemap(in) upisnd_pin_t {
	$1 = (upisnd_pin_t) PyLong_AsLong($input);
}

%typemap(in) upisnd_pin_direction_e {
	$1 = (upisnd_pin_direction_e) PyLong_AsLong($input);
}

%typemap(in) upisnd_pin_pull_e {
	$1 = (upisnd_pin_pull_e) PyLong_AsLong($input);
}

%typemap(in) upisnd_activity_e {
	$1 = (upisnd_activity_e) PyLong_AsLong($input);
}

%typemap(in) upisnd_value_mode_e {
	$1 = (upisnd_value_mode_e) PyLong_AsLong($input);
}

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
//%ignore upisnd::AnalogInput::getOpts;
//%ignore upisnd::Encoder::getOpts;

// Ignore the templated 'as' method since it can't be directly mapped to Python
%ignore upisnd::Element::as;

// Instead, add explicit as_* methods for each derived type that directly use the templated 'as' method
%extend upisnd::Element {
	upisnd::AnalogInput* as_analog_input() {
		return new upisnd::AnalogInput(self->as<upisnd::AnalogInput>());
	}

	upisnd::Encoder* as_encoder() {
		return new upisnd::Encoder(self->as<upisnd::Encoder>());
	}

	upisnd::Gpio* as_gpio() {
		return new upisnd::Gpio(self->as<upisnd::Gpio>());
	}

	upisnd::Activity* as_activity() {
		return new upisnd::Activity(self->as<upisnd::Activity>());
	}
}

// Typemap for ValueFd::read(int *err) to return a tuple (value, error)
%typemap(in, numinputs=0) int *err (int temp) {
    temp = 0;
    $1 = &temp;
}

%typemap(argout) int *err {
    PyObject *o = PyTuple_New(2);
    PyTuple_SetItem(o, 0, $result);
    PyTuple_SetItem(o, 1, PyLong_FromLong(*$1));
    $result = o;
}

%include <pisound-micro.h>

%pythoncode %{
import atexit
from threading import Lock

_upisnd_mutex = Lock()

_upisnd_initializer = None

def _init():
	global _upisnd_initializer
	with _upisnd_mutex:
		if _upisnd_initializer is None:
			_upisnd_initializer = LibInitializer()

def _cleanup():
	global _upisnd_initializer
	with _upisnd_mutex:
		if _upisnd_initializer is not None:
			del _upisnd_initializer
			_upisnd_initializer = None

# Register the cleanup function to be called on normal interpreter shutdown.
atexit.register(_cleanup)

_init()
%}
