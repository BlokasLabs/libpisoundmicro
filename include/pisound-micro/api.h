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

#ifndef PISOUND_MICRO_API_H
#define PISOUND_MICRO_API_H

/** @defgroup c C API
 *
 * The C API reference documentation for libpisoundmicro.
 *
 * # Quick Start
 *
 * First, install the development package:
 *
 * ```bash
 * sudo apt install libpisoundmicro-dev
 * ```
 *
 * Then, let's create a simple program to read the GPIO value of pin B03:
 *
 * ```c
#include <pisound-micro.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	// Initialize the libpisoundmicro library.
	int err = upisnd_init();
	if (err != 0)
	{
		fprintf(stderr, "Failed to initialize libpisoundmicro: %d\n", err);
		return 1;
	}

	// Generate a random name for the Element. (optional, can use a fixed name instead)
	char random_name[UPISND_MAX_ELEMENT_NAME_LENGTH];
	if (upisnd_generate_random_element_name(random_name,
											sizeof(random_name),
											NULL) >= sizeof(random_name))
	{
		fprintf(stderr, "Failed to generate a random name.\n");
		return 1;
	}

	// Set up the GPIO Input Element on pin B03, with pull-up enabled.
	upisnd_element_ref_t gpio = upisnd_setup_gpio_input(random_name,
														UPISND_PIN_B03,
														UPISND_PIN_PULL_UP);

	if (!gpio)
	{
		fprintf(stderr, "Failed to setup GPIO.\n");
		return 1;
	}

	// Open the GPIO value file descriptor. (`/sys/pisound-micro/elements/<name>/value`)
	int fd = upisnd_element_open_value_fd(gpio, O_RDONLY | O_CLOEXEC);
	if (fd < 0)
	{
		fprintf(stderr, "Failed to open GPIO value file.\n");
		return 1;
	}

	// Read the GPIO value.
	int value = upisnd_value_read(fd);
	if (errno != 0)
	{
		fprintf(stderr, "Failed to read GPIO value: %d\n", errno);
		return 1;
	}

	// Close the GPIO value file descriptor.
	close(fd);

	fprintf(stderr, "B03 is %s.\n", value ? "high" : "low");

	// Unref the Element, this will unsetup it as well, as the last
	// reference to it is released.
	upisnd_element_unref(&gpio);

	// Uninitialize the libpisoundmicro library. If any references to
	// Elements still remained, they would get released too.
	upsind_uninit();

	return 0;
}
```
 * Save the code as `example.c` and compile it:
 * ```bash
 * gcc example.c -lpisoundmicro -o example
 * ```
 *
 * And run it:
 *
 * ```bash
 * ./example
 * ```
 *
 * You should see it output either \"\a B03 \a is \a high.\" or \"\a B03 \a is \a low.\", depending on the B03 pin state.
 *
 * See the below documentation for more details.
 *
 * @{
 */

/**
 * @file api.h
 * @brief This header defines the C APIs of libpisoundmicro.
 */

#ifndef PISOUND_MICRO_H
#	error Please #include <pisound-micro.h>!
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** @brief libpisoundmicro initialization function. Must be called once before using any other API.
 *
 * It is reference counted, so you may call it multiple times, you must call ::upisnd_uninit
 * a matching number of times.
 *
 * Not thread-safe.
 *
 * @return 0 on success, -1 on error, inspect `errno` for details.
 */
UPISND_API int upisnd_init(void);

/** @brief libpisoundmicro uninitialization function.
 *
 * Takes care of releasing any remaining Elements.
 *
 * Not thread-safe.
 *
 * Must be called upon process cleanup, including when handling signals,
 * the same number of times as ::upisnd_init was called.
 */
UPISND_API void upisnd_uninit(void);

/// Checks validity of the provided pin number.
UPISND_API bool upisnd_is_pin_valid(upisnd_pin_t pin);

/// Converts the provided pin to its string representation.
UPISND_API const char *upisnd_pin_to_str(upisnd_pin_t pin);

/** @brief Parses the provided string and returns the corresponding pin number.
 *
 * @return The pin number, or #UPISND_PIN_INVALID if the string is not a valid pin number.
 */
UPISND_API upisnd_pin_t upisnd_str_to_pin(const char *str);

/// Converts the provided pin pull to its string representation.
UPISND_API const char *upisnd_pin_pull_to_str(upisnd_pin_pull_e pull);
/// Parses the provided string and returns the corresponding pin pull.
UPISND_API upisnd_pin_pull_e upisnd_str_to_pin_pull(const char *str);

/// Converts the provided activity type to its string representation.
UPISND_API const char *upisnd_activity_to_str(upisnd_activity_e activity);
/// Parses the provided string and returns the corresponding activity type.
UPISND_API upisnd_activity_e upisnd_str_to_activity(const char *str);

/// Converts the provided element type to its string representation.
UPISND_API const char *upisnd_element_type_to_str(upisnd_element_type_e type);
/// Parses the provided string and returns the corresponding element type.
UPISND_API upisnd_element_type_e upisnd_str_to_element_type(const char *str);

/// Converts the provided pin direction to its string representation.
UPISND_API const char *upisnd_pin_direction_to_str(upisnd_pin_direction_e dir);
/// Parses the provided string and returns the corresponding pin direction.
UPISND_API upisnd_pin_direction_e upisnd_str_to_pin_direction(const char *str);

/// Converts the provided value mode to its string representation.
UPISND_API const char *upisnd_value_mode_to_str(upisnd_value_mode_e mode);
/// Parses the provided string and returns the corresponding value mode.
UPISND_API upisnd_value_mode_e upisnd_str_to_value_mode(const char *str);

/// Extracts the Element type from the setup container.
UPISND_API upisnd_element_type_e  upisnd_setup_get_element_type      (upisnd_setup_t setup);
/// Extracts the main pin from the setup container.
UPISND_API upisnd_pin_t           upisnd_setup_get_pin_id            (upisnd_setup_t setup);
/// Extracts the GPIO pull from the setup container, applies to GPIO Input and the first pin of Encoder.
UPISND_API upisnd_pin_pull_e      upisnd_setup_get_gpio_pull         (upisnd_setup_t setup);
/// Extracts the GPIO direction from the setup container, applies only to GPIO Input or Output.
UPISND_API upisnd_pin_direction_e upisnd_setup_get_gpio_dir          (upisnd_setup_t setup);
/// Extracts the GPIO output level from the setup container, applies only to GPIO Output.
UPISND_API int                    upisnd_setup_get_gpio_output       (upisnd_setup_t setup);
/// Extracts the Encoder's second pin from the setup container, applies only to Encoders.
UPISND_API upisnd_pin_t           upisnd_setup_get_encoder_pin_b_id  (upisnd_setup_t setup);
/// Extracts the Encoder's second pin pull from the setup container, applies only to Encoders.
UPISND_API upisnd_pin_pull_e      upisnd_setup_get_encoder_pin_b_pull(upisnd_setup_t setup);
/// Extracts the Activity type from the setup container, applies only to Activity Elements.
UPISND_API upisnd_activity_e      upisnd_setup_get_activity_type     (upisnd_setup_t setup);

/** @brief Sets the Element type in the setup container.
 *
 * Always set the Element Type before setting any other property of setup, as
 * the type is double-checked to know if the set operation is valid for this type.
 *
 * @return 0 on success, `-errno` on error.
 */
UPISND_API int upisnd_setup_set_element_type(upisnd_setup_t *setup, upisnd_element_type_e value);

/** @brief Sets the main pin id in the setup container.
 *
 * @return 0 on success, `-errno` on error.
 */
UPISND_API int upisnd_setup_set_pin_id(upisnd_setup_t *setup, upisnd_pin_t value);

/** @brief Sets the GPIO direction in the setup container, applies only to GPIO Elements.
 *
 * Always set the GPIO dir before setting the pull and output, as the direction is double-checked
 * to know whether pull (only input) or output level (only output) properties are valid.
 *
 * @return 0 on success, `-errno` on error.
 */
UPISND_API int upisnd_setup_set_gpio_dir(upisnd_setup_t *setup, upisnd_pin_direction_e value);

/** @brief Sets the GPIO pull in the setup container, applies only to GPIO Input.
 *
 * Make sure to first set the GPIO dir to #UPISND_PIN_DIR_INPUT.
 *
 * @return 0 on success, `-errno` on error.
 */
UPISND_API int upisnd_setup_set_gpio_pull(upisnd_setup_t *setup, upisnd_pin_pull_e value);

/** @brief Sets the GPIO output level in the setup container, applies only to GPIO Output.
 *
 * Make sure to first set the GPIO dir to #UPISND_PIN_DIR_OUTPUT.
 *
 * @return 0 on success, `-errno` on error.
 */
UPISND_API int upisnd_setup_set_gpio_output(upisnd_setup_t *setup, bool value);

/** @brief Sets the Encoder's second pin id in the setup container, applies only to Encoders.
 *
 * @return 0 on success, `-errno on error.
 */
UPISND_API int upisnd_setup_set_encoder_pin_b_id(upisnd_setup_t *setup, upisnd_pin_t value);

/** @brief Sets the Encoder's second pin pull in the setup container, applies only to Encoders.
 *
 * @return 0 on success, `-errno` on error.
 */
UPISND_API int upisnd_setup_set_encoder_pin_b_pull(upisnd_setup_t *setup, upisnd_pin_pull_e value);

/** @brief Sets the Activity type in the setup container, applies only to Activity Elements.
 *
 * @return 0 on success, `-errno` on error.
 */
UPISND_API int upisnd_setup_set_activity_type(upisnd_setup_t *setup, upisnd_activity_e value);

/** @brief Verifies that the provided element name is valid.
 *
 * @return The length of \a name on success, `-errno` on error.
 */
UPISND_API int upisnd_validate_element_name(const char *name);

/** @brief A helper for generating a random element name with the provided prefix.
 *
 * @param dst The buffer to write the generated name to.
 * @param n The size of the buffer, at most #UPISND_MAX_ELEMENT_NAME_LENGTH.
 * @param prefix The prefix to use for the generated name, can be NULL if not needed. The prefix can be up to `UPISND_MAX_ELEMENT_NAME_LENGTH - 23` long.
 *
 * It is not strictly necessary to use this function, you may use any fixed valid name for your Element.
 *
 * @return The return value is directly forwarded from the `snprintf` call. A negative value indicates an error,
 * a value >= \a n means the \a dst buffer was not long enough, and a value < \a n indicates success. Check out
 * `snprintf` documentation for more details.
 */
UPISND_API int upisnd_generate_random_element_name(char *dst, size_t n, const char *prefix);

/** @brief Force-unsetup an Element by name. This is normally not necessary.
 *
 * Unrefing the element will automatically unsetup it once last reference is released.
 * This function is only useful if recovering from crash, avoid if possible.
 *
 * @return 0 on success, -1 on error, inspect `errno` for details.
 */
UPISND_API int upisnd_unsetup(const char *name);


/** @brief Get a reference to an Element by name that was set up during the current runtime session.
 *
 * If the Element exists in `/sys/pisound-micro/elements/`, but it was not created
 * by the current program, it won't get returned. In this case, you can try setting up
 * an Element with the exact same setup options, if they match, you'll get a reference,
 * otherwise, use ::upisnd_unsetup to remove the Element first, and set it up fresh.
 *
 * @return A valid Element reference on success, NULL on error.
 */
UPISND_API upisnd_element_ref_t upisnd_element_get(const char *name);

/** Increment the reference count of the Element.
 *
 * @return The same reference that was passed in.
 */
UPISND_API upisnd_element_ref_t upisnd_element_add_ref(upisnd_element_ref_t ref);

/** Decrement the reference count of the Element.
 *
 * @param ref The pointer to reference to unref, will be automatically NULL-ed out.
 *
 * If the reference count reaches 0, the Element will be unsetup and released.
 */
UPISND_API void upisnd_element_unref(upisnd_element_ref_t *ref);

/** @brief Get the name of the Element.
 *
 * The result is valid, as long as the element reference is valid.
 *
 * @return The name of the Element, or NULL if the reference is invalid.
 */
UPISND_API const char *upisnd_element_get_name(upisnd_element_ref_t el);

/** @brief Get the type of the Element.
 *
 * @return The type of the Element. If #UPISND_ELEMENT_TYPE_INVALID, check the `errno`.
 */
UPISND_API upisnd_element_type_e upisnd_element_get_type(upisnd_element_ref_t el);

/** @brief Get the pin number of the Element.
 *
 * @return The pin number of the Element. If #UPISND_PIN_INVALID, check the `errno`.
 */
UPISND_API upisnd_pin_t upisnd_element_get_pin(upisnd_element_ref_t el);

/** @brief Opens the Element's value file descriptor.
 *
 * @param el The Element reference.
 * @param flags The flags to pass to the `open` system call.
 *
 * You must set the access flags, such as O_RDONLY, O_WRONLY or O_RDWR,
 * defined in the system's `fcntl.h` header. We recommend setting O_CLOEXEC as well.
 * (combine the flags using logic OR `|`).
 *
 * Use ::upisnd_value_read and ::upisnd_value_write to read and write the value.
 *
 * You may also `poll` for changes (use `POLLPRI`). Look into the documentation for
 * `poll`, `ppoll` and `select` system APIs for more details.
 *
 * Once you're done with the fd, close it using the `close` system call.
 *
 * @return The file descriptor on success, -1 on error, inspect `errno` for details.
 */
UPISND_API int upisnd_element_open_value_fd(upisnd_element_ref_t el, int flags);

/** @brief Reads the Element's value.
 *
 * @param fd The file descriptor of the Element's value file.
 *
 * @return The value read from the Element's value file. On successful read, `errno` will be 0.
 */
UPISND_API int upisnd_value_read(int fd);

/** @brief Writes the Element's value.
 *
 * @param fd The file descriptor of the Element's value file.
 * @param value The value to write to the Element's value file.
 *
 * @return Number of characters written on success, -1 on error, inspect `errno` for details.
 */
UPISND_API int upisnd_value_write(int fd, int value);

// Setup functions create and setup an element, using the arguments provided for its configuration.
// In case an element already exists with the same name, and the requested configuration is the same,
// the existing element will get returned and its refcount will get incremented. Otherwise, NULL
// will be returned, and `errno` set appropriately.
//

/** @brief Set up an Element with the provided name and setup options container.
 *
 * @param name The name of the Element to set up.
 * @param setup The setup options container for the Element.
 *
 * In case an Element already exists with the same name, and the requested configuration is the same,
 * the existing Element will get returned and its refcount will get incremented. Otherwise, NULL
 * will be returned, and `errno` set appropriately.
 *
 * @return A valid Element reference on success, NULL on error, inspect `errno` for details.
 */
UPISND_API upisnd_element_ref_t upisnd_setup(const char *name, upisnd_setup_t setup);

/// Set up an Encoder Element with the provided name and setup options.
UPISND_API upisnd_element_ref_t upisnd_setup_encoder(const char *name, upisnd_pin_t pin_a, upisnd_pin_pull_e pull_a, upisnd_pin_t pin_b, upisnd_pin_pull_e pull_b);
/// Set up an Analog Input Element with the provided name and setup options.
UPISND_API upisnd_element_ref_t upisnd_setup_analog_input(const char *name, upisnd_pin_t pin);
/// Set up a GPIO Input Element with the provided name and setup options.
UPISND_API upisnd_element_ref_t upisnd_setup_gpio_input(const char *name, upisnd_pin_t pin, upisnd_pin_pull_e pull);
/// Set up a GPIO Output Element with the provided name and setup options.
UPISND_API upisnd_element_ref_t upisnd_setup_gpio_output(const char *name, upisnd_pin_t pin, bool high);
/// Set up an Activity Element with the provided name and setup options.
UPISND_API upisnd_element_ref_t upisnd_setup_activity(const char *name, upisnd_pin_t pin, upisnd_activity_e activity);

/// Get the GPIO Element's direction.
UPISND_API upisnd_pin_direction_e upisnd_element_gpio_get_direction(upisnd_element_ref_t el);
/// Get the GPIO or Encoder Element's input pull.
UPISND_API upisnd_pin_pull_e upisnd_element_gpio_get_pull(upisnd_element_ref_t el);

/// Get the Activity Element's activity type.
UPISND_API upisnd_activity_e upisnd_element_activity_get_type(upisnd_element_ref_t el);

/// Initialize the values of Encoder's options struct to the defaults.
UPISND_API void upisnd_element_encoder_init_default_opts(upisnd_encoder_opts_t *opts);
/// Retrieve the Encoder's options.
UPISND_API int  upisnd_element_encoder_get_opts(upisnd_element_ref_t el, upisnd_encoder_opts_t *opts);	
/// Set the Encoder's options.
UPISND_API int  upisnd_element_encoder_set_opts(upisnd_element_ref_t el, const upisnd_encoder_opts_t *opts);
///  Get the Encoder's second pin.
UPISND_API upisnd_pin_t upisnd_element_encoder_get_pin_b(upisnd_element_ref_t el);
///  Get the Encoder's second pin pull. Use ::upisnd_element_gpio_get_pull to get pull of first pin.
UPISND_API upisnd_pin_pull_e upisnd_element_encoder_get_pin_b_pull(upisnd_element_ref_t el);

/// Initialize the values of Analog Input's options struct to the defaults.
UPISND_API void upisnd_element_analog_input_init_default_opts(upisnd_analog_input_opts_t *opts);
/// Retrieve the Analog Input's options.
UPISND_API int  upisnd_element_analog_input_get_opts(upisnd_element_ref_t el, upisnd_analog_input_opts_t *opts);
/// Set the Analog Input's options.
UPISND_API int  upisnd_element_analog_input_set_opts(upisnd_element_ref_t el, const upisnd_analog_input_opts_t *opts);

#ifdef __cplusplus
} // extern "C"
#endif

/** @} */

#endif // PISOUND_MICRO_API_H
