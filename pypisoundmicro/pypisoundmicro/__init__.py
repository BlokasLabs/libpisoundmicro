from .element import Element
from .name import ElementName
from .analoginput import AnalogInput
from .activity import Activity
from .encoder import Encoder
from .gpio import Gpio
from .types import ActivityType, ElementType, Pin, PinDirection, PinPull, Range, ValueMode
from .setup import Setup
from .valuefd import ValueFd

from .swig.pypisoundmicro import _init, _cleanup

def init() -> None:
	"""
	Initialize libpisoundmicro resources.
	
	This function initializes the libpisoundmicro library and prepares it
	for use. It is automatically called when the module is imported, but you can
	call it explicitly if needed, in case you have used cleanup explicitly and need
	to reinitialize the library.
	
	Thread-safe:
		This function is thread-safe and can be called from any thread.
	
	Example:
		import pypisoundmicro as psm
		
		psm.init()
	"""
	_init()

def cleanup() -> None:
	"""
	Clean up libpisoundmicro resources.
	
	This function ensures that all libpisoundmicro resources are properly
	released, including any remaining elements and the global initializer.
	
	Note:
		This function is automatically registered with atexit when the module
		is imported, so resources will be cleaned up during normal program
		termination. However, for abnormal terminations (e.g., when signals
		are received), your main script should call this function explicitly
		in its signal handlers.
	
	Thread-safe:
		This function is thread-safe and can be called from any thread.
	
	Example:
		# For handling abnormal termination with signals:
		
		import signal
		import sys
		import pypisoundmicro as psm
		
		def signal_handler(signum, frame):
			# Clean up pisound resources first
			psm.cleanup()
			
			# Then exit or perform other cleanup
			sys.exit(1)
		
		# Register signal handlers for common termination signals
		signal.signal(signal.SIGINT, signal_handler)   # Ctrl+C
		signal.signal(signal.SIGTERM, signal_handler)  # Termination signal
	"""
	_cleanup()

__all__ = [
	'Element',
	'ElementName',
	'AnalogInput',
	'Activity',
	'Encoder',
	'Gpio',
	'ActivityType',
	'ElementType',
	'Pin',
	'PinDirection',
	'PinPull',
	'Range',
	'ValueMode',
	'Setup',
	'ValueFd',

	'init',
	'cleanup'
]
