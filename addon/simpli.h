#ifndef SIMPLI_H
#define SIMPLI_H
#include "qrencode.h"
#ifdef __cplusplus
extern "C" {
#endif
	/*
enum imageType {
PNG_TYPE,
PNG32_TYPE
};
*/
	/*
struct mem_encode{
char* buf;
size_t size;
size_t mem;
};
*/
//static 
	QRcode *encode(const unsigned char *intext, int length);
//static 
	struct mem_encode  writePNG(const QRcode *qrcode, const char *outfile, enum imageType type);
	
#ifdef __cplusplus
}
#endif
#endif