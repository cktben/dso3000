This is a set of tools in Python and C for interfacing with an Agilent DSO3000-series oscilloscope.
These are actually rebadged Rigol DS5000 oscilloscopes.
I have not tested this code with an actual Rigol-branded scope.  I have an Agilent DSO3102A.

gpib_console.c	A simple C console program.
command		Execute one command or query from the command line.
		If a query is given (ending in '?'), the result will be printed.
console		A Python GPIB console.
screenshot	Saves a screenshot from the scope.
dso3000.py	Python class for interfacing with a DSO3000-series oscilloscope.
rigol-scope.rules
		Copy to /etc/udev/rules.d before plugging in the scope to
		make it available to users in the plugdev group.
commands.txt	A summary of known commands.  Not all are well understood.

libusb (http://www.libusb.org/) is required to build the C code.
libusb and PyUSB (http://pyusb.berlios.de/) are required to use the Python programs.
The Python Imaging Library (http://www.pythonware.com/products/pil/) is required to save screenshots.

rigol-scope.rules should be the only Linux-specific file, but I have not tried to use this code
on any other platform.

You must upgrade the scope's firmware.  Very early versions have serious GPIB-handling bugs.
The current version still has some problems (block data has extra junk after it), but the code works around that.

Agilent provides a Programmer's Reference which you should download from their web site.
It describes all the GPIB commands regular users should be aware of (except for input termination, which is
a secret feature of the scope).

This code was written by Ben Johnson.  It may be used, modified, and distributed without restriction
(although you should acknowledge me as the source in derivative works).
Be aware that this code is not based on official support from the manufacturer, and you will be using it at your own risk.

The original version of this code is maintained at:
https://github.com/cktben/dso3000

My web site, including a description of the communications protocol, is at:
http://www.circuitben.net/dso3000

