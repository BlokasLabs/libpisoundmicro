from ._utils import copy_doc
from .swig import pypisoundmicro as psm
from typing import Self, Type, Optional
from .types import Pin, Range
from . import Element

class AnalogInputOpts:
	"""
	Wrapper for analog input options.
	
	This class provides an interface for working with analog input
	options for configuring input and value ranges.
	"""

	def __init__(self, input_range: Range = None, value_range: Range = None) -> None:
		"""
		Initialize AnalogInputOpts with input and value ranges.
		
		Args:
			input_range: Range object for input values
			value_range: Range object for output values
		"""
		self._opts = psm.upisnd_analog_input_opts_t()
		
		# Default ranges
		if input_range is None:
			input_range = Range(0, 1023)
		if value_range is None:
			value_range = Range(0, 1023)
			
		self.input_range = input_range
		self.value_range = value_range

	@property
	def input_range(self) -> Range:
		"""Get the current input range."""
		return Range(self._opts.input_range.low, self._opts.input_range.high)

	@input_range.setter
	def input_range(self, value: Range) -> None:
		"""Set the input range."""
		self._opts.input_range.low = value.low
		self._opts.input_range.high = value.high

	@property
	def value_range(self) -> Range:
		"""Get the current value range."""
		return Range(self._opts.value_range.low, self._opts.value_range.high)

	@value_range.setter
	def value_range(self, value: Range) -> None:
		"""Set the value range."""
		self._opts.value_range.low = value.low
		self._opts.value_range.high = value.high

	@classmethod
	def from_c_opts(cls, opts: psm.upisnd_analog_input_opts_t) -> Self:
		"""Create AnalogInputOpts from a C struct."""
		result = cls()
		result._opts = opts
		return result

	def to_c_opts(self) -> psm.upisnd_analog_input_opts_t:
		"""Convert to C struct."""
		return self._opts

@copy_doc(psm.AnalogInput)
class AnalogInput(Element):
	@classmethod
	@copy_doc(psm.AnalogInput.setup)
	def setup(cls: Type[Self], name: str, pin: Pin) -> Self:
		native_obj = psm.AnalogInput.setup(name, pin)
		return cls(native_obj)

	@copy_doc(psm.AnalogInput.get)
	def get_value(self) -> int:
		return self._native_obj.get()

	@copy_doc(psm.AnalogInput.setOpts)
	def set_opts(self, opts: AnalogInputOpts) -> int:
		return self._native_obj.setOpts(opts.to_c_opts())

	@copy_doc(psm.AnalogInput.getOpts)
	def get_opts(self) -> AnalogInputOpts:
		c_opts = psm.upisnd_analog_input_opts_t()
		result = self._native_obj.getOpts(c_opts)
		if result != 0:
			raise IOError(f"Failed to get analog input options: {result}")
		return AnalogInputOpts.from_c_opts(c_opts)
