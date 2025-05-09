"""Setup utilities for Element configuration in pypisoundmicro."""

from ._utils import copy_doc
from .swig import pypisoundmicro as psm
from typing import Optional, Union, Self
from .types import ElementType, Pin, PinDirection, PinPull, ActivityType
from .name import ElementName
from . import Element

class Setup:
	"""Wrapper for upisnd_setup_t type that encapsulates Element configuration.
	
	This class provides a Pythonic interface for working with element setup options
	that are passed to Element.setup() to create new elements.
	"""

	def __init__(self) -> None:
		"""Initialize an empty Setup object."""
		self._setup = 0  # The underlying C upisnd_setup_t is just a 32-bit integer

	@property
	@copy_doc(psm.upisnd_setup_get_element_type)
	def element_type(self) -> ElementType:
		return ElementType(psm.upisnd_setup_get_element_type(self._setup))

	@element_type.setter
	@copy_doc(psm.upisnd_setup_set_element_type)
	def element_type(self, value: ElementType) -> None:
		result, setup = psm.upisnd_setup_set_element_type(self._setup, value)
		if result != 0:
			raise ValueError(f"Failed to set element type: {result}")
		self._setup = setup

	@property
	@copy_doc(psm.upisnd_setup_get_pin_id)
	def pin(self) -> Pin:
		return Pin(psm.upisnd_setup_get_pin_id(self._setup))

	@pin.setter
	@copy_doc(psm.upisnd_setup_set_pin_id)
	def pin(self, value: Pin) -> None:
		result, setup = psm.upisnd_setup_set_pin_id(self._setup, value)
		if result != 0:
			raise ValueError(f"Failed to set pin: {result}")
		self._setup = setup

	@property
	@copy_doc(psm.upisnd_setup_get_gpio_dir)
	def gpio_direction(self) -> PinDirection:
		return PinDirection(psm.upisnd_setup_get_gpio_dir(self._setup))

	@gpio_direction.setter
	@copy_doc(psm.upisnd_setup_set_gpio_dir)
	def gpio_direction(self, value: PinDirection) -> None:
		result, setup = psm.upisnd_setup_set_gpio_dir(self._setup, value)
		if result != 0:
			raise ValueError(f"Failed to set GPIO direction: {result}")
		self._setup = setup

	@property
	@copy_doc(psm.upisnd_setup_get_gpio_pull)
	def gpio_pull(self) -> PinPull:
		return PinPull(psm.upisnd_setup_get_gpio_pull(self._setup))

	@gpio_pull.setter
	@copy_doc(psm.upisnd_setup_set_gpio_pull)
	def gpio_pull(self, value: PinPull) -> None:
		result, setup = psm.upisnd_setup_set_gpio_pull(self._setup, value)
		if result != 0:
			raise ValueError(f"Failed to set GPIO pull: {result}")
		self._setup = setup

	@property
	@copy_doc(psm.upisnd_setup_get_gpio_output)
	def gpio_output(self) -> int:
		return psm.upisnd_setup_get_gpio_output(self._setup)

	@gpio_output.setter
	@copy_doc(psm.upisnd_setup_set_gpio_output)
	def gpio_output(self, value: int) -> None:
		result, setup = psm.upisnd_setup_set_gpio_output(self._setup, value)
		if result != 0:
			raise ValueError(f"Failed to set GPIO output: {result}")
		self._setup = setup

	@property
	@copy_doc(psm.upisnd_setup_get_encoder_pin_b_id)
	def encoder_pin_b(self) -> Pin:
		return Pin(psm.upisnd_setup_get_encoder_pin_b_id(self._setup))

	@encoder_pin_b.setter
	@copy_doc(psm.upisnd_setup_set_encoder_pin_b_id)
	def encoder_pin_b(self, value: Pin) -> None:
		result, setup = psm.upisnd_setup_set_encoder_pin_b_id(self._setup, value)
		if result != 0:
			raise ValueError(f"Failed to set encoder pin B: {result}")
		self._setup = setup

	@property
	@copy_doc(psm.upisnd_setup_get_encoder_pin_b_pull)
	def encoder_pin_b_pull(self) -> PinPull:
		return PinPull(psm.upisnd_setup_get_encoder_pin_b_pull(self._setup))

	@encoder_pin_b_pull.setter
	@copy_doc(psm.upisnd_setup_set_encoder_pin_b_pull)
	def encoder_pin_b_pull(self, value: PinPull) -> None:
		result, setup = psm.upisnd_setup_set_encoder_pin_b_pull(self._setup, value)
		if result != 0:
			raise ValueError(f"Failed to set encoder pin B pull: {result}")
		self._setup = setup

	@property
	@copy_doc(psm.upisnd_setup_get_activity_type)
	def activity_type(self) -> ActivityType:
		return ActivityType(psm.upisnd_setup_get_activity_type(self._setup))

	@activity_type.setter
	@copy_doc(psm.upisnd_setup_set_activity_type)
	def activity_type(self, value: ActivityType) -> None:
		result, setup = psm.upisnd_setup_set_activity_type(self._setup, value)
		if result != 0:
			raise ValueError(f"Failed to set activity type: {result}")
		self._setup = setup

	def to_int(self) -> int:
		"""Get the raw setup integer value."""
		return self._setup

	@classmethod
	def from_int(cls, setup_int: int) -> Self:
		"""Create a Setup object from an existing integer setup value.
		
		Args:
			setup_int: An integer representing a upisnd_setup_t value
			
		Returns:
			A new Setup object initialized with the given setup value
		"""
		setup = cls()
		setup._setup = setup_int
		return setup

	@staticmethod
	@copy_doc(psm.upisnd_setup_for_encoder)
	def for_encoder(pin_a: Pin, pull_a: PinPull, pin_b: Pin, pull_b: PinPull) -> 'Setup':
		setup = Setup()
		result, setup_value = psm.upisnd_setup_for_encoder(setup._setup, pin_a, pull_a, pin_b, pull_b)
		if result != 0:
			raise ValueError(f"Failed to setup encoder: {result}")
		setup._setup = setup_value
		return setup

	@staticmethod
	@copy_doc(psm.upisnd_setup_for_analog_input)
	def for_analog_input(pin: Pin) -> 'Setup':
		setup = Setup()
		result, setup_value = psm.upisnd_setup_for_analog_input(setup._setup, pin)
		if result != 0:
			raise ValueError(f"Failed to setup analog input: {result}")
		setup._setup = setup_value
		return setup

	@staticmethod
	@copy_doc(psm.upisnd_setup_for_gpio_input)
	def for_gpio_input(pin: Pin, pull: PinPull) -> 'Setup':
		setup = Setup()
		result, setup_value = psm.upisnd_setup_for_gpio_input(setup._setup, pin, pull)
		if result != 0:
			raise ValueError(f"Failed to setup GPIO input: {result}")
		setup._setup = setup_value
		return setup

	@staticmethod
	@copy_doc(psm.upisnd_setup_for_gpio_output)
	def for_gpio_output(pin: Pin, high: int) -> 'Setup':
		setup = Setup()
		result, setup_value = psm.upisnd_setup_for_gpio_output(setup._setup, pin, high)
		if result != 0:
			raise ValueError(f"Failed to setup GPIO output: {result}")
		setup._setup = setup_value
		return setup

	@staticmethod
	@copy_doc(psm.upisnd_setup_for_activity)
	def for_activity(pin: Pin, activity: ActivityType) -> 'Setup':
		setup = Setup()
		result, setup_value = psm.upisnd_setup_for_activity(setup._setup, pin, activity)
		if result != 0:
			raise ValueError(f"Failed to setup activity: {result}")
		setup._setup = setup_value
		return setup


@copy_doc(psm.upisnd_setup)
def setup_element(name: Union[str, ElementName], setup: Union[Setup, int]) -> Element:
	return Element.setup(name, setup)
