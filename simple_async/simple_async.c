#include <assert.h>
// SYMPL_ASYNC FOLDER
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
#include <node_api.h>
#include "qrencode.h"
#if defined _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
//#define MAX_CANCEL_THREADS 6
//#define nullptr ((void*)0)

 typedef struct{
	const unsigned char* _input;
	char* _output;
	size_t  _bufferlength;
	size_t _out_bufsize;
    napi_ref _callback;
	napi_async_work _request;
}carrier;

//carrier async_carrier[MAX_CANCEL_THREADS];

#define INCHES_PER_METER (100.0/2.54)

static int dummy_Int=10;

static int casesensitive = 1;

static int eightbit = 1;//0; harcoded to yes
static int version = 0;
static int size = 3;//dot_size=number
static int margin = -1;// margin=number
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
int abba=0;
const int mn=41;
static uint32_t objectLength(napi_env,napi_value);
static const char* getString(napi_env, napi_value);
static bool isNumber(napi_env, napi_value);
static int32_t getZifra(napi_env, napi_value, const char*);
static bool isString(napi_env, napi_value);
static bool hasNamedProperty(napi_env, napi_value, const char*);
static napi_value getNamedProperty(napi_env, napi_value, const char*);

static enum imageType image_type = PNG_TYPE;
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

static int color_set(unsigned char color[4], const char *value)
{
	int len = strlen(value);
	int i, count;
	unsigned int col[4];
	if(len == 6) {
		count = sscanf(value, "%02x%02x%02x%n", &col[0], &col[1], &col[2], &len);
		if(count < 3 || len != 6) {
			return -1;
		}
		for(i = 0; i < 3; i++) {
			color[i] = col[i];
		}
		color[3] = 255;
	} else if(len == 8) {
		count = sscanf(value, "%02x%02x%02x%02x%n", &col[0], &col[1], &col[2], &col[3], &len);
		if(count < 4 || len != 8) {
			return -1;
		}
		for(i = 0; i < 4; i++) {
			color[i] = col[i];
		}
	} else {
		return -1;
	}
	return 0;
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
return state;
	

#else
	fputs("\n\nPNG output is disabled at compile time. No output generated.\n", stderr);
	return 0;
#endif
}
static QRcode *encode(const unsigned char *intext, int length)
{
	//eightbit=1;
	
	QRcode *code;

	if(micro) {
		printf("MICRO?\n");
		if(eightbit) {
			// version 3 or 4
			 //code=QRcode_encodeDataMQR(4,(unsigned char*)"mama\0",4,QR_ECLEVEL_M);
			fprintf(stderr,"FUCK!!!!!!!!!! intext %s, length: %zu, version: %d, level: %d\n",intext,length,version,level);
			code = QRcode_encodeDataMQR(length, intext, version, /*QR_ECLEVEL_M*/level);
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
static struct mem_encode qrencode(const unsigned char *intext, int length, const char *outfile)

{
	QRcode *qrcode;
//printf("INTEXT: %s\n",intext);
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

	printf("image_type: %d version: %d\n",image_type,qrcode->version);

	switch(image_type) {
		case PNG_TYPE:
		case PNG32_TYPE:
			p=writePNG(qrcode, outfile, image_type);
			break;
		default:
			fprintf(stderr, "Unknown image type.\n");
			exit(EXIT_FAILURE);
	}
	
	
//fprintf(stderr,"Some data: %s\n",p.buf);
//	fprintf(stderr,"p.size: %zu\n",p.size);

	QRcode_free(qrcode);
	return p;
}


int labuda=0;
void Execute(napi_env env,void* data){

 carrier* c=(carrier*)data;
	if(c==NULL){fprintf(stderr,"NULL!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");}
	//if(c !=&the_carrier){
	//napi_throw_type_error(env,nullptr,"wrong data parameter to Execute");
	//	return;
	//}
	fprintf(stderr,"DUMMY_INT IN Execute() UUUUUUUU*******************************************************: %d\n",dummy_Int);

	struct mem_encode p=qrencode(c->_input,c->_bufferlength,"-");
	
	c->_output=(char*)malloc(sizeof(c->_output)*p.size);
	if(c->_output==NULL){fprintf(stderr,"some malloc error\n");}
	
     memcpy(c->_output,p.buf,p.size);
	c->_out_bufsize=p.size;
	labuda=1;
	
	if(labuda==1){
		//fprintf(stderr,"length: %zu\n",p.size);
		free(p.buf);
		p.buf=NULL;
		p.size=0;
		
	
	//fprintf(stderr,"\nLABUDA IS 1!!!\n");
		labuda=0;
	}else{fprintf(stderr,"\nLABUDA IS 0\n");}

}

void Complete(napi_env env,napi_status status, void* data){
 carrier* c=(carrier*)data;
	
	//if(c !=&the_carrier){
	//napi_throw_type_error(env,nullptr,"wrong data parameter to Complete");
	//	return;
	//}
	
	if(status !=napi_ok){
	napi_throw_type_error(env,NULL,"execute callback failed.");
		return;
	}
//napi_handle_scope scope;
	napi_value argv[2];
	//status=napi_open_handle_scope(env,&scope);
	//assert(status==napi_ok);
	napi_get_null(env,&argv[0]);
	//fprintf(stderr,"\nDATA!!!!: %zu\n",c->_out_bufsize);
status=napi_create_buffer_copy(env,c->_out_bufsize,c->_output,NULL,&argv[1]);	
	assert(status==napi_ok);
	//if(c->_output){free(c->_output);}
	napi_value callback;
	napi_get_reference_value(env,c->_callback,&callback);
	napi_value global;
	status=napi_get_global(env,&global);
	assert(status==napi_ok);
	napi_value result;
	//fprintf(stderr,"Labuda vor napi_call_func: %d\n",labuda);
	status=napi_call_function(env,global,callback,2,argv,&result);
	assert(status==napi_ok);
	//fprintf(stderr,"Labuda vor del_ref: %d\n",labuda);
	status=napi_delete_reference(env,c->_callback);
	assert(status==napi_ok);
	status=napi_delete_async_work(env,c->_request);
	assert(status==napi_ok);
	//status=napi_close_handle_scope(env,scope);
	//assert(status==napi_ok);
	free(c->_output);
	free(c);
}

napi_value Test(napi_env env,napi_callback_info info){
size_t argc=3;
if(margin <= 0){if(micro){margin=2;}else{margin=4;}}
	napi_status status;
	napi_value argv[3];
	napi_value _this;
	napi_value resource_name;
	void* data;
	carrier* c=(carrier*)malloc(sizeof(carrier));
	//napi_handle_scope scope;
	//status=napi_open_handle_scope(env,&scope);
	//assert(status==napi_ok);
	status=napi_get_cb_info(env,info,&argc,argv,&_this,&data);
	assert(status==napi_ok);
	//if(argc >3){napi_throw_type_error(env,nullptr,"not enough arguments, expected 3?.");
	//			return nullptr;
	//			}
	//napi_valuetype t;
	//status=napi_typeof(env,argv[0],&t);
	//assert(status==napi_ok);
	//if(t !=napi_object){
	//napi_throw_type_error(env,nullptr,"Wrong first argument, obj or buf expected.");return nullptr;
	//}
	//status=napi_is_buffer(env,argv[0],&hasIstance)
	//assert(status==napi_ok);
	
	/*
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
	*/
	
	c->_output=NULL;

	status=napi_get_buffer_info(env, argv[0],(void**)(&c->_input),&c->_bufferlength);
	assert(status==napi_ok);
	
	
	status=napi_create_reference(env,argv[2],1,&c->_callback);
	assert(status==napi_ok);
	status=napi_create_string_utf8(env,"TestResource",NAPI_AUTO_LENGTH,&resource_name);
	assert(status==napi_ok);
	status=napi_create_async_work(env,argv[1],resource_name,Execute,Complete,c,&c->_request);
	assert(status==napi_ok);
	status=napi_queue_async_work(env,c->_request);
	assert(status==napi_ok);
	//napi_close_handle_scope(env,scope);
	return NULL;
	}

napi_value setOptions(napi_env env,napi_callback_info info){
size_t argc=1;
	napi_value args[1];
	napi_status status;
	status=napi_get_cb_info(env,info,&argc,args,NULL,NULL);
	assert(status==napi_ok);
	if(argc >1){
	status=napi_throw_type_error(env,NULL,"Wrong number of arguments! Must be 1.");
	assert(status==napi_ok);
	return NULL;}
	napi_valuetype vtype;
	status=napi_typeof(env,args[0],&vtype);
	assert(status==napi_ok);
	if(vtype != napi_object){
	status=napi_throw_type_error(env,NULL,"Wrong type of arguments! xpects an object as first argument.");
	assert(status==napi_ok);
	return NULL;
	}
	napi_value obj=args[0];
	uint32_t lind;
	napi_value props_names;
	status=napi_get_property_names(env,obj,&props_names);
	assert(status==napi_ok);
	lind=objectLength(env,props_names);
	fprintf(stderr,"ARR LENGTH: %d\n",lind);
	if(lind==0){
	napi_throw_type_error(env,NULL,"A provided object must not to be empty!");
	return NULL;}
	const char*margi="margin";
	bool has_p;
	has_p=hasNamedProperty(env,obj,margi);
	fprintf(stderr,"If OBJECT has_named_property %s? : %d\n",margi,has_p);
	if(has_p){fprintf(stderr,"YES! MARGIN is!\n");}else{
	fprintf(stderr,"No! MARGIN is NOT!\n");}//dummy_Int=10
	fprintf(stderr,"DUMMY_INT BEFORE: %d\n",dummy_Int);
	if(has_p){
	napi_value vresi;
	vresi=getNamedProperty(env,obj,margi);
	int32_t dummyTwo=getZifra(env,vresi," for margin.");
	if(dummyTwo < 0){
	status=napi_throw_type_error(env,NULL,"Invalid margin!");
	assert(status==napi_ok);
	return NULL;
	}
		
	margin=dummyTwo;
	fprintf(stderr,"margin: %d\n", margin);
	}else{
	if(margin < 0){
	if(micro){margin=2;}else{margin=4;}
	}
	}
	bool has_dot_size;
	const char*dotsizi="dot_size";
	has_dot_size=hasNamedProperty(env,obj,dotsizi);
	if(has_dot_size){
	napi_value nres;
	nres=getNamedProperty(env, obj, dotsizi);
	int32_t dotSize=getZifra(env, nres, " for dot_size.");
	if(dotSize <=0){
	status=napi_throw_type_error(env,NULL,"Invalid dot_size.");
	assert(status==napi_ok);
	return NULL;
	}
	size=dotSize;
	fprintf(stderr, "dot_size: %d\n", size);
	}
	bool has_micro;
	const char*micri="micro";
	has_micro=hasNamedProperty(env,obj,micri);
	if(has_micro){
	napi_value nres2;
	nres2=getNamedProperty(env, obj, micri);
	int32_t micron=getZifra(env, nres2, " for micro.");
	if(micron == 1){micro=micron;}else{micro=0;}
		
	/*if(micro){
	if(version==0){
	status=napi_throw_type_error(env,NULL,"Version must be specified to encode a Micro QR Cide symbol.");
	assert(status==napi_ok);
	return NULL;
	}
	}*/

	fprintf(stderr, "micro: %d\n", micro);
	}
	//version
	bool has_version;
	const char*versi="version";
	has_version=hasNamedProperty(env,obj,versi);
	if(has_version){
	napi_value nres3;
	nres3=getNamedProperty(env, obj, versi);
	int32_t verson=getZifra(env, nres3, " for version.");
	if(micro && verson > MQRSPEC_VERSION_MAX){
	status=napi_throw_type_error(env,NULL,"Version should be less or equal to %d, MQRSPEC_VERSION_MAX");
	assert(status==napi_ok);
	return NULL;
	}else if(!micro && verson > QRSPEC_VERSION_MAX){
	status=napi_throw_type_error(env,NULL,"Version should be less or equal to qrspec_version_max.");
	assert(status==napi_ok);
	return NULL;	
	}
	version=verson;
	if(micro){
	if(version==0){
	status=napi_throw_type_error(env,NULL,"Version must be specified to encode a Micro QR Cide symbol.");
	assert(status==napi_ok);
	return NULL;
	}
	}

    fprintf(stderr, "version: %d\n", version);
	}
	
	//level
	bool has_level;
	const char*leveli="level";
	has_level=hasNamedProperty(env,obj,leveli);
	if(has_level){
	napi_value nres4;
	nres4=getNamedProperty(env, obj, leveli);
	const char*lev;
	lev=getString(env,nres4);
	fprintf(stderr,"LEVEL: %s\n",lev);
	if(strcmp(lev,"h")==0  || strcmp(lev,"H")==0){
	fprintf(stderr,"HIGH LEVEL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	level=QR_ECLEVEL_H;
	}else if(strcmp(lev,"m")==0  || strcmp(lev,"M")==0){
		fprintf(stderr,"MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM: %d\n",QR_ECLEVEL_M);
	level=QR_ECLEVEL_M;
	}else if(strcmp(lev,"l")==0 || strcmp(lev,"L")==0){
	level=QR_ECLEVEL_L;
		fprintf(stderr,"LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL\n");
	}else if(strcmp(lev,"q")==0 || strcmp(lev,"Q")==0){
	level=QR_ECLEVEL_Q;
		fprintf(stderr,"QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ\n");
	}else{
	fprintf(stderr,"UNKNOWN LEVEL TYPE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	status=napi_throw_type_error(env,NULL,"Invalid level.");
	assert(status==napi_ok);
	return NULL;
	}
	}
	
	//colors
	bool has_b;const char*coli_b="white";
	const char*backi="background_color";
	has_b=hasNamedProperty(env,obj,backi);
	if(has_b){
	napi_value nres5;
	nres5=getNamedProperty(env, obj, backi);
	//char*coli_b;
	coli_b=getString(env,nres5);
	fprintf(stderr,"Color background: %s\n",coli_b);
	if(color_set(bg_color,coli_b)){
	status=napi_throw_type_error(env,NULL,"Invalid background color value.");
	assert(status==napi_ok);
	return NULL;
		}
	}
	
	// colors f
	
	bool has_f;
	const char*fori="foreground_color";const char*coli_f="Black";
	has_f=hasNamedProperty(env,obj,fori);
	if(has_f){
	napi_value nres6;
	nres6=getNamedProperty(env, obj, fori);
	//char*coli_f;
	coli_f=getString(env,nres6);
	fprintf(stderr,"Color foreground: %s\n",coli_f);
	if(color_set(fg_color,coli_f)){
	status=napi_throw_type_error(env,NULL,"Invalid foreground color value.");
	assert(status==napi_ok);
	return NULL;
		}
	}
	napi_value qr_version,qr_margin,qr_level, qr_size,qr_versi,qr_micro,qr_background_color,qr_foreground_color,qr_full_version;
	napi_value object;
	//background_color:'76eec6',foreground_color:'ff0000'
	status=napi_create_string_utf8(env,"Copyright (C) 2006-2017 Kentaro Fukuchi.",NAPI_AUTO_LENGTH,&qr_version);
	assert(status==napi_ok);
	status=napi_create_string_utf8(env,(const char*)coli_b,NAPI_AUTO_LENGTH,&qr_background_color);
	assert(status==napi_ok);
	status=napi_create_string_utf8(env,(const char*)coli_f,NAPI_AUTO_LENGTH,&qr_foreground_color);
	assert(status==napi_ok);
	status=napi_create_string_utf8(env,QRcode_APIVersionString(),NAPI_AUTO_LENGTH,&qr_full_version);
	assert(status==napi_ok);
	status=napi_create_int32(env,level,&qr_level);
	assert(status==napi_ok);
	status=napi_create_int32(env,margin,&qr_margin);
	assert(status==napi_ok);
	status=napi_create_int32(env,size,&qr_size);
	assert(status==napi_ok);
	status=napi_create_int32(env,version,&qr_versi);
	assert(status==napi_ok);
	status=napi_create_int32(env,micro,&qr_micro);
	assert(status==napi_ok);
	status=napi_create_object(env,&object);
	assert(status==napi_ok);
	status=napi_set_named_property(env,object,"full_version",qr_full_version);
	assert(status==napi_ok);
	status=napi_set_named_property(env,object,"copyright",qr_version);
	assert(status==napi_ok);
	status=napi_set_named_property(env,object,"margin",qr_margin);
	assert(status==napi_ok);
	status=napi_set_named_property(env,object,"level",qr_level);
	assert(status==napi_ok);
	
	status=napi_set_named_property(env,object,"size",qr_size);
	assert(status==napi_ok);
	
	status=napi_set_named_property(env,object,"micro",qr_micro);
	assert(status==napi_ok);
	status=napi_set_named_property(env,object,"version",qr_versi);
	assert(status==napi_ok);
	
	status=napi_set_named_property(env,object,"bg_color",qr_background_color);
	assert(status==napi_ok);
	
	status=napi_set_named_property(env,object,"fg_color",qr_foreground_color);
	assert(status==napi_ok);
	return object;
}

napi_value Init(napi_env env,napi_value exports){
	napi_status status;
napi_property_descriptor properties[]={
	{"Test",0,Test,0,0,0,napi_default,0},
	{"setOptions",0,setOptions,0,0,0,napi_default,0}
};
status=napi_define_properties(env,exports, sizeof(properties)/sizeof(*properties),properties);
assert(status==napi_ok);
return exports;
}
NAPI_MODULE(addon,Init)
	
	
uint32_t objectLength(napi_env env, napi_value prop_names){
napi_status status;
uint32_t lindex;
status=napi_get_array_length(env,prop_names,&lindex);
assert(status==napi_ok);
fprintf(stderr,"arr length: %d\n",lindex);
return lindex;
}
const char* getString(napi_env env,napi_value js_str){
char st[50];
size_t st_size=50;
napi_status status;
if(isString(env,js_str)){
status=napi_get_value_string_utf8(env,js_str,st,st_size,NULL);
assert(status==napi_ok);
	const char*du=st;
return du;
}
return NULL;
}
bool isNumber(napi_env env,napi_value nnumber){
napi_status status;
napi_valuetype t;
status=napi_typeof(env,nnumber,&t);
assert(status==napi_ok);
if(t == napi_number) return 1;
return 0;
}

int32_t getZifra(napi_env env,napi_value nresult,const char*er){
int32_t zfr;
char foo[100];
char*f="Expected a number";
napi_status status;
if(isNumber(env,nresult)){
status=napi_get_value_int32(env,nresult,&zfr);
assert(status==napi_ok);
return zfr;
}else{
strcpy(foo,f);
if(er) {
strcat(foo,er);
}
napi_throw_type_error(env,NULL,foo);
return zfr;
}}

bool isString(napi_env env,napi_value str){
napi_status status;
napi_valuetype vtype;
status=napi_typeof(env,str,&vtype);
assert(status==napi_ok);
if(vtype == napi_string) return 1;
return 0;
}

bool hasNamedProperty(napi_env env,napi_value obj,const char*s){
bool hasp;
napi_status status;
status=napi_has_named_property(env,obj,s,&hasp);//1=YES! 0=NO!
assert(status==napi_ok);
	if(status !=napi_ok) return 0;
return hasp;
}

napi_value getNamedProperty(napi_env env,napi_value obj,const char*s){
napi_value vresi;napi_status status;
status=napi_get_named_property(env,obj,s,&vresi);// displays the value of key "suka" to the vresi
assert(status==napi_ok);
return vresi;
}
