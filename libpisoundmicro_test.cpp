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

#include <pisound-micro.h>

#include <stdio.h>
#include <errno.h>

using namespace upisnd;

int main(int argc, char **argv)
{
	LibInitializer initializer;

	Encoder e = Encoder::setup("enc", UPISND_PIN_B03, UPISND_PIN_PULL_UP, UPISND_PIN_B04, UPISND_PIN_PULL_DOWN);

	printf("setup, e.isvalid()=%d, errno=%d (%m)\n", e.isValid(), errno);

#if 0
	Element r = Element::exportRandom("test");

	Element _ = Element::exportRandom(NULL);
	Encoder u = _.setupEncoder(UPISND_PIN_B03, UPISND_PIN_PULL_UP, UPISND_PIN_B04, UPISND_PIN_PULL_DOWN);

	Encoder x = _.as<Encoder>();
	Encoder y = r.as<Encoder>();

	bool a = x.isValid();
	bool b = y.isValid();
#endif

	return 0;
}
