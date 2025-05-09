from ._utils import copy_doc
from . import Element
from .swig import pypisoundmicro as psm
from typing import Self, Type
from .types import PinDirection, PinPull

@copy_doc(psm.Gpio)
class Gpio(Element):
	@classmethod
	@copy_doc(psm.Gpio.setupInput)
	def setup_input(cls: Type[Self], name: str, pin: int, pull: PinPull) -> Self:
		native_obj = psm.Gpio.setupInput(name, pin, pull)
		return cls(native_obj)

	@classmethod
	@copy_doc(psm.Gpio.setupOutput)
	def setup_output(cls: Type[Self], name: str, pin: int, high: int) -> Self:
		native_obj = psm.Gpio.setupOutput(name, pin, high)
		return cls(native_obj)

	@property
	@copy_doc(psm.Gpio.getDirection)
	def direction(self) -> PinDirection:
		return self._native_obj.getDirection()

	@property
	@copy_doc(psm.Gpio.getPull)
	def pull(self) -> PinPull:
		return self._native_obj.getPull()

	@copy_doc(psm.Gpio.get)
	def get_value(self) -> int:
		return self._native_obj.get()

	@copy_doc(psm.Gpio.set)
	def set_value(self, high: int) -> int:
		return self._native_obj.set(high)
