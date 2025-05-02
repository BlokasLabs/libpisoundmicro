"""Type definitions for the pypisoundmicro package."""

from typing import Union, TypeVar
from enum import IntEnum, auto
from .swig import pypisoundmicro as psm

# Define type aliases for common types
class Range:
	"""Range class with high and low properties for input and value ranges."""
	def __init__(self, low: int = 0, high: int = 0):
		"""Initialize a Range object.
		
		Args:
			low: The lower bound of the range
			high: The upper bound of the range
		"""
		self.low = low
		self.high = high

# Maximum element name length imported directly from the SWIG wrapper
UPISND_MAX_ELEMENT_NAME_LENGTH = psm.UPISND_MAX_ELEMENT_NAME_LENGTH

class Pin(IntEnum):
	"""Pin numbers for the Pisound Micro."""
	# Header A pins
	A27 = psm.UPISND_PIN_A27
	A28 = psm.UPISND_PIN_A28
	A29 = psm.UPISND_PIN_A29
	A30 = psm.UPISND_PIN_A30
	A31 = psm.UPISND_PIN_A31
	A32 = psm.UPISND_PIN_A32
	
	# Header B pins
	B03 = psm.UPISND_PIN_B03
	B04 = psm.UPISND_PIN_B04
	B05 = psm.UPISND_PIN_B05
	B06 = psm.UPISND_PIN_B06
	B07 = psm.UPISND_PIN_B07
	B08 = psm.UPISND_PIN_B08
	B09 = psm.UPISND_PIN_B09
	B10 = psm.UPISND_PIN_B10
	B11 = psm.UPISND_PIN_B11
	B12 = psm.UPISND_PIN_B12
	B13 = psm.UPISND_PIN_B13
	B14 = psm.UPISND_PIN_B14
	B15 = psm.UPISND_PIN_B15
	B16 = psm.UPISND_PIN_B16
	B17 = psm.UPISND_PIN_B17
	B18 = psm.UPISND_PIN_B18

	B23 = psm.UPISND_PIN_B23
	B24 = psm.UPISND_PIN_B24
	B25 = psm.UPISND_PIN_B25
	B26 = psm.UPISND_PIN_B26
	B27 = psm.UPISND_PIN_B27
	B28 = psm.UPISND_PIN_B28
	B29 = psm.UPISND_PIN_B29
	B30 = psm.UPISND_PIN_B30
	B31 = psm.UPISND_PIN_B31
	B32 = psm.UPISND_PIN_B32
	B33 = psm.UPISND_PIN_B33
	B34 = psm.UPISND_PIN_B34

	B37 = psm.UPISND_PIN_B37
	B38 = psm.UPISND_PIN_B38
	B39 = psm.UPISND_PIN_B39

	INVALID = psm.UPISND_PIN_INVALID
	"""Value for indicating an invalid pin."""


class ElementType(IntEnum):
	"""Element type enumeration."""
	INVALID = psm.UPISND_ELEMENT_TYPE_INVALID
	NONE = psm.UPISND_ELEMENT_TYPE_NONE
	ENCODER = psm.UPISND_ELEMENT_TYPE_ENCODER
	ANALOG_INPUT = psm.UPISND_ELEMENT_TYPE_ANALOG_INPUT
	GPIO = psm.UPISND_ELEMENT_TYPE_GPIO
	ACTIVITY = psm.UPISND_ELEMENT_TYPE_ACTIVITY


class PinPull(IntEnum):
	"""Pin pull-up/pull-down configuration."""
	INVALID = psm.UPISND_PIN_PULL_INVALID
	NONE = psm.UPISND_PIN_PULL_NONE
	UP = psm.UPISND_PIN_PULL_UP
	DOWN = psm.UPISND_PIN_PULL_DOWN


class PinDirection(IntEnum):
	"""Pin direction (input/output)."""
	INVALID = psm.UPISND_PIN_DIR_INVALID
	INPUT = psm.UPISND_PIN_DIR_INPUT
	OUTPUT = psm.UPISND_PIN_DIR_OUTPUT


class ActivityType(IntEnum):
	"""Activity type for LED indicators."""
	INVALID = psm.UPISND_ACTIVITY_INVALID
	MIDI_INPUT = psm.UPISND_ACTIVITY_MIDI_INPUT
	MIDI_OUTPUT = psm.UPISND_ACTIVITY_MIDI_OUTPUT


class ValueMode(IntEnum):
	"""Value mode for encoders and analog inputs."""
	INVALID = psm.UPISND_VALUE_MODE_INVALID
	CLAMP = psm.UPISND_VALUE_MODE_CLAMP
	"""The value is clamped to input_min and input_max range."""
	WRAP = psm.UPISND_VALUE_MODE_WRAP
	"""The value is wrapped over to the other boundary of the input range."""
