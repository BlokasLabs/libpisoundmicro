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
#include "include/pisound-micro/api_internal.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <threads.h>
#include <stdatomic.h>

#include "utils.h"

static const uint32_t UPISND_TIMEOUT_MS = 2000ul;
static const char *const UPISND_TEXT_SEPARATORS = " \n\t";

static const char const UPISND_PIN_NAMES[UPISND_PIN_COUNT][4] =
{
	"A27", "A28", "A29", "A30", "A31", "A32", "B03", "B04",
	"B05", "B06", "B07", "B08", "B09", "B10", "B11", "B12",
	"B13", "B14", "B15", "B16", "B17", "B18", "B23", "B24",
	"B25", "B26", "B27", "B28", "B29", "B30", "B31", "B32",
	"B33", "B34", "B37", "B38", "B39"
};

enum
{
	UPISND_MAX_BASE_PATH_LENGTH    = 64,
	UPISND_ELEMENT_MAX_PATH_LENGTH = UPISND_MAX_ELEMENT_NAME_LENGTH + UPISND_MAX_BASE_PATH_LENGTH,
	UPISND_MAX_REQUEST_LENGTH      = UPISND_MAX_ELEMENT_NAME_LENGTH + 64,
};

static const char UPISND_SYSFS_DEFAULT_BASE_PATH[] = "/sys/pisound-micro";

static const char UPISND_PATH_SETUP_FMT[]          = "%s/setup";
static const char UPISND_PATH_UNSETUP_FMT[]        = "%s/unsetup";
#ifdef UPISND_INTERNAL
static const char UPISND_PATH_ADC_OFFSET_FMT[]     = "%s/adc_offset";
static const char UPISND_PATH_ADC_GAIN_FMT[]       = "%s/adc_gain";
#endif
static const char UPISND_ELEMENT_PATH_ATTR_FMT[]   = "%s/elements/%s/%s";

struct upisnd_ctx_t
{
	volatile int                       refcount;
	mtx_t                              mutex;
	const char                         *sysfs_base;
	upisnd_xoshiro128_star_star_seed_t seed;
	struct upisnd_elements_list_node_t *elements;
	struct upisnd_ctx_t                *next;
};

static struct upisnd_ctx_t *upisnd_active_ctx;
static struct upisnd_ctx_t *upisnd_ctx_list;

static struct upisnd_ctx_t *upisnd_ctx_alloc(const char *sysfs_base)
{
	upisnd_xoshiro128_star_star_seed_t seed;

	int rand_fd = open("/dev/urandom", O_CLOEXEC | O_RDONLY, 0);
	if (rand_fd < 0)
		return NULL;

	int err = read(rand_fd, seed, sizeof(seed));
	if (err != sizeof(seed))
		err = -errno;

	close(rand_fd);

	if (err < 0)
	{
		errno = -err;
		return NULL;
	}

	struct upisnd_ctx_t *ctx = malloc(sizeof(struct upisnd_ctx_t));
	if (!ctx)
	{
		errno = ENOMEM;
		return NULL;
	}

	ctx->refcount = 1;
	mtx_init(&ctx->mutex, mtx_plain);
	ctx->sysfs_base = sysfs_base ? sysfs_base : UPISND_SYSFS_DEFAULT_BASE_PATH;
	memcpy(ctx->seed, seed, sizeof(seed));
	ctx->elements = NULL;
	ctx->next = NULL;

	return ctx;
}

static void upisnd_ctx_free(struct upisnd_ctx_t *ctx);

static void upisnd_contexts_prepend(struct upisnd_ctx_t *ctx)
{
	ctx->next = upisnd_ctx_list;
	upisnd_ctx_list = ctx;
}

static int upisnd_contexts_remove(struct upisnd_ctx_t *ctx)
{
	int err = ENOENT;
	struct upisnd_ctx_t **n = &upisnd_ctx_list;

	while (*n)
	{
		if (*n == ctx)
		{
			err = 0;
			*n = ctx->next;
			break;
		}
		n = &(*n)->next;
	}

	if (err)
	{
		errno = err;
		return -err;
	}

	upisnd_ctx_free(ctx);

	errno = 0;
	return 0;
}

struct upisnd_elements_list_node_t
{
	volatile int                       refcount;
	const char                         *name;
	struct upisnd_ctx_t                *ctx;
	struct upisnd_elements_list_node_t *next;
};

static struct upisnd_elements_list_node_t *upisnd_element_alloc(const char *name, struct upisnd_ctx_t *ctx)
{
	struct upisnd_elements_list_node_t *node = malloc(sizeof(struct upisnd_elements_list_node_t));

	if (!node)
		return NULL;

	node->name = strdup(name);
	if (node->name)
	{
		node->refcount = 1;
		node->ctx = ctx;
		node->next = NULL;
		return node;
	}

	free(node);

	return NULL;
}

static void upisnd_elements_free(upisnd_element_ref_t el)
{
	free((void*)el->name);
	free(el);
}

static void upisnd_elements_prepend(struct upisnd_elements_list_node_t *node)
{
	node->next = node->ctx->elements;
	node->ctx->elements = node;
}

static int upisnd_elements_remove(upisnd_element_ref_t el)
{
	int err = ENOENT;
	struct upisnd_elements_list_node_t **n = &el->ctx->elements;

	while (*n)
	{
		if (*n == el)
		{
			err = 0;
			*n = el->next;
			break;
		}
		n = &(*n)->next;
	}

	if (err)
	{
		errno = err;
		return -err;
	}

	upisnd_elements_free(el);

	errno = 0;
	return 0;
}

static uint32_t upisnd_get_ms(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000ul + ts.tv_nsec / 1000000ull;
}

#ifndef UPISND_INTERNAL
static
#endif
struct upisnd_ctx_t *upisnd_set_active_ctx(struct upisnd_ctx_t *ctx)
{
	struct upisnd_ctx_t *old = upisnd_active_ctx;
	upisnd_active_ctx = ctx;
	return old;
}

#ifndef UPISND_INTERNAL
static
#endif
struct upisnd_ctx_t *upisnd_init_internal(const char *sysfs_base)
{
	if (!sysfs_base)
	{
		sysfs_base = UPISND_SYSFS_DEFAULT_BASE_PATH;
	}
	else
	{
		if (strlen(sysfs_base) > UPISND_MAX_BASE_PATH_LENGTH)
		{
			errno = ENAMETOOLONG;
			return NULL;
		}
		if (*sysfs_base != '/')
		{
			errno = EINVAL;
			return NULL;
		}
	}

	struct upisnd_ctx_t *ctx = upisnd_active_ctx;
	while (ctx)
	{
		if (strcmp(ctx->sysfs_base, sysfs_base) == 0)
		{
			__atomic_add_fetch(&ctx->refcount, 1, memory_order_seq_cst);
			upisnd_set_active_ctx(ctx);
			return ctx;
		}
		ctx = ctx->next;
	}

	ctx = upisnd_ctx_alloc(sysfs_base);
	if (ctx)
	{
		upisnd_contexts_prepend(ctx);
		upisnd_set_active_ctx(ctx);
	}

	return ctx;
}

typedef enum
{
	UPISND_PATH_SETUP,
	UPISND_PATH_UNSETUP,
#ifdef UPISND_INTERNAL
	UPISND_PATH_ADC_OFFSET,
	UPISND_PATH_ADC_GAIN,
#endif
} upisnd_path_type_t;

static int upisnd_path(char *path, upisnd_path_type_t type, const char *sysfs_base)
{
	int n;
	switch (type)
	{
	case UPISND_PATH_SETUP:
		n = snprintf(path, UPISND_MAX_BASE_PATH_LENGTH, UPISND_PATH_SETUP_FMT, sysfs_base);
		break;
	case UPISND_PATH_UNSETUP:
		n = snprintf(path, UPISND_MAX_BASE_PATH_LENGTH, UPISND_PATH_UNSETUP_FMT, sysfs_base);
		break;
	#ifdef UPISND_INTERNAL
	case UPISND_PATH_ADC_OFFSET:
		n = snprintf(path, UPISND_MAX_BASE_PATH_LENGTH, UPISND_PATH_ADC_OFFSET_FMT, sysfs_base);
		break;
	case UPISND_PATH_ADC_GAIN:
		n = snprintf(path, UPISND_MAX_BASE_PATH_LENGTH, UPISND_PATH_ADC_GAIN_FMT, sysfs_base);
		break;
	#endif
	default:
		return -EINVAL;
	}

	return n > 0 && n < UPISND_MAX_BASE_PATH_LENGTH ? n : -ENAMETOOLONG;
}

static int upisnd_open_fd(upisnd_path_type_t type, int flags, const char *sysfs_base)
{
	char path[UPISND_MAX_BASE_PATH_LENGTH];
	int err = upisnd_path(path, type, sysfs_base);
	if (err < 0)
		return err;

	errno = 0;
	int fd = open(path, flags, 0);
	if (fd < 0)
		return -errno;

	return fd;
}

typedef enum
{
	UPISND_ELEMENT_ATTR_ROOT,
	UPISND_ELEMENT_ATTR_TYPE,
	UPISND_ELEMENT_ATTR_DIRECTION,
	UPISND_ELEMENT_ATTR_PIN,
	UPISND_ELEMENT_ATTR_PIN_NAME,
	UPISND_ELEMENT_ATTR_PIN_PULL,
	UPISND_ELEMENT_ATTR_PIN_B,
	UPISND_ELEMENT_ATTR_PIN_B_NAME,
	UPISND_ELEMENT_ATTR_PIN_B_PULL,
	UPISND_ELEMENT_ATTR_GPIO_EXPORT,
	UPISND_ELEMENT_ATTR_GPIO_UNEXPORT,
	UPISND_ELEMENT_ATTR_INPUT_MIN,
	UPISND_ELEMENT_ATTR_INPUT_MAX,
	UPISND_ELEMENT_ATTR_VALUE_LOW,
	UPISND_ELEMENT_ATTR_VALUE_HIGH,
	UPISND_ELEMENT_ATTR_VALUE_MODE,
	UPISND_ELEMENT_ATTR_VALUE,
	UPISND_ELEMENT_ATTR_ACTIVITY_TYPE,
} upisnd_element_attr_e;

static int upisnd_element_path(char *path, const char *sysfs_base, const char *name, upisnd_element_attr_e attr)
{
	const char *a;
	switch (attr)
	{
	case UPISND_ELEMENT_ATTR_ROOT:          a = "";              break;
	case UPISND_ELEMENT_ATTR_TYPE:          a = "type";          break;
	case UPISND_ELEMENT_ATTR_DIRECTION:     a = "direction";     break;
	case UPISND_ELEMENT_ATTR_PIN:           a = "pin";           break;
	case UPISND_ELEMENT_ATTR_PIN_NAME:      a = "pin_name";      break;
	case UPISND_ELEMENT_ATTR_PIN_PULL:      a = "pin_pull";      break;
	case UPISND_ELEMENT_ATTR_PIN_B:         a = "pin_b";         break;
	case UPISND_ELEMENT_ATTR_PIN_B_NAME:    a = "pin_b_name";    break;
	case UPISND_ELEMENT_ATTR_PIN_B_PULL:    a = "pin_b_pull";    break;
	case UPISND_ELEMENT_ATTR_GPIO_EXPORT:   a = "gpio_export";   break;
	case UPISND_ELEMENT_ATTR_GPIO_UNEXPORT: a = "gpio_unexport"; break;
	case UPISND_ELEMENT_ATTR_INPUT_MIN:     a = "input_min";     break;
	case UPISND_ELEMENT_ATTR_INPUT_MAX:     a = "input_max";     break;
	case UPISND_ELEMENT_ATTR_VALUE_LOW:     a = "value_low";     break;
	case UPISND_ELEMENT_ATTR_VALUE_HIGH:    a = "value_high";    break;
	case UPISND_ELEMENT_ATTR_VALUE_MODE:    a = "value_mode";    break;
	case UPISND_ELEMENT_ATTR_VALUE:         a = "value";         break;
	case UPISND_ELEMENT_ATTR_ACTIVITY_TYPE: a = "activity_type"; break;
	default: return -EINVAL;
	}
	int n = snprintf(path, UPISND_ELEMENT_MAX_PATH_LENGTH, UPISND_ELEMENT_PATH_ATTR_FMT, sysfs_base, name, a);
	if (n > 0 && n < UPISND_ELEMENT_MAX_PATH_LENGTH)
	{
		if (attr == UPISND_ELEMENT_ATTR_ROOT)
			path[n--] = '\0'; // Strip trailing slash.
		return n;
	}
	return n > 0 && n < UPISND_ELEMENT_MAX_PATH_LENGTH ? n : -ENAMETOOLONG;
}

/**
 * Valid Element names are null terminated strings of characters, 1-63 long, and may not contain '`/`'.
 */
int upisnd_validate_element_name(const char *name)
{
	int n;
	if (!name || !*name || (n = strlen(name)) >= UPISND_MAX_ELEMENT_NAME_LENGTH)
		return -1;

	if (strchr(name, '/') != NULL)
		return -1;

	return n;
}

static int upisnd_element_attr_open(upisnd_element_attr_e attr, int flags, upisnd_element_ref_t el)
{
	if (!el)
	{
		errno = EINVAL;
		return -1;
	}

	char path[UPISND_ELEMENT_MAX_PATH_LENGTH];
	int err=upisnd_element_path(path, el->ctx->sysfs_base, el->name, attr);
	if (err < 0)
	{
		errno = -err;
		return -1;
	}

	int fd;
	uint32_t started_at = upisnd_get_ms();
	do
	{
		if ((fd = open(path, flags, 0)) >= 0)
		{
			errno = 0;
			return fd;
		}
		else
		{
			switch (errno)
			{
			case ENOENT:
			case EACCES:
				usleep(1000);
				continue;
			default:
				return -1;
			}
		}
	}
	while (upisnd_get_ms() - started_at < UPISND_TIMEOUT_MS); // Wait for up to UPISND_TIMEOUT_MS for udev permission rule to kick in.
	return -1;
}

static int upisnd_element_attr_read_int(upisnd_element_attr_e attr, upisnd_element_ref_t el)
{
	int value, err, fd = upisnd_element_attr_open(attr, O_CLOEXEC | O_RDONLY, el);
	if (fd < 0)
		return -1;

	value = upisnd_value_read(fd);

	err = errno;
	close(fd);

	errno = err;

	return value;
}

static int upisnd_element_attr_write_int(upisnd_element_attr_e attr, upisnd_element_ref_t el, int i)
{
	int err, fd = upisnd_element_attr_open(attr, O_CLOEXEC | O_WRONLY, el);
	if (fd < 0)
		return -1;

	err = upisnd_value_write(fd, i) < 0 ? errno : 0;

	close(fd);

	errno = err;

	return err == 0;
}

static int upisnd_element_attr_read_str(char *dst, size_t n, upisnd_element_attr_e attr, upisnd_element_ref_t el)
{
	if (!dst || n < 2)
	{
		errno = EINVAL;
		return -1;
	}

	int fd = upisnd_element_attr_open(attr, O_CLOEXEC | O_RDONLY, el);
	if (fd < 0)
		return -1;

	int r = read(fd, dst, n-1);
	int err;
	if (r >= 0)
	{
		err = 0;
		dst[r] = '\0';
		char *t = strpbrk(dst, UPISND_TEXT_SEPARATORS);
		if (t) *t = '\0';
	}
	else err = errno;

	close(fd);

	errno = err;
	return r;
}

static int upisnd_element_attr_write_str(upisnd_element_attr_e attr, upisnd_element_ref_t el, const char *str)
{
	int err, fd = upisnd_element_attr_open(attr, O_CLOEXEC | O_WRONLY, el);
	if (fd < 0)
		return -1;

	char s[UPISND_MAX_REQUEST_LENGTH+1];
	strncpy(s, str, UPISND_MAX_REQUEST_LENGTH);
	s[UPISND_MAX_REQUEST_LENGTH] = '\0';
	size_t n = strlen(s);

	errno = err = 0;
	if (lseek(fd, 0, SEEK_SET) < 0 || write(fd, s, n) < 0 || fdatasync(fd) < 0)
		err = errno;

	close(fd);
	if (err != 0)
	{
		errno = err;
		return -1;
	}

	return n;
}

int upisnd_generate_random_element_name(char *dst, size_t n, const char *prefix)
{
	struct upisnd_ctx_t *ctx = upisnd_active_ctx;
	if (!ctx)
	{
		errno = EINVAL;
		return -1;
	}

	uint32_t uuid[4];
	mtx_lock(&ctx->mutex);
	uuid[0] = upisnd_xoshiro128_star_star_next(ctx->seed);
	uuid[1] = upisnd_xoshiro128_star_star_next(ctx->seed);
	uuid[2] = upisnd_xoshiro128_star_star_next(ctx->seed);
	uuid[3] = upisnd_xoshiro128_star_star_next(ctx->seed);
	mtx_unlock(&ctx->mutex);

	char name[23];
	upisnd_base64_encode(name, sizeof(name), uuid, sizeof(uuid), false);
	if (prefix && *prefix)
	{
		return snprintf(dst, n, "%s-%s", prefix, name);
	}
	else
	{
		return snprintf(dst, n, "%s", name);
	}
}

static int upisnd_unsetup_do(const char *sysfs_base, const char *name)
{
	int fd = upisnd_open_fd(UPISND_PATH_UNSETUP, O_CLOEXEC | O_WRONLY, sysfs_base);

	if (fd < 0)
		return fd;

	int err = 0;

	if (write(fd, name, strlen(name)) < 0 || fdatasync(fd) < 0)
		err = errno;

	if (close(fd) < 0)
		err = errno;

	errno = err;
	return -err;
}

int upisnd_unsetup(const char *name)
{
	if (!upisnd_active_ctx)
	{
		errno = EINVAL;
		return -1;
	}
	return upisnd_unsetup_do(upisnd_active_ctx->sysfs_base, name);
}

upisnd_element_ref_t upisnd_element_get(const char *name)
{
	struct upisnd_ctx_t *ctx = upisnd_active_ctx;
	if (!ctx || upisnd_validate_element_name(name) < 0)
	{
		errno = EINVAL;
		return NULL;
	}

	mtx_lock(&ctx->mutex);

	struct upisnd_elements_list_node_t *el = ctx->elements;
	while (el)
	{
		if (strcmp(el->name, name) == 0)
		{
			upisnd_element_add_ref(el);
			errno = 0;
			break;
		}

		el = el->next;
	}

	mtx_unlock(&ctx->mutex);

	return el;
}

upisnd_element_ref_t upisnd_element_add_ref(upisnd_element_ref_t ref)
{
	if (ref)
	{
		__atomic_add_fetch(&ref->refcount, 1, memory_order_seq_cst);
		return ref;
	}
	return NULL;
}

void upisnd_element_unref(upisnd_element_ref_t *ref)
{
	if (!ref || !*ref)
		return;

	if (__atomic_sub_fetch(&(*ref)->refcount, 1, memory_order_seq_cst) == 0)
	{
		mtx_t *mtx = &(*ref)->ctx->mutex;
		mtx_lock(mtx);
		upisnd_unsetup_do((*ref)->ctx->sysfs_base, (*ref)->name);
		upisnd_elements_remove(*ref);
		mtx_unlock(mtx);
	}
	*ref = NULL;
}

int upisnd_element_open_value_fd(upisnd_element_ref_t el, int flags)
{
	return upisnd_element_attr_open(UPISND_ELEMENT_ATTR_VALUE, flags, el);
}

int upisnd_value_read(int fd)
{
	char value[16];
	errno = 0;
	int n;
	if (lseek(fd, 0, SEEK_SET) < 0 || (n = read(fd, value, 15)) < 0)
		return -1;
	value[n] = '\0';

	return strtol(value, NULL, 10);
}

int upisnd_value_write(int fd, int value)
{
	char s[16];
	int n = sprintf(s, "%d", value);
	errno = 0;
	if (lseek(fd, 0, SEEK_SET) < 0 || write(fd, s, n) < 0 || fdatasync(fd) < 0)
		return -1;

	return n;
}

bool upisnd_is_pin_valid(upisnd_pin_t pin)
{
	return pin < UPISND_PIN_COUNT;
}

const char *upisnd_pin_to_str(upisnd_pin_t pin)
{
	return upisnd_is_pin_valid(pin) ? UPISND_PIN_NAMES[pin] : "";
}

upisnd_pin_t upisnd_str_to_pin(const char *str)
{
	if (!str || strlen(str) != 3 || !isdigit(str[1]) || !isdigit(str[2]))
		return UPISND_PIN_INVALID;

	char sanitized[4];
	switch (*str)
	{
	case 'a': case 'A':
		sanitized[0] = 'A';
		break;
	case 'b': case 'B':
		sanitized[0] = 'B';
		break;
	default:
		return UPISND_PIN_INVALID;
	}

	memcpy(&sanitized[1], &str[1], 2);
	sanitized[3] = '\0';

	for (int i=0; i<UPISND_PIN_COUNT; ++i)
	{
		if (strcmp(sanitized, UPISND_PIN_NAMES[i]) == 0)
			return i;
	}

	return UPISND_PIN_INVALID;
}

#define UPISND_DEFINE_STR_TO_ENUM(middle_part, count) \
	upisnd_ ## middle_part ## _e upisnd_str_to_ ## middle_part(const char *str) \
	{ \
		for (int i=0; i<count; ++i) \
		{ \
			if (strcmp(str, upisnd_ ## middle_part ## _to_str((upisnd_ ## middle_part ## _e)i)) == 0) \
				return (upisnd_ ## middle_part ## _e)i; \
		} \
		return (upisnd_ ## middle_part ## _e)-1; \
	}

const char *upisnd_pin_pull_to_str(upisnd_pin_pull_e pull)
{
	switch (pull)
	{
	case UPISND_PIN_PULL_NONE: return "pull_none";
	case UPISND_PIN_PULL_UP:   return "pull_up";
	case UPISND_PIN_PULL_DOWN: return "pull_down";
	default: return "";
	}
}
UPISND_DEFINE_STR_TO_ENUM(pin_pull, UPISND_PIN_PULL_COUNT);

const char *upisnd_activity_to_str(upisnd_activity_e activity)
{
	switch (activity)
	{
	case UPISND_ACTIVITY_MIDI_INPUT:  return "midi_in";
	case UPISND_ACTIVITY_MIDI_OUTPUT: return "midi_out";
	default: return "";
	}
}
UPISND_DEFINE_STR_TO_ENUM(activity, UPISND_ACTIVITY_COUNT);

const char *upisnd_element_type_to_str(upisnd_element_type_e type)
{
	switch (type)
	{
	case UPISND_ELEMENT_TYPE_NONE:         return "none";
	case UPISND_ELEMENT_TYPE_ENCODER:      return "encoder";
	case UPISND_ELEMENT_TYPE_ANALOG_INPUT: return "analog_in";
	case UPISND_ELEMENT_TYPE_GPIO:         return "gpio";
	case UPISND_ELEMENT_TYPE_ACTIVITY:     return "activity";
	default: return "";
	}
}
UPISND_DEFINE_STR_TO_ENUM(element_type, UPISND_ELEMENT_TYPE_COUNT);

const char *upisnd_pin_direction_to_str(upisnd_pin_direction_e dir)
{
	switch (dir)
	{
	case UPISND_PIN_DIR_INPUT:  return "in";
	case UPISND_PIN_DIR_OUTPUT: return "out";
	default: return "";
	}
}
UPISND_DEFINE_STR_TO_ENUM(pin_direction, UPISND_PIN_DIR_COUNT);

const char *upisnd_value_mode_to_str(upisnd_value_mode_e mode)
{
	switch (mode)
	{
	case UPISND_VALUE_MODE_CLAMP: return "clamp";
	case UPISND_VALUE_MODE_WRAP:  return "wrap";
	default: return "";
	}
}
UPISND_DEFINE_STR_TO_ENUM(value_mode, UPISND_VALUE_MODE_COUNT);

#undef UPISND_DEFINE_STR_TO_ENUM

#define UPISND_DEFINE_INTERNAL_SETUP_FIELD(shift, bits, type, name) \
	static inline type upisnd_internal_setup_get_ ## name(upisnd_setup_t setup) { \
		return (type)((setup & (((1 << bits)-1) << shift)) >> shift); \
	} \
	static inline void upisnd_internal_setup_set_ ## name(upisnd_setup_t *setup, type value) { \
		*setup = ((*setup)&~(((1 << bits)-1) << shift)) | ((value & ((1 << bits)-1)) << shift); \
	}

UPISND_DEFINE_INTERNAL_SETUP_FIELD( 0, 3, upisnd_element_type_e,  element_type);
UPISND_DEFINE_INTERNAL_SETUP_FIELD( 3, 8, upisnd_pin_t,           pin_id);
UPISND_DEFINE_INTERNAL_SETUP_FIELD(11, 2, upisnd_pin_pull_e,      gpio_pull);
UPISND_DEFINE_INTERNAL_SETUP_FIELD(13, 1, upisnd_pin_direction_e, gpio_dir);
UPISND_DEFINE_INTERNAL_SETUP_FIELD(12, 1, bool,                   gpio_output);
UPISND_DEFINE_INTERNAL_SETUP_FIELD(13, 8, upisnd_pin_t,           encoder_pin_b_id);
UPISND_DEFINE_INTERNAL_SETUP_FIELD(21, 2, upisnd_pin_pull_e,      encoder_pin_b_pull);
UPISND_DEFINE_INTERNAL_SETUP_FIELD(11, 2, upisnd_activity_e,      activity_type);

#undef UPISND_DEFINE_INTERNAL_SETUP_FIELD

upisnd_element_type_e upisnd_setup_get_element_type(upisnd_setup_t setup)
{
	return upisnd_internal_setup_get_element_type(setup);
}

upisnd_pin_t upisnd_setup_get_pin_id(upisnd_setup_t setup)
{
	switch (upisnd_internal_setup_get_element_type(setup))
	{
	case UPISND_ELEMENT_TYPE_ENCODER:
	case UPISND_ELEMENT_TYPE_ANALOG_INPUT:
	case UPISND_ELEMENT_TYPE_GPIO:
	case UPISND_ELEMENT_TYPE_ACTIVITY:
		return upisnd_internal_setup_get_pin_id(setup);
	default:
		return UPISND_PIN_INVALID;
	}
}

upisnd_pin_pull_e upisnd_setup_get_gpio_pull(upisnd_setup_t setup)
{
	switch (upisnd_internal_setup_get_element_type(setup))
	{
	case UPISND_ELEMENT_TYPE_ENCODER:
		break;
	case UPISND_ELEMENT_TYPE_GPIO:
		if (upisnd_internal_setup_get_gpio_dir(setup) == UPISND_PIN_DIR_INPUT)
			break;
		// fallthrough intentional.
	default:
		return UPISND_PIN_PULL_INVALID;
	}

	return upisnd_internal_setup_get_gpio_pull(setup);
}

upisnd_pin_direction_e upisnd_setup_get_gpio_dir(upisnd_setup_t setup)
{
	return (upisnd_internal_setup_get_element_type(setup) == UPISND_ELEMENT_TYPE_GPIO) ? upisnd_internal_setup_get_gpio_dir(setup) : UPISND_PIN_DIR_INVALID;
}

int upisnd_setup_get_gpio_output(upisnd_setup_t setup)
{
	return (upisnd_internal_setup_get_element_type(setup) == UPISND_ELEMENT_TYPE_GPIO && 
		upisnd_internal_setup_get_gpio_dir(setup) == UPISND_PIN_DIR_OUTPUT) ?
		(upisnd_internal_setup_get_gpio_output(setup) ? 1 : 0) : -EINVAL;
}

upisnd_pin_t upisnd_setup_get_encoder_pin_b_id(upisnd_setup_t setup)
{
	return (upisnd_internal_setup_get_element_type(setup) == UPISND_ELEMENT_TYPE_ENCODER) ? upisnd_internal_setup_get_encoder_pin_b_id(setup) : UPISND_PIN_INVALID;
}

upisnd_pin_pull_e upisnd_setup_get_encoder_pin_b_pull(upisnd_setup_t setup)
{
	return (upisnd_internal_setup_get_element_type(setup) == UPISND_ELEMENT_TYPE_ENCODER) ? upisnd_internal_setup_get_encoder_pin_b_pull(setup) : UPISND_PIN_PULL_INVALID;
}

upisnd_activity_e upisnd_setup_get_activity_type(upisnd_setup_t setup)
{
	return (upisnd_internal_setup_get_element_type(setup) == UPISND_ELEMENT_TYPE_ACTIVITY) ? upisnd_internal_setup_get_activity_type(setup) : UPISND_ACTIVITY_INVALID;
}

int upisnd_setup_set_element_type(upisnd_setup_t *setup, upisnd_element_type_e value)
{
	if (value < 0 || value >= UPISND_ELEMENT_TYPE_COUNT)
		return -EINVAL;

	memset(setup, 0, sizeof(*setup));
	upisnd_internal_setup_set_element_type(setup, value);
	return 0;
}

int upisnd_setup_set_pin_id(upisnd_setup_t *setup, upisnd_pin_t value)
{
	switch (upisnd_internal_setup_get_element_type(*setup))
	{
	case UPISND_ELEMENT_TYPE_ENCODER:
	case UPISND_ELEMENT_TYPE_ANALOG_INPUT:
	case UPISND_ELEMENT_TYPE_GPIO:
	case UPISND_ELEMENT_TYPE_ACTIVITY:
		upisnd_internal_setup_set_pin_id(setup, value);
		return 0;
	default:
		return -EINVAL;
	}
}

int upisnd_setup_set_gpio_dir(upisnd_setup_t *setup, upisnd_pin_direction_e value)
{
	if (upisnd_internal_setup_get_element_type(*setup) != UPISND_ELEMENT_TYPE_GPIO)
		return -EINVAL;
	upisnd_internal_setup_set_gpio_dir(setup, value);
	return 0;
}

int upisnd_setup_set_gpio_pull(upisnd_setup_t *setup, upisnd_pin_pull_e value)
{
	switch (upisnd_internal_setup_get_element_type(*setup))
	{
	case UPISND_ELEMENT_TYPE_ENCODER:
		break;
	case UPISND_ELEMENT_TYPE_GPIO:
		if (upisnd_internal_setup_get_gpio_dir(*setup) == UPISND_PIN_DIR_INPUT)
			break;
		// fallthrough intentional.
	default:
		return -EINVAL;
	}

	upisnd_internal_setup_set_gpio_pull(setup, value);
	return 0;
}

int upisnd_setup_set_gpio_output(upisnd_setup_t *setup, bool value)
{
	if (upisnd_internal_setup_get_element_type(*setup) != UPISND_ELEMENT_TYPE_GPIO || upisnd_internal_setup_get_gpio_dir(*setup) != UPISND_PIN_DIR_OUTPUT)
		return -EINVAL;
	upisnd_internal_setup_set_gpio_output(setup, value);
	return 0;
}

int upisnd_setup_set_encoder_pin_b_id(upisnd_setup_t *setup, upisnd_pin_t value)
{
	if (upisnd_internal_setup_get_element_type(*setup) != UPISND_ELEMENT_TYPE_ENCODER)
		return -EINVAL;
	upisnd_internal_setup_set_encoder_pin_b_id(setup, value);
	return 0;
}

int upisnd_setup_set_encoder_pin_b_pull(upisnd_setup_t *setup, upisnd_pin_pull_e value)
{
	if (upisnd_internal_setup_get_element_type(*setup) != UPISND_ELEMENT_TYPE_ENCODER)
		return -EINVAL;
	upisnd_internal_setup_set_encoder_pin_b_pull(setup, value);
	return 0;
}

int upisnd_setup_set_activity_type(upisnd_setup_t *setup, upisnd_activity_e value)
{
	if (upisnd_internal_setup_get_element_type(*setup) != UPISND_ELEMENT_TYPE_ACTIVITY)
		return -EINVAL;
	upisnd_internal_setup_set_activity_type(setup, value);
	return 0;
}

static bool upisnd_element_exists_in_sysfs(const char *sysfs_base, const char *name)
{
	char path[UPISND_ELEMENT_MAX_PATH_LENGTH];
	int err = upisnd_element_path(path, sysfs_base, name, UPISND_ELEMENT_ATTR_ROOT);
	if (err < 0)
		return false;

	struct stat st;
	if (stat(path, &st) == 0)
	{
		if (st.st_mode & S_IFDIR)
			return true;
	}

	return false;
}

// Request_fmt without %s for name.
static upisnd_element_ref_t upisnd_setup_do(struct upisnd_ctx_t *ctx, const char *name, const char *request_fmt, ...)
{
	int name_len;
	if (!ctx || (name_len = upisnd_validate_element_name(name)) < 0)
	{
		errno = EINVAL;
		return NULL;
	}

	char path[UPISND_MAX_BASE_PATH_LENGTH];
	int err = upisnd_path(path, UPISND_PATH_SETUP, ctx->sysfs_base);

	if (err < 0)
	{
		errno = -err;
		return NULL;
	}

	char request[UPISND_MAX_REQUEST_LENGTH];
	strncpy(request, name, UPISND_MAX_ELEMENT_NAME_LENGTH);
	request[name_len++] = ' ';

	va_list ap;
	va_start(ap, request_fmt);
	int n = vsnprintf(request+name_len, sizeof(request)-name_len, request_fmt, ap);
	va_end(ap);
	if (n < 0 || n+name_len >= UPISND_MAX_REQUEST_LENGTH)
	{
		errno = EINVAL;
		return NULL;
	}

	struct upisnd_elements_list_node_t *el = upisnd_element_get(name);
	bool existing, existed_in_sysfs = false;

	if (el == NULL)
	{
		existing = false;
		el = upisnd_element_alloc(name, ctx);
		if (!el)
		{
			errno = ENOMEM;
			return NULL;
		}

		existed_in_sysfs = upisnd_element_exists_in_sysfs(ctx->sysfs_base, name);
	}
	else existing = true;

	int fd;

	mtx_lock(&ctx->mutex);

	fd = open(path, O_CLOEXEC | O_WRONLY, 0);

	if (fd >= 0)
	{
		err = 0;
		if (write(fd, request, n+name_len) < 0 || fdatasync(fd) < 0)
			err = errno;

		if (close(fd) < 0)
			err = errno;
	}
	else err = errno;

	if (err == 0 && !existing)
	{
		upisnd_elements_prepend(el);
		if (existed_in_sysfs)
			existing = true;
	}

	mtx_unlock(&ctx->mutex);

	if (err != 0)
	{
		if (existing && !existed_in_sysfs) upisnd_element_unref(&el);
		else upisnd_elements_free(el);
		errno = err;
		return NULL;
	}

	errno = !existing ? 0 : EEXIST;

	return el;
}

upisnd_element_ref_t upisnd_setup(const char *name, upisnd_setup_t setup)
{
	switch (upisnd_setup_get_element_type(setup))
	{
	case UPISND_ELEMENT_TYPE_ENCODER:
		return upisnd_setup_encoder(
			name,
			upisnd_internal_setup_get_pin_id(setup),
			upisnd_internal_setup_get_gpio_pull(setup),
			upisnd_internal_setup_get_encoder_pin_b_id(setup),
			upisnd_internal_setup_get_encoder_pin_b_pull(setup)
			);
	case UPISND_ELEMENT_TYPE_ANALOG_INPUT:
		return upisnd_setup_analog_input(name, upisnd_internal_setup_get_pin_id(setup));
	case UPISND_ELEMENT_TYPE_GPIO:
		switch (upisnd_internal_setup_get_gpio_dir(setup))
		{
		case UPISND_PIN_DIR_INPUT:
			return upisnd_setup_gpio_input(name, upisnd_internal_setup_get_pin_id(setup), upisnd_internal_setup_get_gpio_pull(setup));
		case UPISND_PIN_DIR_OUTPUT:
			return upisnd_setup_gpio_output(name, upisnd_internal_setup_get_pin_id(setup), upisnd_internal_setup_get_gpio_output(setup));
		default:
			break;
		}
		break;
	case UPISND_ELEMENT_TYPE_ACTIVITY:
		return upisnd_setup_activity(name, upisnd_internal_setup_get_pin_id(setup), upisnd_internal_setup_get_activity_type(setup));
	default:
		break;
	}

	errno = EINVAL;
	return NULL;
}

/**
 * @param name The name of the Element to set up.
 * @param pin_a The pin number of the first pin of the Encoder.
 * @param pull_a The pull of the first pin of the Encoder.
 * @param pin_b The pin number of the second pin of the Encoder.
 * @param pull_b The pull of the second pin of the Encoder.
 *
 * @see ::upisnd_setup for more details.
 *
 * @return A valid Element reference on success, NULL on error, inspect `errno` for details.
 */
upisnd_element_ref_t upisnd_setup_encoder(const char *name, upisnd_pin_t pin_a, upisnd_pin_pull_e pull_a, upisnd_pin_t pin_b, upisnd_pin_pull_e pull_b)
{
	return upisnd_setup_do(upisnd_active_ctx, name, "encoder %s %s %s %s", upisnd_pin_to_str(pin_a), upisnd_pin_pull_to_str(pull_a), upisnd_pin_to_str(pin_b), upisnd_pin_pull_to_str(pull_b));
}

/**
 * @param name The name of the Element to set up.
 * @param pin The pin number of the Analog Input.
 * @param mode The value mode of the Analog Input.
 *
 * @see ::upisnd_setup for more details.
 *
 * @return A valid Element reference on success, NULL on error, inspect `errno` for details.
 */
upisnd_element_ref_t upisnd_setup_analog_input(const char *name, upisnd_pin_t pin)
{
	return upisnd_setup_do(upisnd_active_ctx, name, "analog_in %s ", upisnd_pin_to_str(pin));
}

/**
 * @param name The name of the Element to set up.
 * @param pin The pin number of the GPIO Input.
 * @param pull The pull of the GPIO Input.
 *
 * @see ::upisnd_setup for more details.
 *
 * @return A valid Element reference on success, NULL on error, inspect `errno` for details.
 */
upisnd_element_ref_t upisnd_setup_gpio_input(const char *name, upisnd_pin_t pin, upisnd_pin_pull_e pull)
{
	return upisnd_setup_do(upisnd_active_ctx, name, "gpio %s input %s", upisnd_pin_to_str(pin), upisnd_pin_pull_to_str(pull));
}

/**
 * @param name The name of the Element to set up.
 * @param pin The pin number of the GPIO Output.
 * @param high Whether the GPIO Output should be high or low.
 *
 * @see ::upisnd_setup for more details.
 *
 * @return A valid Element reference on success, NULL on error, inspect `errno` for details.
 */
upisnd_element_ref_t upisnd_setup_gpio_output(const char *name, upisnd_pin_t pin, bool high)
{
	return upisnd_setup_do(upisnd_active_ctx, name, "gpio %s output %c", upisnd_pin_to_str(pin), high ? '1' : '0');
}

/**
 * @param name The name of the Element to set up.
 * @param pin The pin number to indicate the Activity on.
 * @param activity The activity type of the Activity Element.
 *
 * @see ::upisnd_setup for more details.
 *
 * @return A valid Element reference on success, NULL on error, inspect `errno` for details.
 */
upisnd_element_ref_t upisnd_setup_activity(const char *name, upisnd_pin_t pin, upisnd_activity_e activity)
{
	return upisnd_setup_do(upisnd_active_ctx, name, "activity_%s %s", upisnd_activity_to_str(activity), upisnd_pin_to_str(pin));
}

upisnd_pin_direction_e upisnd_element_gpio_get_direction(upisnd_element_ref_t el)
{
	if (!el)
	{
		errno = EINVAL;
		return UPISND_PIN_DIR_INVALID;
	}

	char dir[64];
	int err = upisnd_element_attr_read_str(dir, sizeof(dir), UPISND_ELEMENT_ATTR_DIRECTION, el);
	if (err < 0)
		return UPISND_PIN_DIR_INVALID;

	upisnd_pin_direction_e d = upisnd_str_to_pin_direction(dir);

	if (d == UPISND_PIN_DIR_INVALID)
	{
		errno = EINVAL;
		return UPISND_PIN_DIR_INVALID;
	}

	return d;
}

static upisnd_pin_pull_e upisnd_element_get_pull(upisnd_element_ref_t el, upisnd_element_attr_e attr)
{
	if (!el)
	{
		errno = EINVAL;
		return UPISND_PIN_PULL_INVALID;
	}

	char pull[64];
	int err = upisnd_element_attr_read_str(pull, sizeof(pull), attr, el);
	if (err < 0)
		return UPISND_PIN_PULL_INVALID;

	upisnd_pin_pull_e p = upisnd_str_to_pin_pull(pull);

	if (p == UPISND_PIN_PULL_INVALID)
	{
		errno = EINVAL;
		return UPISND_PIN_PULL_INVALID;
	}

	return p;
}

upisnd_pin_pull_e upisnd_element_gpio_get_pull(upisnd_element_ref_t el)
{
	return upisnd_element_get_pull(el, UPISND_ELEMENT_ATTR_PIN_PULL);
}

upisnd_activity_e upisnd_element_activity_get_type(upisnd_element_ref_t el)
{
	if (!el)
	{
		errno = EINVAL;
		return UPISND_ACTIVITY_INVALID;
	}

	char activity[64];
	int err = upisnd_element_attr_read_str(activity, sizeof(activity), UPISND_ELEMENT_ATTR_ACTIVITY_TYPE, el);
	if (err < 0)
		return UPISND_ACTIVITY_INVALID;

	upisnd_activity_e a = upisnd_str_to_activity(activity);

	if (a == UPISND_ACTIVITY_INVALID)
	{
		errno = EINVAL;
		return UPISND_ACTIVITY_INVALID;
	}

	return a;
}

void upisnd_element_encoder_init_default_opts(upisnd_encoder_opts_t *opts)
{
	opts->input_range.low = opts->value_range.low = 0;
	opts->input_range.high = opts->value_range.high = 23;
	opts->value_mode = UPISND_VALUE_MODE_CLAMP;
}

int upisnd_element_encoder_get_opts(upisnd_element_ref_t el, upisnd_encoder_opts_t *opts)
{
	struct
	{
		upisnd_element_attr_e attr;
		int *dst;
	} t[4] =
	{
		{ UPISND_ELEMENT_ATTR_INPUT_MIN,  &opts->input_range.low  },
		{ UPISND_ELEMENT_ATTR_INPUT_MAX,  &opts->input_range.high },
		{ UPISND_ELEMENT_ATTR_VALUE_LOW,  &opts->value_range.low  },
		{ UPISND_ELEMENT_ATTR_VALUE_HIGH, &opts->value_range.high },
	};

	for (int i=0; i<sizeof(t)/sizeof(t[0]); ++i)
	{
		*t[i].dst = upisnd_element_attr_read_int(t[i].attr, el);
		if (errno != 0)
			return -1;
	}

	char mode[64];
	int err = upisnd_element_attr_read_str(mode, sizeof(mode), UPISND_ELEMENT_ATTR_VALUE_MODE, el);
	if (err < 0)
		return -1;

	opts->value_mode = upisnd_str_to_value_mode(mode);
	if (opts->value_mode == UPISND_VALUE_MODE_INVALID)
	{
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int upisnd_element_encoder_set_opts(upisnd_element_ref_t el, const upisnd_encoder_opts_t *opts)
{
	int err;
	(err = upisnd_element_attr_write_int(UPISND_ELEMENT_ATTR_INPUT_MIN,  el, opts->input_range.low))  >= 0 &&
	(err = upisnd_element_attr_write_int(UPISND_ELEMENT_ATTR_INPUT_MAX,  el, opts->input_range.high)) >= 0 &&
	(err = upisnd_element_attr_write_int(UPISND_ELEMENT_ATTR_VALUE_LOW,  el, opts->value_range.low))  >= 0 &&
	(err = upisnd_element_attr_write_int(UPISND_ELEMENT_ATTR_VALUE_HIGH, el, opts->value_range.high)) >= 0 &&
	(err = upisnd_element_attr_write_str(UPISND_ELEMENT_ATTR_VALUE_MODE, el, upisnd_value_mode_to_str(opts->value_mode))) >= 0;

	return err >= 0 ? 0 : err;
}

upisnd_pin_t upisnd_element_encoder_get_pin_b(upisnd_element_ref_t el)
{
	if (!el)
	{
		errno = EINVAL;
		return UPISND_PIN_INVALID;
	}

	int i = upisnd_element_attr_read_int(UPISND_ELEMENT_ATTR_PIN_B, el);
	return upisnd_is_pin_valid(i) ? (upisnd_pin_t)i : UPISND_PIN_INVALID;
}

upisnd_pin_pull_e upisnd_element_encoder_get_pin_b_pull(upisnd_element_ref_t el)
{
	return upisnd_element_get_pull(el, UPISND_ELEMENT_ATTR_PIN_B_PULL);
}

void upisnd_element_analog_input_init_default_opts(upisnd_analog_input_opts_t *opts)
{
	opts->input_range.low = opts->value_range.low = 0;
	opts->input_range.high = opts->value_range.high = 1023;
}

int upisnd_element_analog_input_get_opts(upisnd_element_ref_t el, upisnd_analog_input_opts_t *opts)
{
	struct
	{
		upisnd_element_attr_e attr;
		int *dst;
	} t[4] =
	{
		{ UPISND_ELEMENT_ATTR_INPUT_MIN,  &opts->input_range.low  },
		{ UPISND_ELEMENT_ATTR_INPUT_MAX,  &opts->input_range.high },
		{ UPISND_ELEMENT_ATTR_VALUE_LOW,  &opts->value_range.low  },
		{ UPISND_ELEMENT_ATTR_VALUE_HIGH, &opts->value_range.high },
	};

	for (int i=0; i<sizeof(t)/sizeof(t[0]); ++i)
	{
		*t[i].dst = upisnd_element_attr_read_int(t[i].attr, el);
		if (errno != 0)
			return -1;
	}

	return 0;
}

int upisnd_element_analog_input_set_opts(upisnd_element_ref_t el, const upisnd_analog_input_opts_t *opts)
{
	int err;
	(err = upisnd_element_attr_write_int(UPISND_ELEMENT_ATTR_INPUT_MIN,  el, opts->input_range.low))  >= 0 &&
	(err = upisnd_element_attr_write_int(UPISND_ELEMENT_ATTR_INPUT_MAX,  el, opts->input_range.high)) >= 0 &&
	(err = upisnd_element_attr_write_int(UPISND_ELEMENT_ATTR_VALUE_LOW,  el, opts->value_range.low))  >= 0 &&
	(err = upisnd_element_attr_write_int(UPISND_ELEMENT_ATTR_VALUE_HIGH, el, opts->value_range.high)) >= 0;

	return err >= 0 ? 0 : err;
}

const char *upisnd_element_get_name(upisnd_element_ref_t el)
{
	return el ? el->name : NULL;
}

upisnd_element_type_e upisnd_element_get_type(upisnd_element_ref_t el)
{
	if (!el)
		return UPISND_ELEMENT_TYPE_INVALID;

	char type[64];
	int n = upisnd_element_attr_read_str(type, sizeof(type), UPISND_ELEMENT_ATTR_TYPE, el);

	if (n < 0)
		return UPISND_ELEMENT_TYPE_INVALID;

	return upisnd_str_to_element_type(type);
}

upisnd_pin_t upisnd_element_get_pin(upisnd_element_ref_t el)
{
	if (!el)
	{
		errno = EINVAL;
		return UPISND_PIN_INVALID;
	}

	int i = upisnd_element_attr_read_int(UPISND_ELEMENT_ATTR_PIN, el);
	return upisnd_is_pin_valid(i) ? (upisnd_pin_t)i : UPISND_PIN_INVALID;
}

int upisnd_init(void)
{
	return upisnd_init_internal(NULL) != NULL ? 0 : -1;
}

static int upisnd_unsetup_all_elements(struct upisnd_ctx_t *ctx)
{
	int fd;

	mtx_lock(&ctx->mutex);

	if (!ctx->elements)
	{
		mtx_unlock(&ctx->mutex);
		return 0;
	}

	fd = upisnd_open_fd(UPISND_PATH_UNSETUP, O_CLOEXEC | O_WRONLY, ctx->sysfs_base);

	if (fd < 0)
	{
		mtx_unlock(&ctx->mutex);
		return fd;
	}

	struct upisnd_elements_list_node_t *n = ctx->elements;
	while (n)
	{
		struct upisnd_elements_list_node_t *next = n->next;

		write(fd, n->name, strlen(n->name));
		fdatasync(fd);
		lseek(fd, 0, SEEK_SET);

		upisnd_elements_free(n);
		n = next;
	}

	ctx->elements = NULL;

	close(fd);

	mtx_unlock(&ctx->mutex);

	return 0;
}

void upisnd_uninit(void)
{
	if (upisnd_active_ctx && __atomic_sub_fetch(&upisnd_active_ctx->refcount, 1, memory_order_seq_cst) == 0)
	{
		upisnd_contexts_remove(upisnd_active_ctx);
		upisnd_active_ctx = upisnd_ctx_list;
	}
}

static void upisnd_ctx_free(struct upisnd_ctx_t *ctx)
{
	upisnd_unsetup_all_elements(ctx);
	mtx_destroy(&ctx->mutex);
	free(ctx);
}

#ifdef UPISND_INTERNAL
int upisnd_set_adc_offset(int16_t offset)
{
	if (!upisnd_active_ctx)
	{
		errno = ENODEV;
		return -ENODEV;
	}

	int fd = upisnd_open_fd(UPISND_PATH_ADC_OFFSET, O_CLOEXEC | O_WRONLY, upisnd_active_ctx->sysfs_base);

	if (fd < 0)
		return fd;

	int err = 0;

	char buf[8];
	int n = snprintf(buf, sizeof(buf), "%d", offset);

	if (write(fd, buf, n) < 0 || fdatasync(fd) < 0)
		err = errno;

	if (close(fd) < 0)
		err = errno;

	errno = err;
	return -err;
}

UPISND_API int16_t upisnd_get_adc_offset(void)
{
	if (!upisnd_active_ctx)
	{
		errno = ENODEV;
		return -ENODEV;
	}

	int fd = upisnd_open_fd(UPISND_PATH_ADC_OFFSET, O_CLOEXEC | O_RDONLY, upisnd_active_ctx->sysfs_base);

	if (fd < 0)
		return fd;

	int err = 0;

	char buf[8];
	int n = read(fd, buf, sizeof(buf)-1);

	if (n < 0)
		err = errno;
	else
	{
		buf[n] = '\0';
		errno = 0;
		n = strtol(buf, NULL, 10);
		if (errno)
			err = errno;
	}

	if (close(fd) < 0)
		err = errno;

	errno = err;
	return err ? -err : n;
}

int upisnd_set_adc_gain(uint16_t gain)
{
	if (!upisnd_active_ctx)
	{
		errno = ENODEV;
		return -ENODEV;
	}

	int fd = upisnd_open_fd(UPISND_PATH_ADC_GAIN, O_CLOEXEC | O_WRONLY, upisnd_active_ctx->sysfs_base);

	if (fd < 0)
		return fd;

	int err = 0;

	char buf[8];
	int n = snprintf(buf, sizeof(buf), "%u", gain);

	if (write(fd, buf, n) < 0 || fdatasync(fd) < 0)
		err = errno;

	if (close(fd) < 0)
		err = errno;

	errno = err;
	return -err;
}

UPISND_API uint16_t upisnd_get_adc_gain(void)
{
	if (!upisnd_active_ctx)
	{
		errno = ENODEV;
		return -ENODEV;
	}

	int fd = upisnd_open_fd(UPISND_PATH_ADC_GAIN, O_CLOEXEC | O_RDONLY, upisnd_active_ctx->sysfs_base);

	if (fd < 0)
		return fd;

	int err = 0;

	char buf[8];
	int n = read(fd, buf, sizeof(buf)-1);

	if (n < 0)
		err = errno;
	else
	{
		buf[n] = '\0';
		errno = 0;
		n = strtoul(buf, NULL, 10);
		if (errno)
			err = errno;
	}

	if (close(fd) < 0)
		err = errno;

	errno = err;
	return err ? -err : n;
}
#endif
