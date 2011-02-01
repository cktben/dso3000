import usb

class DSO3000:
	'''Class to interface over USB with a Agilent DSO3000A-series (and probably Rigol DSO5000) oscilloscope.
	
	Example:
	  scope = DSO3000()    # Connect to scope
	  print scope.command(':CHAN1:COUPL?')   # Print selected channel
	  print scope.readScreen(1)              # Read waveform in volts
	'''
	
	def __init__(self, flush=True):
		'''Finds and opens the scope's USB device.
		If flush==True, any pending data is discarded.'''
		
		self.device = None
		
		# Time in milliseconds to wait for a USB control transaction to finish.
		# *RST takes about 1500ms, so the timeout should be longer than that.
		self.timeout = 10000
		
		for bus in usb.busses():
			for dev in bus.devices:
				if dev.idVendor == 0x0400 and dev.idProduct == 0xc55d:
					self.device = dev.open()
					self.device.setConfiguration(1)
					self.device.claimInterface(0)
					break
		if not self.device:
			raise IOError('No USB oscilloscope found')
		if flush:
			self.read()
	
	def writeChar(self, ch):
		'''Writes a single character to the scope'''
		assert self.device.controlMsg(0xc0, 1, 0, ord(ch), 0, self.timeout) == ()
	
	def write(self, s):
		'''Writes a string and an end-of-line to the scope'''
		for ch in s:
			self.writeChar(ch)
		self.writeChar('\r')

	def getResponseLength(self):
		'''Returns the number of characters waiting to be read.
		If more than 255 characters are available, returns 255.'''
		return self.device.controlMsg(0xc0, 0, 1, 0, 0, self.timeout)[0]
	
	def read(self):
		'''Reads as much data as is currently available'''
		text = ''
		while True:
			n = self.getResponseLength()
			if n == 0:
				break
			data = self.device.controlMsg(0xc0, 0, n, 1, 0, self.timeout)
			text += ''.join(chr(x) for x in data).rstrip('\r\n')
		
		return text
	
	def command(self, s):
		'''Writes a command string and returns the response (if any)'''
		
		# Write the command
		self.write(s)
		
		# Read the response
		text = self.read()
		if not text:
			# No response text
			return None

		# When requesting waveform data, the firmware sends a
		# bunch of junk in memory that we have to discard.
		# Keep everything up to the first newline.
		return text.splitlines()[0]
	
	def readData(self, command):
		'''Reads waveform data with the given command and returns the waveform in volts.'''
		# Send the command
		text = self.command(command)
		
		# Parse the result into ADC readings
		data = [int(x, 16) for x in text.split()]
		
		# Get channel configuration needed to convert to volts
		yinc = float(self.command(':WAV:YINC?'))
		yor = float(self.command(':WAV:YOR?'))
		
		# Convert ADC readings to volts.
		# Agilent's Programmer's Reference appears to have this formula right,
		# but setting a channel to GND produces all 126's (one count below zero volts).
		data = [(125.0 - x) * yinc - yor for x in data]
		return data
	
	def readMemory(self, channel):
		'''Reads waveform memory for the given channel (1 or 2)'''
		self.command(':WAV:SOUR CHANNEL%d' % channel)
		return self.readData(':WAV:MEM?')

	def readScreen(self, channel):
		'''Reads the waveform on the screen for the given channel (1 or 2)'''
		self.command(':WAV:SOUR CHANNEL%d' % channel)
		return self.readData(':WAV:DATA?')
	
	def screenshot(self):
		self.write(':HARD_COPY')
		assert self.device.controlMsg(0xc0, 8, 0, 0, 0x50, self.timeout) == ()
		# wIndex:wValue is the length of data returned
		assert self.device.controlMsg(0xc0, 9, 0, 0x2c00, 1, self.timeout) == ()
		assert self.device.controlMsg(0xc0, 7, 0, 0, 0x50, self.timeout) == ()
		data = self.device.bulkRead(1, 76800, self.timeout)
		assert self.getResponseLength() == 0
		return data

# Colors:
# 50 background yellow
# 30 Remote red/pink
# c0 green
# a8 white
# e0 yellow
# f0 blue
# inverted changes the meaning of e0...
