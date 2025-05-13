from ._utils import copy_doc
from . import Element, ElementName
from .swig import pypisoundmicro as psm
from typing import Self, Type, Union
from .types import ActivityType, Pin

@copy_doc(psm.Activity)
class Activity(Element):
	@classmethod
	@copy_doc(psm.Activity.setup)
	def setup_activity(cls: Type[Self], name: Union[str, ElementName, psm.ElementName], pin: Pin, activity: ActivityType) -> Self:
		if isinstance(name, str):
			name = psm.ElementName.regular(name)
		elif isinstance(name, ElementName):
			name = name._name
		native_obj = psm.Activity.setupActivity(name, pin, activity)
		return cls(native_obj)

	@property
	@copy_doc(psm.Activity.getActivity)
	def activity_type(self) -> ActivityType:
		return self._native_obj.getActivity()
