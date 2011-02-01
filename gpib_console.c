#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <usb.h>

struct usb_device *scope_dev = NULL;
struct usb_dev_handle *scope = NULL;

void gpib_send_char(unsigned char ch)
{
	usb_control_msg(scope, 0xc0, 1, ch, 0, NULL, 0, 1000);
}

void gpib_send(const char *str)
{
	int i;

	for (i = 0; str[i]; i++)
		gpib_send_char(str[i]);
	gpib_send_char('\r');
}

int gpib_get_response_length()
{
	unsigned char len;

	usb_control_msg(scope, 0xc0, 0, 0, 0, &len, 1, 1000);

	return len;
}

int gpib_get_response(char *buf, int maxlen)
{
	int i, len;

	len = gpib_get_response_length();
	if (!len)
	{
		if (maxlen)
			buf[0] = 0;

		return 0;
	}

	memset(buf, 'A', maxlen);
	buf[maxlen - 1] = 0;

	if (len > (maxlen - 1))
		len = maxlen - 1;

	len = usb_control_msg(scope, 0xc0, 0, 1, 0, buf, len, 1000);

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

void gpib_flush()
{
	unsigned char buf[256];
	int len, total = 0;

	do
	{
		len = gpib_get_response(buf, sizeof(buf));
		total += len;
	} while (len);
	printf("Flushed %d bytes\n", total);
}

int split(char *str, char delim, char **tok, int num)
{
	int i, t;

	t = 0;
	tok[0] = str;
	for (i = 0; str[i] && t < num; i++)
	{
		if (str[i] == delim)
		{
			str[i] = 0;
			i++;
			t++;
			tok[t] = str + i;
		}
	}

	return t + 1;
}

void strip_crlf(char *str)
{
	int i;

	for (i = strlen(str) - 1; i >= 0; i--)
	{
		if (str[i] == '\r' || str[i] == '\n')
		{
			str[i] = 0;
		} else {
			break;
		}
	}
}

void identify()
{
	char buf[256];
	char *tok[4];

	/* Get manufacturer, model, and serial number */
	gpib_send("*IDN?");
	gpib_get_response(buf, sizeof(buf));
	strip_crlf(buf);

	if (split(buf, ',', tok, 4) != 4)
	{
		printf("Can't parse IDN response\n");
		return;
	}

	printf("Manufacturer: %s\n", tok[0]);
	printf("Model:        %s\n", tok[1]);
	printf("Version:      %s\n", tok[3]);

	// Old 03.01.18 version
	// gpib_send(":SYS_INFO:ID:?");
	gpib_send(":SYS_INFO:ID?");
	if (gpib_get_response(buf, sizeof(buf)))
	{
		strip_crlf(buf);
	} else {
		strcpy(buf, "(unknown)");
	}

	printf("Serial:       %s\n", buf);
	printf("\n");
}

void dump_mem()
{
	unsigned char buf[256];
	FILE *fp;

	fp = fopen("mem.txt", "wt");
	if (!fp)
	{
		printf("Can't write to mem.txt\n");
		return;
	}

	fprintf(fp, "# name: mem\n");
	fprintf(fp, "# type: matrix\n");
	fprintf(fp, "# rows: 1\n");
	fprintf(fp, "# columns: 4000\n");

	gpib_send(":WAV:MEM?");
	while (gpib_get_response(buf, sizeof(buf)))
	{
		char *tok, *str;
		char *end = strchr(buf, '\n');
		if (end)
			*end = 0;

		str = buf;
		while ((tok = strtok(str, " ")))
		{
			fprintf(fp, "%d ", (int)strtol(tok + 2, 0, 16));
			str = NULL;
		}

		if (end)
			break;
	}

	fclose(fp);
}

void dump_screen()
{
	unsigned char buf[256];
	FILE *fp;

	fp = fopen("scr.txt", "wt");
	if (!fp)
	{
		printf("Can't write to scr.txt\n");
		return;
	}

	fprintf(fp, "# name: scr\n");
	fprintf(fp, "# type: matrix\n");
	fprintf(fp, "# rows: 1\n");
	fprintf(fp, "# columns: 600\n");

	gpib_send(":WAV:SCREENDATA?");
	while (gpib_get_response(buf, sizeof(buf)))
	{
		char *tok, *str;
		char *end = strchr(buf, '\n');
		if (end)
			*end = 0;

		str = buf;
		while ((tok = strtok(str, " ")))
		{
			fprintf(fp, "%d ", (int)strtol(tok + 2, 0, 16));
			str = NULL;
		}

		if (end)
			break;
	}

	fclose(fp);
}

int main()
{
	struct usb_bus *busses, *bus;
	struct usb_device *dev;
	unsigned char buf[256];

	usb_init();
	usb_find_busses();
	usb_find_devices();

	busses = usb_get_busses();
	for (bus = busses; bus; bus = bus->next)
	{
		for (dev = bus->devices; dev; dev = dev->next)
		{
			if (dev->descriptor.idVendor == 0x0400 && dev->descriptor.idProduct == 0xc55d)
			{
				scope_dev = dev;
				scope = usb_open(scope_dev);
				if (!scope_dev)
				{
					printf("Can't open scope\n");
					return 1;
				}
			}
		}
	}

	if (!scope_dev)
	{
		printf("No scope found\n");
		return 1;
	}

	if (usb_set_configuration(scope, 1))
	{
		printf("Can't set config\n");
		return 1;
	}

	if (usb_claim_interface(scope, 0))
	{
		printf("Can't claim interface\n");
		return 1;
	}

	gpib_flush();

	identify();

	while (1)
	{
		printf("> ");
		fflush(stdout);

		fgets(buf, sizeof(buf), stdin);
		if (buf[0] == 0)
			break;
		buf[strlen(buf) - 1] = 0;
		if (buf[0] == 0)
			break;

		if (!strcasecmp(buf, "&mem"))
		{
			dump_mem();
			continue;
		} else if (!strcasecmp(buf, "&scr"))
		{
			dump_screen();
			continue;
		}

		gpib_send(buf);
		while (gpib_get_response(buf, sizeof(buf)))
			printf("%s", buf);
		printf("\n");
	}

	return 0;
}

