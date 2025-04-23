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

VERSION=1.0.0

PREFIX?=/usr/local
INCLUDEDIR?=$(PREFIX)/include
LIBDIR?=$(PREFIX)/lib

CC?=cc
AR?=ar

CFLAGS?=-O3 -fvisibility=hidden -fno-exceptions
CFLAGS += -Iinclude

INSTALL?=install
INSTALL_PROGRAM?=$(INSTALL)
INSTALL_DATA?=$(INSTALL) -m 644

-include Makefile.local

ifeq ($(DEBUG),yes)
	CFLAGS += -DDEBUG -g -O0
endif

INSTALL_ADDITIONAL_HDRS=
ifeq ($(INTERNAL),yes)
	CFLAGS += -DUPISND_INTERNAL
	INSTALL_ADDITIONAL_HDRS+=api_internal.h
endif

CXXFLAGS=$(CFLAGS) -fno-rtti

all: libpisoundmicro.so

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

LIB_OBJS=pisound-micro.o element.o encoder.o analog_input.o gpio.o activity.o lib_initializer.o value_fd.o utils.o name.o

$(LIB_OBJS): CFLAGS+=-DUPISND_BUILDING_LIB -fPIC

libpisoundmicro.so: $(LIB_OBJS)
	$(CXX) -Wl,--no-undefined -shared -Wl,-soname,$@.0 -g $^ -lpthread -o$@.0
	ln -fs $@.0 $@

#pisoundmicro_wrap.cxx: pisoundmicro.i
#	swig -python -py3 -includeall -c++ $^
#
#_pisoundmicro.so: pisoundmicro.py pisoundmicro_wrap.cxx
#	$(CXX) -O2 $(CXXFLAGS) -shared -fPIC -I/usr/include/python3.9 pisoundmicro_wrap.cxx -lpisoundmicro -o _pisoundmicro.so

install: libpisoundmicro.so
	mkdir -p $(DESTDIR)$(INCLUDEDIR)/pisound-micro
	mkdir -p $(DESTDIR)$(LIBDIR)
	$(INSTALL_DATA) include/pisound-micro.h $(DESTDIR)$(INCLUDEDIR)/
	cd include/pisound-micro && $(INSTALL_DATA) \
		types.h api.h api_cpp.h element.h encoder.h analog_input.h gpio.h activity.h lib_initializer.h value_fd.h name.h $(INSTALL_ADDITIONAL_HDRS) \
		$(DESTDIR)$(INCLUDEDIR)/pisound-micro
	$(INSTALL_DATA) libpisoundmicro.so.0 $(DESTDIR)$(LIBDIR)/
	ln -fs $(DESTDIR)$(LIBDIR)/libpisoundmicro.so.0 $(DESTDIR)$(LIBDIR)/libpisoundmicro.so
	@echo
	@echo "You may want to run \`sudo ldconfig\`, especially if you get startup errors about libpisoundmicro.so."

test: libpisoundmicro_test.o libpisoundmicro.so
	$(CXX) $(CFLAGS) $^ -lpthread -o $@

clean:
	rm -f *.o *.a *.so *.so.* test
	rm -rf docs

dist:
	git archive --format=tar.gz --prefix=libpisoundmicro-$(VERSION)/ HEAD -o libpisoundmicro-$(VERSION).tar.gz

docs:
	doxygen

.PHONY: clean docs
