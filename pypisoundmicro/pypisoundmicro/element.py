from .swig import pypisoundmicro as psm
from ._utils import copy_doc
from typing import Optional, Self, Type, Union
from .name import ElementName
from .types import Pin, ElementType, PinDirection, PinPull, ActivityType
from .valuefd import ValueFd
import os


@copy_doc(psm.Element)
class Element:
	def __init__(self, native_obj: Optional[psm.Element] = None) -> None:
		"""Initialize an Element.

		Args:
			native_obj: The native SWIG-wrapped Element object
		"""
		# Store the direct reference to preserve the object type
		self._native_obj = native_obj if native_obj is not None else psm.Element()

	@property
	@copy_doc(psm.Element.isValid)
	def is_valid(self) -> bool:
		if self._native_obj:
			return self._native_obj.isValid()
		return False

	@copy_doc(psm.Element.release)
	def release(self) -> None:
		if self._native_obj:
			self._native_obj.release()
			self._native_obj = None

	@property
	@copy_doc(psm.Element.getName)
	def name(self) -> Optional[str]:
		if self._native_obj:
			return self._native_obj.getName()
		return None

	@property
	@copy_doc(psm.Element.getType)
	def type(self) -> ElementType:
		if self._native_obj:
			return ElementType(self._native_obj.getType())
		return ElementType.INVALID

	@property
	@copy_doc(psm.Element.getPin)
	def pin(self) -> Pin:
		if self._native_obj:
			return Pin(self._native_obj.getPin())
		return Pin.INVALID

	@copy_doc(psm.Element.openValueFd)
	def open_value_fd(self, flags: int = os.O_RDWR | os.O_CLOEXEC) -> Optional[ValueFd]:
		if self._native_obj:
			# Use the native openValueFd method of the Element object
			native_fd = self._native_obj.openValueFd(flags)
			if native_fd and native_fd.isValid():
				# Convert from native ValueFd to our Python ValueFd
				return ValueFd(native_fd)
		return None

	def as_encoder(self) -> Optional['Encoder']:
		"""Cast the element to an Encoder if possible."""
		from .encoder import Encoder
		if self._native_obj and self.type == ElementType.ENCODER:
			# Use the SWIG-provided as_encoder method that properly handles reference counting
			swig_encoder = self._native_obj.as_encoder()
			if swig_encoder and swig_encoder.isValid():
				return Encoder(swig_encoder)
		return None

	def as_analog_input(self) -> Optional['AnalogInput']:
		"""Cast the element to an AnalogInput if possible."""
		from .analoginput import AnalogInput
		if self._native_obj and self.type == ElementType.ANALOG_INPUT:
			# Use the SWIG-provided as_analog_input method that properly handles reference counting
			swig_analog = self._native_obj.as_analog_input()
			if swig_analog and swig_analog.isValid():
				return AnalogInput(swig_analog)
		return None

	def as_gpio(self) -> Optional['Gpio']:
		"""Cast the element to a GPIO if possible."""
		from .gpio import Gpio
		if self._native_obj and self.type == ElementType.GPIO:
			# Use the SWIG-provided as_gpio method that properly handles reference counting
			swig_gpio = self._native_obj.as_gpio()
			if swig_gpio and swig_gpio.isValid():
				return Gpio(swig_gpio)
		return None

	def as_activity(self) -> Optional['Activity']:
		"""Cast the element to an Activity if possible."""
		from .activity import Activity
		if self._native_obj and self.type == ElementType.ACTIVITY:
			# Use the SWIG-provided as_activity method that properly handles reference counting
			swig_activity = self._native_obj.as_activity()
			if swig_activity and swig_activity.isValid():
				return Activity(swig_activity)
		return None

	@classmethod
	@copy_doc(psm.Element.get)
	def get(cls, name: Union[str, ElementName]) -> Optional[Self]:
		if isinstance(name, str):
			name = psm.ElementName.regular(name)

		native_obj = psm.Element.get(name)
		if native_obj.isValid():
			return cls(native_obj)
		return None

	@classmethod
	@copy_doc(psm.Element.setup)
	def setup(cls, name: Union[str, ElementName], setup_obj: Union[int, 'Setup']) -> Optional[Self]:
		"""Set up a new Element with the provided name and setup options.
		
		Args:
			name: The name of the element to set up
			setup_obj: Either a raw setup integer value or a Setup object
			
		Returns:
			A newly created Element, or None if setup failed
		"""
		if isinstance(name, str):
			name = psm.ElementName.regular(name)
		
		# Handle both raw integers and Setup objects
		if hasattr(setup_obj, 'to_int'):
			setup_value = setup_obj.to_int()
		else:
			setup_value = setup_obj
			
		native_obj = psm.Element.setup(name, setup_value)
		if native_obj.isValid():
			return cls(native_obj)
		return None

	def __enter__(self) -> Self:
		"""Context manager support."""
		return self

	def __exit__(self, exc_type, exc_val, exc_tb) -> None:
		"""Context manager support for auto-releasing resources."""
		self.release()
		
	def __del__(self) -> None:
		"""Destructor to ensure resources are released.
		
		Note: This may not always be called immediately when the object goes out of scope,
		especially if the object is part of a reference cycle. For deterministic cleanup,
		use the release() method explicitly or use the object as a context manager.
		"""
		try:
			# Check if _native_obj is still accessible
			if hasattr(self, '_native_obj') and self._native_obj:
				self.release()
		except Exception:
			# Silently ignore exceptions during garbage collection
			# This prevents errors that would be swallowed by Python anyway
			pass
