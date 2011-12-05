/* 
	CInflator.cpp

	Author:			Tom Naughton
	Description:	<describe the CInflator class here>
*/

#include "utilities.h"
#include "CInflator.h"
#include <iostream.h>
#include <string.h>
#include "CPakRatApp.h"

#define CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        dprintf("%s error: %d\n", msg, err); \
        return;\
    } \
}

#  define TESTFILE "foo.gz"

extern "C" {

void test_compress      OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
void test_gzio          OF((const char *out, const char *in, 
		            Byte *uncompr, int uncomprLen));
void test_deflate       OF((Byte *compr, uLong comprLen));
void test_inflate       OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
void test_large_deflate OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
void test_large_inflate OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
void test_flush         OF((Byte *compr, uLong *comprLen));
void test_sync          OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
void test_dict_deflate  OF((Byte *compr, uLong comprLen));
void test_dict_inflate  OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
}

CInflator::CInflator()
{
    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;
    d_stream.total_in = 0;
    d_stream.total_out = 0; 

	int result = inflateInit2(&d_stream, -MAX_WBITS);
	ReportError(result);
}

void CInflator::Inflate(Byte *compr, uLong comprLen, Byte *uncompr, uLong uncomprLen)
{	
    d_stream.next_in  = compr;
    d_stream.avail_in = comprLen;
    d_stream.next_out = uncompr;
    d_stream.avail_out = uncomprLen; 
	
	int result = inflate(&d_stream, Z_PARTIAL_FLUSH);
	ReportError(result);
    inflateReset(&d_stream);
}



void CInflator::ReportError(int result) 
{

	switch (result) {
		case Z_STREAM_END:
			break;
		case Z_OK:
			break;
		case Z_NEED_DICT:
			dprintf("Z_NEED_DIC %s\n", d_stream.msg);
			ThrowOSErr_(memFullErr);
			break;
		case Z_STREAM_ERROR:
			gApplication->MemoryIsLow();
			dprintf("Z_STREAM_ERROR %s\n", d_stream.msg);
			ThrowOSErr_(memFullErr);
			break;
		case Z_BUF_ERROR:
			gApplication->MemoryIsLow();
			dprintf("Z_BUF_ERROR %s\n", d_stream.msg);
			ThrowOSErr_(memFullErr);
			break;
		case Z_DATA_ERROR:
			dprintf("Z_DATA_ERROR %s\n", d_stream.msg);
			ThrowOSErr_(CantDecompress);
			break;
		case Z_MEM_ERROR:
			gApplication->MemoryIsLow();
			dprintf("Z_MEM_ERROR %s\n", d_stream.msg);
			ThrowOSErr_(memFullErr);
			break;
		case Z_ERRNO:
			dprintf("Z_ERRNO %s\n", d_stream.msg);
			ThrowOSErr_(CantDecompress);
			break;
		case Z_VERSION_ERROR:
			dprintf("Z_VERSION_ERROR %s\n", d_stream.msg);
			ThrowOSErr_(CantDecompress);
			break;
		default:
			dprintf("Z_SOME_OTHER_ERROR %s\n", d_stream.msg);
			ThrowOSErr_(CantDecompress);
			break;
    }
}

CInflator::~CInflator()
{
    int result = inflateEnd(&d_stream);
    ReportError(result);
}
