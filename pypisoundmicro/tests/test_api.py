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

import unittest
import os
import random
import signal
import string
import sys
import time
import pypisoundmicro as psm
from pypisoundmicro import (Pin, ElementType, PinPull, PinDirection, ActivityType,
						   ValueMode, ElementName, Element, Setup, ValueFd, Range)
from pypisoundmicro.swig import pypisoundmicro as swig_psm

# Signal handler for clean shutdown during abnormal termination
def signal_handler(signum, frame):
	print(f"\nReceived signal {signum}. Cleaning up resources...")
	psm.cleanup()
	print("Cleanup complete. Exiting.")
	sys.exit(1)

# Register the handler for common termination signals
signal.signal(signal.SIGINT, signal_handler)   # Ctrl+C
signal.signal(signal.SIGTERM, signal_handler)  # Termination signal

def random_string(length=8):
	"""Generate a random string for test element names."""
	return ''.join(random.choice(string.ascii_lowercase) for i in range(length))

class TestLibInitialization(unittest.TestCase):
	"""Test the initialization and uninitialization of the library."""
	
	def test_init_uninit(self):
		# Test that we can initialize and uninitialize the library
		# This implicitly tests that the base functions are working
		result = swig_psm.upisnd_init()
		self.assertEqual(result, 0, "Failed to initialize libpisoundmicro")
		swig_psm.upisnd_uninit()


class TestElementName(unittest.TestCase):
	"""Test the ElementName class."""
	
	def test_regular_name(self):
		name = ElementName.regular("test_name")
		self.assertIsNotNone(name)
		self.assertEqual(str(name), "test_name")
	
	def test_randomized_name(self):
		name = ElementName.randomized()
		self.assertIsNotNone(name)
		self.assertTrue(len(str(name)) > 0)
	
	def test_prefixed_randomized_name(self):
		prefix = "test_"
		name = ElementName.randomized(prefix)
		self.assertIsNotNone(name)
		self.assertTrue(str(name).startswith(prefix))


class TestElement(unittest.TestCase):
	"""Test the Element base class."""
	
	def setUp(self):
		swig_psm.upisnd_init()
	
	def tearDown(self):
		swig_psm.upisnd_uninit()
	
	def test_element_constructor(self):
		element = Element()
		self.assertFalse(element.is_valid)
	
	def test_element_setup_gpio_input(self):
		name = f"test_{random_string()}"
		setup = Setup.for_gpio_input(Pin.B03, PinPull.UP)
		element = Element.setup(name, setup)
		
		self.assertTrue(element.is_valid)
		self.assertEqual(element.name, name)
		self.assertEqual(element.type, ElementType.GPIO)
		self.assertEqual(element.pin, Pin.B03)
		
		element.release()
	
	def test_element_context_manager(self):
		name = f"test_{random_string()}"
		setup = Setup.for_gpio_output(Pin.B05, False)
		
		with Element.setup(name, setup) as element:
			self.assertTrue(element.is_valid)
			self.assertEqual(element.name, name)
		
		# After the context manager exits, the element should be released
		element = Element.get(name)
		self.assertIsNone(element)


class TestValueFd(unittest.TestCase):
	"""Test the ValueFd class."""
	
	def setUp(self):
		swig_psm.upisnd_init()
		
		# Set up a GPIO output element for testing
		self.element_name = f"test_valuefd_{random_string()}"
		setup = Setup.for_gpio_output(Pin.B03, False)
		self.element = Element.setup(self.element_name, setup)
		
	def tearDown(self):
		if hasattr(self, 'element') and self.element and self.element.is_valid:
			self.element.release()
		swig_psm.upisnd_uninit()
	
	def test_valuefd_read_write(self):
		# Open a ValueFd for the element
		fd = self.element.open_value_fd(os.O_RDWR | os.O_CLOEXEC)
		self.assertIsNotNone(fd)
		self.assertTrue(fd.is_valid())
		
		# Write a value
		result = fd.write(1)
		self.assertGreater(result, 0)
		
		# Read the value back
		value = fd.read()
		self.assertEqual(value, 1)
		
		# Write another value
		result = fd.write(0)
		self.assertGreater(result, 0)
		
		# Read again
		value = fd.read()
		self.assertEqual(value, 0)
		
		# Explicitly close the fd
		fd.close()
		self.assertFalse(fd.is_valid())
	
	def test_valuefd_get_take(self):
		fd = self.element.open_value_fd(os.O_RDWR | os.O_CLOEXEC)
		self.assertTrue(fd.is_valid())
		
		# Get the raw fd value
		raw_fd = fd.get()
		self.assertGreaterEqual(raw_fd, 0)
		
		# Take ownership of the fd
		raw_fd = fd.take()
		self.assertGreaterEqual(raw_fd, 0)
		self.assertFalse(fd.is_valid())
		
		# Close the raw fd ourselves
		os.close(raw_fd)
	
	def test_valuefd_destruction(self):
		fd = self.element.open_value_fd(os.O_RDWR | os.O_CLOEXEC)
		self.assertTrue(fd.is_valid())
		
		# Get the raw fd value
		raw_fd = fd.get()
		
		# Delete the ValueFd object, which should close the fd
		del fd
		
		# Try to use the fd, which should fail if it was closed
		try:
			os.write(raw_fd, b"test")
			self.fail("File descriptor should be closed")
		except OSError:
			pass  # Expected behavior


class TestGPIO(unittest.TestCase):
	"""Test GPIO functionality."""
	
	def setUp(self):
		swig_psm.upisnd_init()
	
	def tearDown(self):
		swig_psm.upisnd_uninit()
	
	def test_gpio_input(self):
		name = f"test_gpio_input_{random_string()}"
		gpio = psm.Gpio.setup_input(name, Pin.B03, PinPull.UP)
		
		self.assertTrue(gpio.is_valid)
		self.assertEqual(gpio.name, name)
		self.assertEqual(gpio.direction, PinDirection.INPUT)  # Using property
		self.assertEqual(gpio.pull, PinPull.UP)  # Using property
		
		# Read the value (could be 0 or 1 depending on hardware state)
		value = gpio.get_value()
		self.assertIn(value, [0, 1])
		
		gpio.release()
	
	def test_gpio_output(self):
		name = f"test_gpio_output_{random_string()}"
		gpio = psm.Gpio.setup_output(name, Pin.B05, False)
		
		self.assertTrue(gpio.is_valid)
		self.assertEqual(gpio.name, name)
		self.assertEqual(gpio.direction, PinDirection.OUTPUT)  # Using property
		
		# Set to high and check
		gpio.set_value(True)
		self.assertEqual(gpio.get_value(), 1)
		
		# Set to low and check
		gpio.set_value(False)
		self.assertEqual(gpio.get_value(), 0)
		
		gpio.release()
	
	def test_as_gpio(self):
		name = f"test_as_gpio_{random_string()}"
		element = Element.setup(name, Setup.for_gpio_output(Pin.B07, False))
		
		# Convert generic Element to Gpio
		gpio = element.as_gpio()
		self.assertIsNotNone(gpio)
		self.assertTrue(gpio.is_valid)
		
		# Test functionality of the cast object
		gpio.set_value(True)
		self.assertEqual(gpio.get_value(), 1)
		
		gpio.release()


class TestAnalogInput(unittest.TestCase):
	"""Test AnalogInput functionality."""
	
	def setUp(self):
		swig_psm.upisnd_init()
	
	def tearDown(self):
		swig_psm.upisnd_uninit()
	
	def test_analog_input_setup(self):
		name = f"test_analog_{random_string()}"
		adc = psm.AnalogInput.setup(name, Pin.B23)
		
		self.assertTrue(adc.is_valid)
		self.assertEqual(adc.name, name)
		self.assertEqual(adc.pin, Pin.B23)
		
		# Read the value (will depend on the physical connection)
		value = adc.get_value()
		
		# ADC values should be in the range 0-1023 (10-bit)
		self.assertGreaterEqual(value, 0)
		self.assertLessEqual(value, 1023)
		
		adc.release()
	
	def test_analog_input_options(self):
		name = f"test_analog_opts_{random_string()}"
		adc = psm.AnalogInput.setup(name, Pin.B24)
		
		# Set custom range mapping
		opts = adc.get_opts()
		opts.input_range = Range(0, 1023)
		opts.value_range = Range(0, 100)
		adc.set_opts(opts)
		
		# Read new options to verify they were set
		new_opts = adc.get_opts()
		self.assertEqual(new_opts.input_range.low, 0)
		self.assertEqual(new_opts.input_range.high, 1023)
		self.assertEqual(new_opts.value_range.low, 0)
		self.assertEqual(new_opts.value_range.high, 100)
		
		# The value should now be in the range 0-100
		value = adc.get_value()
		self.assertGreaterEqual(value, 0)
		self.assertLessEqual(value, 100)
		
		adc.release()
	
	def test_as_analog_input(self):
		name = f"test_as_analog_{random_string()}"
		element = Element.setup(name, Setup.for_analog_input(Pin.B25))
		
		# Convert generic Element to AnalogInput
		adc = element.as_analog_input()
		self.assertIsNotNone(adc)
		self.assertTrue(adc.is_valid)
		
		# Test functionality of the cast object
		value = adc.get_value()
		self.assertGreaterEqual(value, 0)
		
		adc.release()


class TestEncoder(unittest.TestCase):
	"""Test Encoder functionality."""
	
	def setUp(self):
		swig_psm.upisnd_init()
	
	def tearDown(self):
		swig_psm.upisnd_uninit()
	
	def test_encoder_setup(self):
		name = f"test_encoder_{random_string()}"
		encoder = psm.Encoder.setup(name, Pin.B03, PinPull.UP, Pin.B04, PinPull.UP)
		
		self.assertTrue(encoder.is_valid)
		self.assertEqual(encoder.name, name)
		self.assertEqual(encoder.pin, Pin.B03)
		
		# Test getting encoder options
		opts = encoder.get_opts()
		self.assertEqual(opts.value_mode, ValueMode.CLAMP)
		
		encoder.release()
	
	def test_encoder_options(self):
		name = f"test_enc_opts_{random_string()}"
		encoder = psm.Encoder.setup(name, Pin.B05, PinPull.UP, Pin.B06, PinPull.UP)
		
		# Set custom encoder options
		opts = encoder.get_opts()
		opts.input_range = Range(-10, 10)
		opts.value_range = Range(0, 100)
		opts.value_mode = ValueMode.WRAP
		encoder.set_opts(opts)
		
		# Read back the options
		new_opts = encoder.get_opts()
		self.assertEqual(new_opts.input_range.low, -10)
		self.assertEqual(new_opts.input_range.high, 10)
		self.assertEqual(new_opts.value_range.low, 0)
		self.assertEqual(new_opts.value_range.high, 100)
		self.assertEqual(new_opts.value_mode, ValueMode.WRAP)
		
		encoder.release()
	
	def test_as_encoder(self):
		name = f"test_as_encoder_{random_string()}"
		setup = Setup.for_encoder(Pin.B07, PinPull.UP, Pin.B08, PinPull.UP)
		element = Element.setup(name, setup)
		
		# Convert generic Element to Encoder
		encoder = element.as_encoder()
		self.assertIsNotNone(encoder)
		self.assertTrue(encoder.is_valid)
		
		encoder.release()


class TestActivity(unittest.TestCase):
	"""Test Activity functionality."""
	
	def setUp(self):
		swig_psm.upisnd_init()
	
	def tearDown(self):
		swig_psm.upisnd_uninit()
	
	def test_activity_setup(self):
		name = f"test_activity_{random_string()}"
		activity = psm.Activity.setup_activity(name, Pin.B09, ActivityType.MIDI_INPUT)
		
		self.assertTrue(activity.is_valid)
		self.assertEqual(activity.name, name)
		self.assertEqual(activity.pin, Pin.B09)
		self.assertEqual(activity.activity_type, ActivityType.MIDI_INPUT)
		
		activity.release()
	
	def test_as_activity(self):
		name = f"test_as_activity_{random_string()}"
		setup = Setup.for_activity(Pin.B10, ActivityType.MIDI_OUTPUT)
		element = Element.setup(name, setup)
		
		# Convert generic Element to Activity
		activity = element.as_activity()
		self.assertIsNotNone(activity)
		self.assertTrue(activity.is_valid)
		self.assertEqual(activity.activity_type, ActivityType.MIDI_OUTPUT)
		
		activity.release()


class TestSetup(unittest.TestCase):
	"""Test Setup functionality."""
	
	def test_setup_for_gpio_input(self):
		setup = Setup.for_gpio_input(Pin.B03, PinPull.UP)
		self.assertIsNotNone(setup)
		
		element_type = swig_psm.upisnd_setup_get_element_type(setup.to_int())
		self.assertEqual(element_type, ElementType.GPIO)
		
		pin = swig_psm.upisnd_setup_get_pin_id(setup.to_int())
		self.assertEqual(pin, Pin.B03)
		
		direction = swig_psm.upisnd_setup_get_gpio_dir(setup.to_int())
		self.assertEqual(direction, PinDirection.INPUT)
		
		pull = swig_psm.upisnd_setup_get_gpio_pull(setup.to_int())
		self.assertEqual(pull, PinPull.UP)
	
	def test_setup_for_gpio_output(self):
		setup = Setup.for_gpio_output(Pin.B05, True)
		self.assertIsNotNone(setup)
		
		element_type = swig_psm.upisnd_setup_get_element_type(setup.to_int())
		self.assertEqual(element_type, ElementType.GPIO)
		
		pin = swig_psm.upisnd_setup_get_pin_id(setup.to_int())
		self.assertEqual(pin, Pin.B05)
		
		direction = swig_psm.upisnd_setup_get_gpio_dir(setup.to_int())
		self.assertEqual(direction, PinDirection.OUTPUT)
		
		output = swig_psm.upisnd_setup_get_gpio_output(setup.to_int())
		self.assertEqual(output, 1)  # 1 for True
	
	def test_setup_for_analog_input(self):
		setup = Setup.for_analog_input(Pin.B23)
		self.assertIsNotNone(setup)
		
		element_type = swig_psm.upisnd_setup_get_element_type(setup.to_int())
		self.assertEqual(element_type, ElementType.ANALOG_INPUT)
		
		pin = swig_psm.upisnd_setup_get_pin_id(setup.to_int())
		self.assertEqual(pin, Pin.B23)
	
	def test_setup_for_encoder(self):
		setup = Setup.for_encoder(Pin.B03, PinPull.UP, Pin.B04, PinPull.UP)
		self.assertIsNotNone(setup)
		
		element_type = swig_psm.upisnd_setup_get_element_type(setup.to_int())
		self.assertEqual(element_type, ElementType.ENCODER)
		
		pin_a = swig_psm.upisnd_setup_get_pin_id(setup.to_int())
		self.assertEqual(pin_a, Pin.B03)
		
		pull_a = swig_psm.upisnd_setup_get_gpio_pull(setup.to_int())
		self.assertEqual(pull_a, PinPull.UP)
		
		pin_b = swig_psm.upisnd_setup_get_encoder_pin_b_id(setup.to_int())
		self.assertEqual(pin_b, Pin.B04)
		
		pull_b = swig_psm.upisnd_setup_get_encoder_pin_b_pull(setup.to_int())
		self.assertEqual(pull_b, PinPull.UP)
	
	def test_setup_for_activity(self):
		setup = Setup.for_activity(Pin.B09, ActivityType.MIDI_INPUT)
		self.assertIsNotNone(setup)
		
		element_type = swig_psm.upisnd_setup_get_element_type(setup.to_int())
		self.assertEqual(element_type, ElementType.ACTIVITY)
		
		pin = swig_psm.upisnd_setup_get_pin_id(setup.to_int())
		self.assertEqual(pin, Pin.B09)
		
		activity_type = swig_psm.upisnd_setup_get_activity_type(setup.to_int())
		self.assertEqual(activity_type, ActivityType.MIDI_INPUT)


if __name__ == '__main__':
	try:
		unittest.main()
	except KeyboardInterrupt:
		# This is a fallback in case the signal handler doesn't catch it
		print("\nKeyboardInterrupt caught. Cleaning up...")
		psm.cleanup()
		sys.exit(1)
	except SystemExit as e:
		# Normal unittest exit, ensure cleanup is done
		psm.cleanup()
		sys.exit(e.code)
	except Exception as e:
		# Any other exception
		print(f"\nUnexpected exception: {e}")
		psm.cleanup()
		sys.exit(1)
	finally:
		# One final attempt to clean up, though this might not be reached
		# if a signal terminates the process
		psm.cleanup()
