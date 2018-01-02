#include <assert.h>

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <getopt.h>
#include <errno.h>
#if HAVE_PNG
#include <png.h>
#endif
#include <node_api.h>
#include "qrencode.h"
#if defined _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#define MAX_CANCEL_THREADS 6
#define nullptr ((void*)0)

typedef struct{
	int32_t _input;
	int32_t _output;
	napi_ref _callback;
	napi_async_work _request;
} carrier;
carrier the_carrier;
carrier async_carrier[MAX_CANCEL_THREADS];

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
static const int mn=41;
static enum imageType image_type = PNG_TYPE;
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
}
struct mem_encode writePNG(const QRcode *qrcode, const char *outfile, enum imageType type)
{
	printf("OUTFILE: %s\n",outfile);
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
//printf("\n\neND of PNG wrITe\n");
//	printf("Buffer: %s\n",state.buf);
	fclose(fp);
	//if(state.buf) free(state.buf);
	free(row);
	free(palette);
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
		printf("MICRO?\n");
		if(eightbit) {
			code = QRcode_encodeDataMQR(length, intext, version, level);
		} else {
			code = QRcode_encodeStringMQR((char *)intext, version, level, hint, casesensitive);
		}
	} else if(eightbit) {
		printf("EIGHTBIT? 1016\n");
		code = QRcode_encodeData(length, intext, version, level);
	} else {
		printf("ENCODESTRING???\n");
		code = QRcode_encodeString((char *)intext, version, level, hint, casesensitive);
	}

	return code;
}

static void qrencode(const unsigned char *intext, int length, const char *outfile)
{
	QRcode *qrcode;
printf("INTEXT: %s\n",intext);
	qrcode = encode(intext, length);
	if(qrcode == NULL) {
		if(errno == ERANGE) {
			fprintf(stderr, "Failed to encode the input data: Input data too large\n");
		} else {
			perror("Failed to encode the input data");
		}
		exit(EXIT_FAILURE);
	}

	struct mem_encode p;

	printf("image_type: %d%d\n",image_type,qrcode->version);

	switch(image_type) {
		case PNG_TYPE:
		case PNG32_TYPE:
			p=writePNG(qrcode, outfile, image_type);
			break;
		default:
			fprintf(stderr, "Unknown image type.\n");
			exit(EXIT_FAILURE);
	}

if(p.buf){free(p.buf);fprintf(stderr,"\np.buf is freed.\n");}else{fprintf(stderr,"p.buf is undefined.\n");}
	QRcode_free(qrcode);
}


// node-gyp configure && build // rebuild
void Execute(napi_env env,void* data){
#if defined _WIN32
	Sleep(1000);
	#else
	//sleep(1);
	#endif
	carrier* c=(carrier*)data;
	if(c !=&the_carrier){
	napi_throw_type_error(env,nullptr,"wrong data parameter to Execute");
		return;
	}
	c->_output=c->_input*2;
}

void Complete(napi_env env,napi_status status, void* data){
carrier* c=(carrier*)data;
	if(c !=&the_carrier){
	napi_throw_type_error(env,nullptr,"wrong data parameter to Complete");
		return;
	}
	if(status !=napi_ok){
	napi_throw_type_error(env,nullptr,"execute callback failed.");
		return;
	}
	napi_value argv[2];
	napi_get_null(env,&argv[0]);
	napi_create_int32(env,c->_output,&argv[1]);
	
	napi_value callback;
	napi_get_reference_value(env,c->_callback,&callback);
	napi_value global;
	napi_get_global(env,&global);
	napi_value result;
	napi_call_function(env,global,callback,2,argv,&result);
	napi_delete_reference(env,c->_callback);
	napi_delete_async_work(env,c->_request);
}

napi_value Test(napi_env env,napi_callback_info info){
size_t argc=3;
	napi_status status;
	napi_value argv[3];
	napi_value _this;
	napi_value resource_name;
	void* data;
	status=napi_get_cb_info(env,info,&argc,argv,&_this,&data);
	assert(status==napi_ok);
	if(argc >3){napi_throw_type_error(env,nullptr,"not enough arguments, expected 3?.");
				return nullptr;
				}
	napi_valuetype t;
	status=napi_typeof(env,argv[0],&t);
	assert(status==napi_ok);
	if(t !=napi_number){
	napi_throw_type_error(env,nullptr,"Wrong first argument, integer expected.");return nullptr;
	}
	
	status=napi_typeof(env,argv[1],&t);
	assert(status==napi_ok);
	if(t !=napi_object){
	napi_throw_type_error(env,nullptr, "wrong second argument, object expected.");
		return nullptr;
	}
		status=napi_typeof(env,argv[2],&t);
	assert(status==napi_ok);
	if(t !=napi_function){
		napi_throw_type_error(env,nullptr,"wrong third argument, function expected.");
		return nullptr;
	}	
	the_carrier._output=0;
	status=napi_get_value_int32(env,argv[0],&the_carrier._input);
	assert(status==napi_ok);
	
	
	status=napi_create_reference(env,argv[2],1,&the_carrier._callback);
	assert(status==napi_ok);
	status=napi_create_string_utf8(env,"TestResource",NAPI_AUTO_LENGTH,&resource_name);
	assert(status==napi_ok);
	status=napi_create_async_work(env,argv[1],resource_name,Execute,Complete,&the_carrier,&the_carrier._request);
	assert(status==napi_ok);
	status=napi_queue_async_work(env,the_carrier._request);
	assert(status==napi_ok);
	return nullptr;
	}


napi_value Init(napi_env env,napi_value exports){
	napi_status status;
napi_property_descriptor properties[]={
	{"Test",0,Test,0,0,0,napi_default,0}
};
status=napi_define_properties(env,exports,sizeof(properties)/sizeof(*properties),properties);
assert(status==napi_ok);
return exports;
}
NAPI_MODULE(addon,Init)