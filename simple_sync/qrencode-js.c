#include <node_api.h>
#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <getopt.h>
#include <errno.h>
#include <assert.h>
#if HAVE_PNG
#include <png.h>
#endif

#include "qrencode.h"

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
struct mem_encode{
 char* buf;
size_t size;
size_t mem;

};
//static 
const int mn=41;//512;
static enum imageType image_type = PNG_TYPE;
//struct mem_encode pi;
/*void mfu(png_flush_ptr d){
	//fprintf(stderr,"FUCK!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! zu\n");
			
			//return png_ptr
}*/
int ct=0;
static void my_png_write_data(png_structp png_ptr,png_bytep data,png_size_t length){
	ct++;
struct mem_encode* p=(struct mem_encode*) png_get_io_ptr(png_ptr);
	//p->buf[1024];
if(!p->buf){
p->buf=(char*)malloc(sizeof*p->buf*mn);
if(!p->buf){png_error(png_ptr,"malloc png error");}
p->mem=mn;
}
	if(p->size +length > p->mem){
		//fprintf(stderr,"USING REALLOC\n");
	char *new_png=(char*)realloc(p->buf,sizeof(char)*p->size + length);
		if(!new_png) png_error(png_ptr,"realloc png error");
		p->buf=new_png;
		p->mem+=length;
	}
	//fprintf(stderr,"data length:  %zu\n",length);
	//fprintf(stderr,"p->buf p->size: %zu\n",p->size);
	//fprintf(stderr,"ct: %d\n",ct);
	memcpy(p->buf + p->size,data,length);
	p->size+=length;
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

/*static int*/
int suka=0;
struct mem_encode writePNG(const QRcode *qrcode, const char *outfile, enum imageType type)
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
		//printf("PNG-tyPe 55\n");
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
//printf("\n\neND of PNG wrITe\n");
//	printf("Buffer: %s\n",state.buf);
	fclose(fp);
	//if(state.buf) free(state.buf);
	free(row);
	free(palette);
	//free(state.buf);
	//fprintf(stderr,"buffer: %s - %zu\n",state.buf,state.size);
	suka=1;
return state;
	//return 0;
#else
	fputs("\n\nPNG output is disabled at compile time. No output generated.\n", stderr);
	return 0;
#endif
}

static QRcode *encode(const unsigned char *intext, int length)
{
	eightbit=1;
	QRcode *code;

	if(micro) {
		//printf("MICRO?\n");
		if(eightbit) {
			code = QRcode_encodeDataMQR(length, intext, version, level);
		} else {
			code = QRcode_encodeStringMQR((char *)intext, version, level, hint, casesensitive);
		}
	} else if(eightbit) {
		//printf("EIGHTBIT? 1016\n");
		code = QRcode_encodeData(length, intext, version, level);
	} else {
		//printf("ENCODESTRING???\n");
		code = QRcode_encodeString((char *)intext, version, level, hint, casesensitive);
	}

	return code;
}

//static void 
static struct mem_encode qrencode(const unsigned char *intext, int length, const char *outfile)
{
	QRcode *qrcode;
//fprintf(stderr,"INTEXT: %s\n",(char*)intext);
	qrcode = encode(intext, length);
	if(qrcode == NULL) {
		if(errno == ERANGE) {
			fprintf(stderr, "Failed to encode the input data: Input data too large\n");
		} else {
			perror("Failed to encode the input data");
		}
		exit(EXIT_FAILURE);
	}

	struct mem_encode state;

	//fprintf(stderr,"KIRKOROV image_type: %d %d\n",image_type,qrcode->version);

	switch(image_type) {
		case PNG_TYPE:
		case PNG32_TYPE:
			state=writePNG(qrcode, outfile, image_type);
			//fprintf(stderr,"zu %zu\n",p.size);
			//fprintf(stderr,"Buffer_2: %s\n",p.buf);
			//fprintf(stderr,"Suka: %d\n",suka);
			//sleep(1);
			//p.mem=0;
			//free(p->buf);
			//if(p.buf){free(p.buf);
					 // p.buf=NULL;
					//  fprintf(stderr,"buffer freed.\n");}else{fprintf(stderr,"buffer is not freed.\n");}
			//fprintf(stderr,"Buffer_3: %s\n",p.buf);
			//p.buf=NULL;
			//p.size=0;
			//p.mem=0;
			//fprintf(stderr,"zu aft %zu\n",p.size);
			break;
		default:
			fprintf(stderr, "Unknown image type.\n");
			exit(EXIT_FAILURE);
	}
/*
if(p.buf){free(p.buf);
		  //p.buf=NULL;
		  p.mem=0;p.size=0;
		  fprintf(stderr,"\np.buf is freed.\n");}else{fprintf(stderr,"p.buf is undefined.\n");}*/
	//printf("BEFORE QRCODE FREE\n");
	//free(state.buf);
	QRcode_free(qrcode);
	//if(p.buf)
		//free(p.buf);p.buf[p.size]='0';
	//=NULL;
	//p.mem=0;p.size=0;
	//fprintf(stderr,"bufer before free: %s\n",p.buf);
	//free(state.buf);
	//state.buf=NULL;
	//p.mem=0;p.size=0;
	//fprintf(stderr,"suka after free: %d\n",suka);
	//fprintf(stderr,"bufer after free - and size -and mem: %s - %zu -%zu\n",p.buf,p.size,p.mem);
	return state;
}

/*
�PNG

IHDRWWKK/PLTE����ٟ�tRNS��ȵ��	pHYs
                                            
                                             ��~��IDAT8��ӱ�0
                                                               П���|�5�e%�
                                                                          0�;��]0�.��
#���+|��e��pB2�U<36�UG�u��e��{wg���P���O�-kNG&E�Q�F5�����qS�s�C����+sB�vQ͜��U��`34K�2���yi�/
�X�����qj�7S4S���X�Z����X���Z�_���T���QIEND�B`�
*/
//struct mem_encode state;
//void du(){struct mem_encode p;free(p.buf);}
napi_value qrencodeBuffer(napi_env env,napi_callback_info info){
	struct mem_encode state;
	//napi_handle_scope scope;
napi_value theBuffer;
napi_status status;
size_t argc=1;
	napi_value args[1];
	//napi_open_handle_scope(env,&scope);
	status=napi_get_cb_info(env,info,&argc,args,NULL,NULL);
	assert(status==napi_ok);
	napi_value myBuffer=args[0];
	

	const unsigned char*bufferdata;
	size_t bufferlength;
	status=napi_get_buffer_info(env,myBuffer,(void**)(&bufferdata),&bufferlength);
	assert(status==napi_ok);
	//qrencode((const unsigned char*)"mama",4,"-");
	//printf("input buffer length %zu \n",bufferlength);
	state=qrencode(bufferdata,bufferlength,"-");
	//printf("some data %s %zu\n",state.buf,state.size);
	//fprintf(stderr,"Suka la: %zu\n",state.size);
	status=napi_create_buffer_copy(env,state.size,state.buf,NULL,&theBuffer);
	
	assert(status==napi_ok);
	suka=2;
	//fprintf(stderr,"suka after: %d\n",suka);
	
	//pf(state);
	//napi_close_handle_scope(env,scope);
	//fprintf(stderr,"size: %zu - %zu\n",state.size,state.mem);
	free(state.buf);
	state.buf=NULL;
	state.mem=0;state.size=0;
	//fprintf(stderr,"buffer: %s = %zu = %zu\n",state.buf,state.mem,state.size);
	return theBuffer;
}
napi_value Init(napi_env env,napi_value exports){
napi_status status;
napi_property_descriptor methods[]={
	{"qrencodeBuffer",0, qrencodeBuffer,0,0,0, napi_default,0}
};
	status=napi_define_properties(env,exports,sizeof(methods)/sizeof(methods[0]),methods);
	assert(status==napi_ok);
	return exports;
}
NAPI_MODULE(addon,Init)