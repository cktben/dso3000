#include "dso3000.hpp"

#include <png.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdexcept>

using namespace std;

void error(const char *str)
{
	printf("Content-type: text/plain\n\nERROR: %s\n", str);
}

int main(int argc, char *argv[])
{
	DSO3000 *scope;
	try
	{
		scope = new DSO3000();
	} catch (exception &e)
	{
		error(e.what());
		return 1;
	}
	
	unsigned char image[76800];
	if (!scope->screenshot(image))
	{
		error("Failed to acquire screenshot");
		return 1;
	}
	
	delete scope;
	
	printf("Content-type: image/png\n\n");
	
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr)
	{
		return 1;
	}
	
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, 0);
		return 1;
	}
	
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, 0);
		return 1;
	}
	
	png_init_io(png_ptr, stdout);
	png_set_IHDR(png_ptr, info_ptr, 320, 240, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
	png_color palette[256];
	for (int i = 0; i < 256; ++i)
	{
		palette[i].red   = i / 64 * 255 / 3;
		palette[i].green = (i & 0x30) / 16 * 255 / 3;
		palette[i].blue  = (i & 0x0c) / 4 * 255 / 3;
	}
	png_set_PLTE(png_ptr, info_ptr, palette, 256);
	
	png_bytep row_pointers[240];
	for (int i = 0; i < 240; ++i)
	{
		row_pointers[i] = image + i * 320;
	}
	png_set_rows(png_ptr, info_ptr, row_pointers);
	
	png_write_png(png_ptr, info_ptr, 0, 0);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	
	return 0;
}
