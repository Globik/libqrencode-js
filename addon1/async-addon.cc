/*
*   An example of an asynchronous C++ node addon.
*   Provided by paulhauner https://github.com/paulhauner
*   License: MIT
*   Tested in node.js v4.4.2 LTS in Ubuntu Linux
*/
#include <node.h>
#include <uv.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <node_buffer.h>
//#include "simpli.h"
#define MAX_CANCEL_THREADS 6
//define nullptr ((void*)0)
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

typedef struct{
const unsigned char* _input;
char* _output;
size_t  _bufferlength;
size_t _out_bufsize;
}carrier;
enum imageType {
	PNG_TYPE,
	PNG32_TYPE
};

static enum imageType image_type = PNG_TYPE;
using namespace std;
//struct mem_encode p;

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
static QRcode *encode(const unsigned char *intext, int length)
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
			//state=writePNG(qrcode, outfile, image_type);
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
			//p.size=
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

namespace asyncAddon {
  using v8::Function;
  using v8::FunctionCallbackInfo;
  using v8::Isolate;
  using v8::Local;
  using v8::Object;
  using v8::String;
  using v8::Value;
  using v8::Persistent;
using v8::Null;
	using v8::Integer;
	using v8::TryCatch;

  /**
  * Work structure is be used to pass the callback function and data 
  * from the initiating function to the function which triggers the callback.
  */
  struct async_req{
    uv_work_t  req;
    Persistent<Function> callback;
	  Isolate* isolate;
	  
int input;    
	char* result2;
	  size_t len;
  };
  
  /**
  * WorkAsync function is the "middle" function which does the work.
  * After the WorkAsync function is called, the WorkAsyncComplete function
  * is called.
  */
	//int labuda=1;
  static void WorkAsync(uv_work_t *r) {
	 // printf("LABUDAAAAAAAAAAAAAAAAAAAAAAAAA\n");
    async_req *req = reinterpret_cast<async_req *>(r->data);
	  // qrencode(const unsigned char *intext, int length, const char *outfile)
	 struct mem_encode state=qrencode((const unsigned char*)"mama",4,"-");
	 req->result2=(char*)malloc(sizeof(req->result2)*state.size);
	if(req->result2==NULL){fprintf(stderr,"some malloc error\n");}
	
     memcpy(req->result2,state.buf,state.size);
	 // req->output = req->input*2;
	 // fprintf(stderr,"sdata %s\n",req->result2);
	 req->len=state.size;
	free(state.buf);state.mem=0;state.size=0;
	//free(req->result2);
  }
  
  /**
  * WorkAsyncComplete function is called once we are ready to trigger the callback
  * function in JS.
  */
  //static 
	template <bool use_makecallback>
	static  void WorkAsyncComplete(uv_work_t *r)
  {
	  async_req* req = reinterpret_cast<async_req*>(r->data);
    Isolate* isolate =req->isolate;// Isolate::GetCurrent();

    v8::HandleScope scope(isolate);
	//Local<Value> argv[2]={Null(isolate),Integer::New(isolate,req->output)};
	  Local<Value> argv[2]={Null(isolate),node::Buffer::New(isolate,req->result2,req->len/*,nullptr,nullptr*/).ToLocalChecked()};
	  TryCatch try_catch(isolate);
	  Local<Object> global=isolate->GetCurrentContext()->Global();
	  //Local<Function> callback=Local<Function>::New(isolate,req->callback);
	  
	// Local<Value> argv[2]={Null(isolate),node::Buffer::New(isolate,(char*)req->result2,req->len,nullptr,nullptr).ToLocalChecked()};
    // https://stackoverflow.com/questions/13826803/calling-javascript-function-from-a-c-callback-in-v8/28554065#28554065
    //Local<Function>::New(isolate, work->callback)->Call(isolate->GetCurrentContext()->Global(), 1, argv);
     Local<Function> callback=Local<Function>::New(isolate,req->callback);
if(use_makecallback){
Local<Value> ret=node::MakeCallback(isolate,global,callback,2,argv);
	assert(!ret.IsEmpty());

}else{callback->Call(global,2,argv);}
    
    
   // const char *result =(char*) work->result;//.c_str();
   // Local<Value> argv[1] = { String::NewFromUtf8(isolate, result) };
  //  Local<Value> argv[1]={node::Buffer::New(isolate,(char*)work->result2,work->len,nullptr,nullptr).ToLocalChecked()};
    // https://stackoverflow.com/questions/13826803/calling-javascript-function-from-a-c-callback-in-v8/28554065#28554065
   // Local<Function>::New(isolate, work->callback)->Call(isolate->GetCurrentContext()->Global(), 1, argv);
    
    req->callback.Reset();
    delete req;
	  if(try_catch.HasCaught()){node::FatalException(isolate,try_catch);}
  }
  
  /**
  * DoTaskAsync is the initial function called from JS. This function returns
  * immediately, however starts a uv task which later calls the callback function
  */
	template <bool use_makecallback>
  void DoTaskAsync(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    
    async_req * req = new async_req;
	
    req->req.data = req;
	req->input=args[0]->IntegerValue();
	//req->output=0;
	req->result2=nullptr;
	req->isolate=isolate;
    
    // args[0] is where we pick the callback function out of the JS function params.
    // Because we chose args[0], we must supply the callback fn as the first parameter
    Local<Function> callback = Local<Function>::Cast(args[1]);
    req->callback.Reset(isolate, callback);
    
    uv_queue_work(node::GetCurrentEventLoop(isolate), &req->req, WorkAsync,(uv_after_work_cb) WorkAsyncComplete<use_makecallback>);
    
  // args.GetReturnValue().Set(Undefined(isolate));  
  }
  
  
  /**
  * init function declares what we will make visible to node
  */
  void init(Local<Object> exports,Local<Object> module) {
    NODE_SET_METHOD(exports, "runCall", DoTaskAsync<false>);
	NODE_SET_METHOD(exports,"runMakeCallback",DoTaskAsync<true>);
  }

  NODE_MODULE(asyncAddon, init)

}  
