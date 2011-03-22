#pragma once

#include <libusb.h>

class DSO3000
{
	public:
		DSO3000();
		~DSO3000();

		// Reads and discards all data from the scope's response buffer.
		// Returns the number of bytes read.
		int flush();

		void writeChar(char ch);
		void write(const char *str);
		int getResponseLength();
		int read(char *buf, int maxlen);
		
		// Acquires a screenshot.
		// buf must point to a 76800-byte buffer to hold the 320x240 image,
		// one byte per pixel.  Each byte is a color with the form RRGGBBxx.
		bool screenshot(unsigned char *buf);

	private:
		libusb_context *_usb;
		libusb_device_handle *_device;
		
		// Time in milliseconds to wait for USB transfers to complete
		unsigned int _timeout;
};
