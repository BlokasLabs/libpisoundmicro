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

#ifndef PISOUND_MICRO_TYPES_H
#define PISOUND_MICRO_TYPES_H

/**
 * @file types.h
 * @brief This header defines the various types of libpisoundmicro.
 */

#ifndef SWIG
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#endif

#ifndef PISOUND_MICRO_H
#	error Please #include <pisound-micro.h>!
#endif

/** @brief A refcounted handle to a Pisound Micro Element.
 *
 * Every upisnd_element_ref_t that gets returned by the APIs,
 * must be unrefed using upisnd_element_unref, once you are done
 * with it.
 */
typedef struct upisnd_elements_list_node_t *upisnd_element_ref_t;

/** @brief A container type for all Pisound Micro Element setup options.
 *
 * Does not include extended Analog Input and Encoder options, which can
 * be accessed via upisnd_encoder_opts_t and upisnd_analog_input_opts_t.
 */
typedef uint32_t upisnd_setup_t;

enum
{
	/// Maximum Element name length, the size includes the '`\0`' character.
	UPISND_MAX_ELEMENT_NAME_LENGTH = 64,
};

typedef enum
{
	UPISND_ELEMENT_TYPE_INVALID = -1,
	UPISND_ELEMENT_TYPE_NONE,
	UPISND_ELEMENT_TYPE_ENCODER,
	UPISND_ELEMENT_TYPE_ANALOG_INPUT,
	UPISND_ELEMENT_TYPE_GPIO,
	UPISND_ELEMENT_TYPE_ACTIVITY,

	// Must be the last one!
	UPISND_ELEMENT_TYPE_COUNT
} upisnd_element_type_e;

/// @brief Activity type.
typedef enum
{
	UPISND_ACTIVITY_INVALID = -1,
	UPISND_ACTIVITY_MIDI_INPUT,
	UPISND_ACTIVITY_MIDI_OUTPUT,

	// Must be the last one!
	UPISND_ACTIVITY_COUNT
} upisnd_activity_e;

/// @brief GPIO pin pull-up/down configuration.
typedef enum
{
	/// Invalid value.
	UPISND_PIN_PULL_INVALID = -1,
	UPISND_PIN_PULL_NONE,
	UPISND_PIN_PULL_UP,
	UPISND_PIN_PULL_DOWN,

	// Must be the last one!
	UPISND_PIN_PULL_COUNT
} upisnd_pin_pull_e;


/// @brief GPIO pin direction.
typedef enum
{
	UPISND_PIN_DIR_INVALID = -1,
	UPISND_PIN_DIR_INPUT,
	UPISND_PIN_DIR_OUTPUT,

	// Must be the last one!
	UPISND_PIN_DIR_COUNT
} upisnd_pin_direction_e;

/** @brief The Pisound Micro header pin number constants.
 *
 * There's 4 sequential enum value ranges:
 *
 * `UPISND_PIN_A27` - `UPISND_PIN_A32` \n
 * `UPISND_PIN_B03` - `UPISND_PIN_B18` \n
 * `UPISND_PIN_B23` - `UPISND_PIN_B34` \n
 * `UPISND_PIN_B37` - `UPISND_PIN_B39` \n
 * \n
 * For a total of 37 pins. (`UPISND_PIN_COUNT`)
 *
 * @see #upisnd_pin_t
 */
typedef enum
{
	UPISND_PIN_A27, UPISND_PIN_A28, UPISND_PIN_A29, UPISND_PIN_A30,
	UPISND_PIN_A31, UPISND_PIN_A32, UPISND_PIN_B03, UPISND_PIN_B04,
	UPISND_PIN_B05, UPISND_PIN_B06, UPISND_PIN_B07, UPISND_PIN_B08,
	UPISND_PIN_B09, UPISND_PIN_B10, UPISND_PIN_B11, UPISND_PIN_B12,
	UPISND_PIN_B13, UPISND_PIN_B14, UPISND_PIN_B15, UPISND_PIN_B16,
	UPISND_PIN_B17, UPISND_PIN_B18, UPISND_PIN_B23, UPISND_PIN_B24,
	UPISND_PIN_B25, UPISND_PIN_B26, UPISND_PIN_B27, UPISND_PIN_B28,
	UPISND_PIN_B29, UPISND_PIN_B30, UPISND_PIN_B31, UPISND_PIN_B32,
	UPISND_PIN_B33, UPISND_PIN_B34, UPISND_PIN_B37, UPISND_PIN_B38,
	UPISND_PIN_B39,

	// Must be the last one!
	UPISND_PIN_COUNT,

	/// Value for indicating an invalid pin.
	UPISND_PIN_INVALID = UPISND_PIN_COUNT
} upisnd_pin_e;

/// @brief The storage type to refer to Pisound Micro's GPIO pins. @see #upisnd_pin_e.
typedef int8_t upisnd_pin_t;

/// @brief Value mode.
typedef enum
{
	UPISND_VALUE_MODE_INVALID = -1,

	/// @brief The value is clamped to input_min and input_max range.
	UPISND_VALUE_MODE_CLAMP,

	/// @brief The value is wrapped over to the other boundary of the input range.
	UPISND_VALUE_MODE_WRAP,

	// Must be the last one!
	UPISND_VALUE_MODE_COUNT
} upisnd_value_mode_e;

/// @brief Number range for input_min, input_max and value_low, value_high.
typedef struct
{
	int low;
	int high;
} upisnd_range_t;

/// @brief Encoder specific options.
typedef struct
{
	upisnd_range_t      input_range;
	upisnd_range_t      value_range;
	upisnd_value_mode_e value_mode;
} upisnd_encoder_opts_t;

/// @brief Analog Input specific options.
typedef struct
{
	upisnd_range_t      input_range;
	upisnd_range_t      value_range;
} upisnd_analog_input_opts_t;

#endif // PISOUND_MICRO_TYPES_H
