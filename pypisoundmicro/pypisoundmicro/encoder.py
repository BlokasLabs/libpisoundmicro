from ._utils import copy_doc
from .swig import pypisoundmicro as psm
from typing import Self, Type, Tuple, Optional, Literal, Union, overload
from .types import Pin, Range, PinPull, ValueMode
from . import Element


class EncoderOpts:
	"""
	Wrapper for encoder options.
	
	This class provides an interface for working with encoder
	options for configuring input and value ranges and value mode.
	"""

	def __init__(self, 
				 input_range: Range = None, 
				 value_range: Range = None,
				 value_mode: int = ValueMode.WRAP) -> None:
		"""
		Initialize EncoderOpts with input range, value range, and value mode.
		
		Args:
			input_range: Range object for input values
			value_range: Range object for output values
			value_mode: Mode for handling values out of range (CLAMP or WRAP)
		"""
		self._opts = psm.upisnd_encoder_opts_t()
		
		# Default ranges
		if input_range is None:
			input_range = Range(0, 100)
		if value_range is None:
			value_range = Range(0, 100)
			
		self.input_range = input_range
		self.value_range = value_range
		self.value_mode = value_mode

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

	@property
	def value_mode(self) -> int:
		"""Get the current value mode."""
		return self._opts.value_mode

	@value_mode.setter
	def value_mode(self, mode: int) -> None:
		"""Set the value mode."""
		self._opts.value_mode = mode

	@classmethod
	def from_c_opts(cls, opts: psm.upisnd_encoder_opts_t) -> Self:
		"""Create EncoderOpts from a C struct."""
		result = cls()
		result._opts = opts
		return result
	
	def to_c_opts(self) -> psm.upisnd_encoder_opts_t:
		"""Convert to C struct."""
		return self._opts

@copy_doc(psm.Encoder)
class Encoder(Element):
	@classmethod
	@copy_doc(psm.Encoder.setup)
	def setup(cls: Type[Self], name: str, pin_a: Pin, pull_a: PinPull, 
			  pin_b: Pin, pull_b: PinPull) -> Self:
		native_obj = psm.Encoder.setup(name, pin_a, pull_a, pin_b, pull_b)
		return cls(native_obj)

	@copy_doc(psm.Encoder.get)
	def get_value(self) -> int:
		return self._native_obj.get()

	@copy_doc(psm.Encoder.setOpts)
	def set_opts(self, opts: EncoderOpts) -> int:
		return self._native_obj.setOpts(opts.to_c_opts())

	@copy_doc(psm.Encoder.getOpts)
	def get_opts(self) -> EncoderOpts:
		c_opts = psm.upisnd_encoder_opts_t()
		result = self._native_obj.getOpts(c_opts)
		if result != 0:
			raise IOError(f"Failed to get encoder options: {result}")
		return EncoderOpts.from_c_opts(c_opts)

	@copy_doc(psm.Encoder.getPinB)
	def get_pin_b(self) -> Pin:
		return self._native_obj.getPinB()

	@copy_doc(psm.Encoder.getPinPull)
	def get_pin_pull(self) -> PinPull:
		return self._native_obj.getPinPull()

	@copy_doc(psm.Encoder.getPinBPull)
	def get_pin_b_pull(self) -> PinPull:
		return self._native_obj.getPinBPull()
