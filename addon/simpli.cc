#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#if HAVE_PNG
#include <png.h>
#endif

#include "qrencode.h"
#include "simpli.h"

#define MAX_CANCEL_THREADS 6
//define nullptr ((void*)0)

#define INCHES_PER_METER (100.0/2.54)

static int casesensitive = 1;
static int eightbit = 0;
static int version = 0;
static int size = 3;
static int margin = 4;//-1;
static int dpi = 72;
static int micro = 0;
static QRecLevel level = QR_ECLEVEL_L;
static QRencodeMode hint = QR_MODE_8;
static unsigned char fg_color[4] = {0, 0, 0, 255};
static unsigned char bg_color[4] = {255, 255, 255, 255};

enum imageType {
	PNG_TYPE,
	PNG32_TYPE
};

static enum imageType image_type = PNG_TYPE;
struct mem_encode{
char* buf;
size_t size;
size_t mem;
};
const int mn=41;int abba=0;
static void my_png_write_data(png_structp png_ptr,png_bytep data,png_size_t length){
struct mem_encode* p=(struct mem_encode*) png_get_io_ptr(png_ptr);

if(!p->buf){
p->buf=(char*)malloc(sizeof(p->buf)*mn);
if(!p->buf){png_error(png_ptr,"malloc png error");}
p->mem=mn;
}
	if(p->size +length > p->mem){
	char *new_png=(char*)realloc(p->buf,sizeof(char)*p->size + length);
		if(!new_png) png_error(png_ptr,"realloc png error");
		p->buf=new_png;
		p->mem+=length;
	}
	memcpy(p->buf + p->size,data,length);
	p->size+=length;
	//memcpy(fl->fuck,data,length);
	//fprintf(stderr,"p->buf: %s\n",p->buf);
	//fprintf(stderr,"p->size: %zu\n",p->size);
	
}
#if HAVE_PNG
static void fillRow(unsigned char *row, int num, const unsigned char color[])
{
	int i;

	for(i = 0; i < num; i++) {
		memcpy(row, color, 4);
		row += 4;
	}
}
#endif

struct mem_encode  writePNG(const QRcode *qrcode, const char *outfile, enum imageType type)
{
	//printf("OUTFILE: %s\n",outfile);
#if HAVE_PNG
	struct mem_encode state;
	
	state.buf=NULL;
	state.size=0;
	state.mem=0;
	
	static FILE *fp; // avoid clobbering by setjmp.
	png_structp png_ptr;
	png_infop info_ptr;
	png_colorp palette = NULL;
	png_byte alpha_values[2];
	unsigned char *row, *p, *q;
	int x, y, xx, yy, bit;
	int realwidth;

	realwidth = (qrcode->width + margin * 2) * size;
	if(type == PNG_TYPE) {
		row = (unsigned char *)malloc((size_t)((realwidth + 7) / 8));
	} else if(type == PNG32_TYPE) {
		row = (unsigned char *)malloc((size_t)realwidth * 4);
	} else {
		fprintf(stderr, "Internal error.\n");
		exit(EXIT_FAILURE);
	}
	if(row == NULL) {
		fprintf(stderr, "Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	if(outfile[0] == '-' && outfile[1] == '\0') {
		fp = stdout;
		
	} else {
		fp = fopen(outfile, "wb");
		if(fp == NULL) {
			fprintf(stderr, "Failed to create file: %s\n", outfile);
			perror(NULL);
			exit(EXIT_FAILURE);
		}
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(png_ptr == NULL) {
		fprintf(stderr, "Failed to initialize PNG writer.\n");
		exit(EXIT_FAILURE);
	}

	info_ptr = png_create_info_struct(png_ptr);
	if(info_ptr == NULL) {
		fprintf(stderr, "Failed to initialize PNG write.\n");
		exit(EXIT_FAILURE);
	}

	if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fprintf(stderr, "Failed to write PNG image.\n");
		exit(EXIT_FAILURE);
	}

	if(type == PNG_TYPE) {
		palette = (png_colorp) malloc(sizeof(png_color) * 2);
		if(palette == NULL) {
			fprintf(stderr, "Failed to allocate memory.\n");
			exit(EXIT_FAILURE);
		}
		palette[0].red   = fg_color[0];
		palette[0].green = fg_color[1];
		palette[0].blue  = fg_color[2];
		palette[1].red   = bg_color[0];
		palette[1].green = bg_color[1];
		palette[1].blue  = bg_color[2];
		alpha_values[0] = fg_color[3];
		alpha_values[1] = bg_color[3];
		png_set_PLTE(png_ptr, info_ptr, palette, 2);
		png_set_tRNS(png_ptr, info_ptr, alpha_values, 2, NULL);
	}

	//png_init_io(png_ptr, fp);
	png_set_write_fn(png_ptr,&state, my_png_write_data, NULL);
	if(type == PNG_TYPE) {
		printf("PNG-tyPe 55\n");
		png_set_IHDR(png_ptr, info_ptr,
				(unsigned int)realwidth, (unsigned int)realwidth,
				1,
				PNG_COLOR_TYPE_PALETTE,
				PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_DEFAULT,
				PNG_FILTER_TYPE_DEFAULT);
	} else {
		png_set_IHDR(png_ptr, info_ptr,
				(unsigned int)realwidth, (unsigned int)realwidth,
				8,
				PNG_COLOR_TYPE_RGB_ALPHA,
				PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_DEFAULT,
				PNG_FILTER_TYPE_DEFAULT);
	}
	png_set_pHYs(png_ptr, info_ptr,
			dpi * INCHES_PER_METER,
			dpi * INCHES_PER_METER,
			PNG_RESOLUTION_METER);
	png_write_info(png_ptr, info_ptr);

	if(type == PNG_TYPE) {
	/* top margin */
		memset(row, 0xff, (size_t)((realwidth + 7) / 8));
		for(y = 0; y < margin * size; y++) {
			png_write_row(png_ptr, row);
		}

		/* data */
		p = qrcode->data;
		for(y = 0; y < qrcode->width; y++) {
			memset(row, 0xff, (size_t)((realwidth + 7) / 8));
			q = row;
			q += margin * size / 8;
			bit = 7 - (margin * size % 8);
			for(x = 0; x < qrcode->width; x++) {
				for(xx = 0; xx < size; xx++) {
					*q ^= (*p & 1) << bit;
					bit--;
					if(bit < 0) {
						q++;
						bit = 7;
					}
				}
				p++;
			}
			for(yy = 0; yy < size; yy++) {
				png_write_row(png_ptr, row);
			}
		}
		/* bottom margin */
		memset(row, 0xff, (size_t)((realwidth + 7) / 8));
		for(y = 0; y < margin * size; y++) {
			png_write_row(png_ptr, row);
		}
	} else {
	/* top margin */
		fillRow(row, realwidth, bg_color);
		for(y = 0; y < margin * size; y++) {
			png_write_row(png_ptr, row);
		}

		/* data */
		p = qrcode->data;
		for(y = 0; y < qrcode->width; y++) {
			fillRow(row, realwidth, bg_color);
			for(x = 0; x < qrcode->width; x++) {
				for(xx = 0; xx < size; xx++) {
					if(*p & 1) {
						memcpy(&row[((margin + x) * size + xx) * 4], fg_color, 4);
					}
				}
				p++;
			}
			for(yy = 0; yy < size; yy++) {
				png_write_row(png_ptr, row);
			}
		}
		/* bottom margin */
		fillRow(row, realwidth, bg_color);
		for(y = 0; y < margin * size; y++) {
			png_write_row(png_ptr, row);
		}
	}

	png_write_end(png_ptr, info_ptr);
	
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	free(row);
	free(palette);
	abba=1;
	fprintf(stderr,"STATE.SIZE %zu\n",state.size);
	fprintf(stderr,"STATE.BUF: %s\n",state.buf);
return state;
	

#else
	fputs("\n\nPNG output is disabled at compile time. No output generated.\n", stderr);
	return 0;
#endif
}
//static 
	QRcode *encode(const unsigned char *intext, int length)
{
	eightbit=1;
	printf("intext: %s\n",intext);
	QRcode *code;

	if(micro) {
		printf("MICRO?\n");
		if(eightbit) {
			
			code = QRcode_encodeDataMQR(length, intext, version, level);
		} else {
			code = QRcode_encodeStringMQR((char *)intext, version, level, hint, casesensitive);
		}
	} else if(eightbit) {
		printf("EIGHTBIT\n");
		code = QRcode_encodeData(length, intext, version, level);
	} else {
		printf("ENCODESTRING???\n");
		code = QRcode_encodeString((char *)intext, version, level, hint, casesensitive);
	}

	return code;
}
