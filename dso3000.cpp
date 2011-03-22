#include "dso3000.hpp"

#include <libusb.h>
#include <stdexcept>

using namespace std;

DSO3000::DSO3000()
{
	_usb = 0;
	_timeout = 10000;

	if (libusb_init(&_usb))
	{
		throw runtime_error("Failed to initialize libusb");
	}

	_device = libusb_open_device_with_vid_pid(_usb, 0x0400, 0xc55d);
	if (!_device)
	{
		throw runtime_error("Failed to open USB device");
	}

	if (libusb_set_configuration(_device, 1))
	{
		throw runtime_error("Can't set configuration");
	}

	if (libusb_claim_interface(_device, 0))
	{
		throw runtime_error("Can't claim interface");
	}

	flush();
}

DSO3000::~DSO3000()
{
	if (_device)
	{
		libusb_close(_device);
	}

	libusb_exit(_usb);
}

int DSO3000::flush()
{
	char buf[256];
	int len, total = 0;

	do
	{
		len = read(buf, sizeof(buf));
		total += len;
	} while (len);

	return total;
}

void DSO3000::writeChar(char ch)
{
	libusb_control_transfer(_device, 0xc0, 1, ch, 0, 0, 0, _timeout);
}

void DSO3000::write(const char *str)
{
	for (int i = 0; str[i]; i++)
	{
		writeChar(str[i]);
	}
	writeChar('\r');
}

int DSO3000::getResponseLength()
{
	unsigned char len;

	libusb_control_transfer(_device, 0xc0, 0, 0, 0, &len, 1, _timeout);

	return len;
}

int DSO3000::read(char *buf, int maxlen)
{
	int i, len;

	len = getResponseLength();
	if (!len)
	{
		if (maxlen)
			buf[0] = 0;

		return 0;
	}

	if (len > (maxlen - 1))
	{
		len = maxlen - 1;
	}

	len = libusb_control_transfer(_device, 0xc0, 0, 1, 0, (unsigned char *)buf, len, _timeout);

	// Truncate the response at the first '\n' because the scope firmware sometimes
	// sends extra junk.
	for (i = 0; i < len; i++)
	{
		if (buf[i] == '\n')
		{
			len = i;
			break;
		}
	}
	buf[len] = 0;

	return len;
}

bool DSO3000::screenshot(unsigned char* buf)
{
	write(":HARD_COPY");
	
	if (libusb_control_transfer(_device, 0xc0, 8, 0, 0x50, 0, 0, _timeout))
	{
		return false;
	}
	
	// wIndex:wValue is the length of data returned
	if (libusb_control_transfer(_device, 0xc0, 9, 0x2c00, 1, 0, 0, _timeout))
	{
		return false;
	}
	
	if (libusb_control_transfer(_device, 0xc0, 7, 0, 0x50, 0, 0, _timeout))
	{
		return false;
	}
	
	int len = 0;
	if (libusb_bulk_transfer(_device, 0x81, buf, 76800, &len, _timeout))
	{
		return false;
	}
	
	getResponseLength();
	
	return len == 76800;
}
