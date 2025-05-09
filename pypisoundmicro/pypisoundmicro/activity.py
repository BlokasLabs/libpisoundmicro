from ._utils import copy_doc
from . import Element
from .swig import pypisoundmicro as psm
from typing import Self, Type
from .types import ActivityType, Pin

@copy_doc(psm.Activity)
class Activity(Element):
	@classmethod
	@copy_doc(psm.Activity.setupActivity)
	def setup_activity(cls: Type[Self], name: str, pin: Pin, activity: ActivityType) -> Self:
		native_obj = psm.Activity.setupActivity(name, pin, activity)
		return cls(native_obj)

	@property
	@copy_doc(psm.Activity.getActivity)
	def activity_type(self) -> ActivityType:
		return self._native_obj.getActivity()
