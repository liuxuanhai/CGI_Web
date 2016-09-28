#include "qr_png.h"
#include "qrencode.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <png.h>
#include <fcntl.h>


#define INCHES_PER_METER (100.0/2.54)

static unsigned char fg_color[4] = {0, 0, 0, 255};
static unsigned char bg_color[4] = {255, 255, 255, 255};
static png_color palette[2] = {{fg_color[0], fg_color[1], fg_color[2]}, {bg_color[0], bg_color[1], bg_color[2]}};
static png_byte alpha_values[2] = {fg_color[3], bg_color[3]};


static void write_data_fn(png_structp png_ptr, png_bytep data, png_size_t datalen)
{
	void* pUserData = png_get_io_ptr(png_ptr);
	((string*)pUserData)->append((const char*)data, datalen);
	return;
}

int QRPng::url_2_png_easy(const std::string& strUrl, string& strPng, string& strErrMsg)
{
	return url_2_png(strUrl, strPng, strErrMsg, 7, 4, 3, 72);
}

int QRPng::url_2_png(const std::string& strUrl, string& strPng, string& strErrMsg, int version, int margin, int size, int dpi)
{
	strPng = "";
	strErrMsg = "";
	
	QRcode *code = QRcode_encodeString((char *)strUrl.c_str(), version, QR_ECLEVEL_L, QR_MODE_8, 1);
	if(!code)
	{
		strErrMsg = "QRcode_encodeString failed";
		return -1;
	}

	unsigned char *row, *p, *q;
	int x, y, xx, yy, bit;
	int realwidth;

	realwidth = (code->width + margin * 2) * size;
	row = (unsigned char *)malloc((realwidth + 7) / 8);
	if(row == NULL) 
	{
		strErrMsg = "Failed to allocate memory";
		return -1;
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(png_ptr == NULL) {
		strErrMsg = "Failed to initialize PNG writer";
		free(row);
		return -1;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(info_ptr == NULL) {
		strErrMsg = "Failed to initialize PNG write";
		free(row);
		return -1;
	}

	png_set_PLTE(png_ptr, info_ptr, palette, 2);
	png_set_tRNS(png_ptr, info_ptr, alpha_values, 2, NULL);

	png_set_write_fn(png_ptr, (void*)&strPng, write_data_fn, NULL);

	png_set_IHDR(png_ptr, info_ptr,
			realwidth, realwidth,
			1,
			PNG_COLOR_TYPE_PALETTE,
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);
	png_set_pHYs(png_ptr, info_ptr,
			png_uint_32(dpi * INCHES_PER_METER),
			png_uint_32(dpi * INCHES_PER_METER),
			PNG_RESOLUTION_METER);
	png_write_info(png_ptr, info_ptr);

	/* top margin */
	memset(row, 0xff, (realwidth + 7) / 8);
	for(y=0; y<margin * size; y++) {
		png_write_row(png_ptr, row);
	}

	/* data */
	p = code->data;
	for(y=0; y<code->width; y++) {
		bit = 7;
		memset(row, 0xff, (realwidth + 7) / 8);
		q = row;
		q += margin * size / 8;
		bit = 7 - (margin * size % 8);
		for(x=0; x<code->width; x++) {
			for(xx=0; xx<size; xx++) {
				*q ^= (*p & 1) << bit;
				bit--;
				if(bit < 0) {
					q++;
					bit = 7;
				}
			}
			p++;
		}
		for(yy=0; yy<size; yy++) {
			png_write_row(png_ptr, row);
		}
	}
	/* bottom margin */
	memset(row, 0xff, (realwidth + 7) / 8);
	for(y=0; y<margin * size; y++) {
		png_write_row(png_ptr, row);
	}

	png_write_end(png_ptr, info_ptr);

	png_destroy_write_struct(&png_ptr, &info_ptr);
	free(row);

	return 0;
}

