"""Element naming utilities for the pypisoundmicro package."""

from typing import Optional, Self
from ._utils import copy_doc
from .swig import pypisoundmicro as psm

@copy_doc(psm.ElementName)
class ElementName:
	def __init__(self, name: str) -> None:
		"""Initialize an ElementName from a string.
		
		Args:
			name: The element name as string
		"""
		self._name = psm.ElementName.regular(name)
	
	def __str__(self) -> str:
		"""Convert the ElementName to a string."""
		return str(self._name)

	@classmethod
	def regular(cls, name: str) -> Self:
		"""Create an ElementName from a string.
		
		In Python, you can use f-strings for formatting element names:
		
		Examples:
			```py3
			# Basic usage
			ElementName.regular("encoder_1")
			
			# With f-strings for formatting
			index = 5
			ElementName.regular(f"encoder_{index}")
			
			# With f-strings and formatting options
			ElementName.regular(f"ctrl_{index:02d}")  # Produces "ctrl_05"
			```
		"""
		result = cls.__new__(cls)
		result._name = psm.ElementName.regular(name)
		return result

	@classmethod
	@copy_doc(psm.ElementName.randomized)
	def randomized(cls, prefix: Optional[str] = None) -> Self:
		result = cls.__new__(cls)
		result._name = psm.ElementName.randomized(prefix)
		return result