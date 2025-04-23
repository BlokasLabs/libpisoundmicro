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

#include "utils.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

static const char UPISND_BASE64_TABLE[64] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'
};

int upisnd_base64_encode(char *dst, size_t dst_length, const void *d, size_t length, bool pad)
{
	char *p = dst;
	const uint8_t *data = (const uint8_t *)d;

	if (pad)
	{
		if (dst_length < 4 * ((length + 2) / 3) + 1)
			return -EINVAL;
	}
	else
	{
		if (dst_length < ((4 * length) + 2) / 3 + 1)
			return -EINVAL;
	}
	

	size_t i;
	for (i = 0; i + 3 <= length; i += 3)
	{
		*p++ = UPISND_BASE64_TABLE[data[i + 0] >> 2];
		*p++ = UPISND_BASE64_TABLE[((data[i + 0] & 0x03) << 4) | ((data[i + 1] & 0xf0) >> 4)];
		*p++ = UPISND_BASE64_TABLE[((data[i + 1] & 0x0f) << 2) | (data[i + 2] >> 6)];
		*p++ = UPISND_BASE64_TABLE[data[i + 2] & 0x3f];
	}

	switch (length - i)
	{
	case 0:
		break;
	case 1:
		*p++ = UPISND_BASE64_TABLE[data[i + 0] >> 2];
		*p++ = UPISND_BASE64_TABLE[((data[i + 0] & 0x03) << 4)];
		if (pad)
		{
			memset(p, '=', 2);
			p += 2;
		}
		break;
	case 2:
		*p++ = UPISND_BASE64_TABLE[data[i + 0] >> 2];
		*p++ = UPISND_BASE64_TABLE[((data[i + 0] & 0x03) << 4) | ((data[i + 1] & 0xf0) >> 4)];
		*p++ = UPISND_BASE64_TABLE[((data[i + 1] & 0x0f) << 2)];
		if (pad) *p++ = '=';
		break;
	}

	*p++ = '\0';

	return p - dst;
}

static inline uint32_t rotl(const uint32_t x, int k)
{
	return (x << k) | (x >> (32 - k));
}

uint32_t upisnd_xoshiro128_star_star_next(upisnd_xoshiro128_star_star_seed_t seed)
{
	const uint32_t result_starstar = rotl(seed[0] * 5, 7) * 9;

	const uint32_t t = seed[1] << 9;

	seed[2] ^= seed[0];
	seed[3] ^= seed[1];
	seed[1] ^= seed[2];
	seed[0] ^= seed[3];

	seed[2] ^= t;

	seed[3] = rotl(seed[3], 11);

	return result_starstar;
}
