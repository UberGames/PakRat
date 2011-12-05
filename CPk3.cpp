/* 
	CPk3.cpp

	Author:			Tom Naughton
	Description:	<describe the CPk3 class here>
*/

#include "CPk3.h"
#include "utilities.h"
#include "CPakStream.h"
#include "CInflator.h"
#include "CPakRatApp.h"



typedef unsigned char byte;
typedef long longint;
typedef unsigned word;
typedef char boolean;
#define STRSIZ  256



void CPk3::get_string(Boolean print, int len, char *   s)
{ 
#pragma unused (print)
	SInt32 length = len;
	_fileStream->GetBytes(s, length);
   s[length] = 0;
//	if (print)
//		dprintf("%s\n", s);
} 


/* ---------------------------------------------------------- */ 

void CPk3::process_local_file_header(UInt32 marker, Boolean add, Boolean open, Boolean print)
{ 
#pragma unused (marker, add, open)
	char filename[STRSIZ];
	char extra[STRSIZ];
	local_file_header 	rec;
	local_file_header 	temp;

	//if (print)
	//	dprintf("---------process_local_file_header\n");

	SInt32 length = local_file_header_size-4;
	_fileStream->GetBytes(&temp, length);
	char *p = (char*)&temp;
	
	// this is necessary because the compiler optimizes the byte alignment
	// of structs, so I can't read the data directly into it.
	
	rec.version_needed_to_extract = swapShort(nextShort(p));
	rec.general_purpose_bit_flag = swapShort(nextShort(p));
	rec.compression_method = swapShort(nextShort(p));
	rec.last_mod_file_time = swapLong(nextLong(p));
	rec.crc32 = swapLong(nextLong(p));
	rec.compressed_size = swapLong(nextLong(p));
	rec.uncompressed_size = swapLong(nextLong(p));
	rec.filename_length = swapShort(nextShort(p));
	rec.extra_field_length = swapShort(nextShort(p));

	/*
	if (print) {
		dprintf("version_needed_to_extract 0x%x\n", (rec.version_needed_to_extract));
		dprintf("general_purpose_bit_flag 0x%x\n", (rec.general_purpose_bit_flag));
		dprintf("compression_method %d\n", (rec.compression_method));
		dprintf("last_mod_file_time 0x%lx\n", (rec.last_mod_file_time));
		dprintf("crc32 0x%lx\n", (rec.crc32));
		dprintf("compressed_size %ld\n", rec.compressed_size);
		dprintf("uncompressed_size %ld\n",rec.uncompressed_size);
		dprintf("filename_length %d\n", (rec.filename_length));
		dprintf("extra_field_length %d\n", (rec.extra_field_length));
	}
	*/

	get_string(print, rec.filename_length,filename);
	get_string(print, rec.extra_field_length,extra);
	
	_entryName = filename;
	_entryCompression = rec.compression_method;
    _entryExpandedSize = rec.uncompressed_size;
    _entryCompressedSize = rec.compressed_size;
    _entryCrc = rec.crc32;  		
    _entryDataOffset = _fileStream->GetMarker();	 
	
} 


/* ---------------------------------------------------------- */ 

void CPk3::process_central_file_header(UInt32 marker, Boolean add, Boolean open, Boolean print)
{ 
#pragma unused (open)
	//if (print)
	//	dprintf("---------process_central_file_header\n");

	central_directory_file_header rec; 
	central_directory_file_header temp; 
	char filename[STRSIZ];
	char extra[STRSIZ];
	char comment[STRSIZ];
  
  	SInt32 length = central_directory_file_header_size-4;
	_fileStream->GetBytes(&temp, length);
	char *p = (char*)&temp;

	rec.version_made_by = swapShort(nextShort(p));
	rec.version_needed_to_extract = swapShort(nextShort(p));
	rec.general_purpose_bit_flag = swapShort(nextShort(p));
	rec.compression_method = swapShort(nextShort(p));
	rec.last_mod_file_time = swapLong(nextLong(p));
	rec.crc32 = swapLong(nextLong(p));
	rec.compressed_size = swapLong(nextLong(p));
	rec.uncompressed_size = swapLong(nextLong(p));
	rec.filename_length = swapShort(nextShort(p));
	rec.extra_field_length = swapShort(nextShort(p));
	rec.file_comment_length = swapShort(nextShort(p));
	rec.disk_number_start = swapShort(nextShort(p));
	rec.internal_file_attributes = swapShort(nextShort(p));
	rec.external_file_attributes = swapLong(nextLong(p));
	rec.relative_offset_local_header = swapLong(nextLong(p));

	/*
	if (print) {
		dprintf("version_made_by 0x%d\n", (rec.version_made_by));
		dprintf("version_needed_to_extract 0x%d\n", (rec.version_needed_to_extract));
		dprintf("general_purpose_bit_flag 0x%d\n", (rec.general_purpose_bit_flag));
		dprintf("compression_method 0x%d\n", (rec.compression_method));
		dprintf("last_mod_file_time 0x%lx\n", (rec.last_mod_file_time));
		dprintf("crc32 0x%lx\n", (rec.crc32));
		dprintf("compressed_size %ld\n", rec.compressed_size);
		dprintf("uncompressed_size %ld\n",rec.uncompressed_size);
		dprintf("filename_length %d\n", (rec.filename_length));
		dprintf("extra_field_length %d\n", (rec.extra_field_length));
		dprintf("file_comment_length %d\n", (rec.file_comment_length));
		dprintf("disk_number_start %d\n", (rec.disk_number_start));
		dprintf("internal_file_attributes %d\n", (rec.internal_file_attributes));
		dprintf("external_file_attributes %ld\n", rec.external_file_attributes);
		dprintf("relative_offset_local_header %ld\n", rec.relative_offset_local_header); 
	}
	*/

	get_string(print, rec.filename_length,filename); 
	get_string(print, rec.extra_field_length,extra); 
	get_string(print, rec.file_comment_length,comment); 

	_entryName = filename;
	_entryCompression = rec.compression_method;
    _entryExpandedSize = rec.uncompressed_size;
    _entryCompressedSize = rec.compressed_size;
    _entryCrc = rec.crc32;  		
    _entryLocalHeaderOffset = rec.relative_offset_local_header;	 
	_compressionMethodsUsed |= 1 << _entryCompression;
	
	if (add)
		AddEntry(filename, marker);
} 


/* ---------------------------------------------------------- */ 

void  CPk3::process_end_central_dir(Boolean print)
{ 
	//if (print)
	//	dprintf("---------process_end_central_dir\n");

	end_central_dir_record rec; 
	end_central_dir_record temp; 
	char comment[STRSIZ];

	SInt32 length = end_central_dir_record_size-4;
	_fileStream->GetBytes(&temp, length);
	char *p = (char*)&temp;


	rec.number_this_disk = swapShort(nextShort(p));
	rec.first_disk = swapShort(nextShort(p));
	rec.total_entries_central_dir_this_disk = swapShort(nextShort(p));
	rec.total_entries_central_dir = swapShort(nextShort(p));
	rec.size_central_directory = swapLong(nextLong(p));
	rec.offset_start_central_directory = swapLong(nextLong(p));
	rec.zipfile_comment_length = swapShort(nextShort(p));

	/*
	if (print) {
		dprintf("number_this_disk 0x%d\n", (rec.number_this_disk));
		dprintf("first_disk 0x%d\n", (rec.first_disk));
		dprintf("total_entries_central_dir_this_disk 0x%d\n", (rec.total_entries_central_dir_this_disk));
		dprintf("total_entries_central_dir 0x%d\n", (rec.total_entries_central_dir));
		dprintf("size_central_directory 0x%ld\n", (rec.size_central_directory));
		dprintf("offset_start_central_directory 0x%d\n", (rec.offset_start_central_directory));
		dprintf("zipfile_comment_length 0x%d\n", (rec.zipfile_comment_length));
	}
	*/
	
    _directoryCentralHeaderLocation = _directoryEndHeaderLocation - rec.size_central_directory;	  	 

	_fileStream->SetMarker(_directoryCentralHeaderLocation, streamFrom_Start);	
	get_string(print, rec.zipfile_comment_length,comment); 
} 


/* ---------------------------------------------------------- */ 

Boolean CPk3::process_headers(UInt32 count, Boolean add, Boolean open, Boolean print)
{ 
   while (count--) { 
	   	UInt32 marker = _fileStream->GetMarker();
	 	UInt32 sig;
	 	SInt32 length = sizeof(sig);
	 	_fileStream->GetBytes(&sig,length);
		sig = swapLong(sig);
		
		if (sig == local_file_header_signature) {
			process_local_file_header(marker, add, open, print);
		} else   if (sig == central_file_header_signature) {
			process_central_file_header(marker, add, open, print);
			_headersCount += 1;
		} else if (sig == end_central_dir_signature)   { 
			return true;
		} else { 
			dprintf("Invalid Zipfile Header\n"); 
			return true;
		} 
	} 
	return false;
} 



UInt32 CPk3::FindEndHeader()
{
    char buf[end_central_dir_record_size * 2];
    SInt32 len, pos;

    // Get the length of the zip file 
    len = pos =	_fileStream->GetLength();
	_fileStream->SetMarker(pos, streamFrom_Start);
    

    /*
     * Search backwards end_central_dir_record_size bytes at a time from end of file stopping
     * when the END header has been found. We need to make sure that we
     * handle the case where the signature may straddle a record boundary.
     * Also, the END header must be located within the last 64k bytes of
     * the file since that is the maximum comment length.
     */
    memset(buf, 0, sizeof(buf));
    while (len - pos < 0xFFFF) {
		char *bp;
		/* Number of bytes to check in next block */
		SInt32 count = 0xFFFF - (len - pos);
		if (count > end_central_dir_record_size) {
		    count = end_central_dir_record_size;
		}
		/* Shift previous block */
		memcpy(buf + count, buf, count);
		/* Update position and read next block */
		pos -= count;
		_fileStream->SetMarker(pos, streamFrom_Start);
		_fileStream->GetBytes(buf, count);

		// scan for end header signature
		for (bp = (char*)buf; bp < buf + count; bp++) {
			char *p = bp;
		 	UInt32 sig = nextLong(p);
			sig = swapLong(sig);
		    if (sig == end_central_dir_signature) {
		    
				// verify its the end header
				end_central_dir_record rec; 
				rec.number_this_disk = swapShort(nextShort(p));
				rec.first_disk = swapShort(nextShort(p));
				rec.total_entries_central_dir_this_disk = swapShort(nextShort(p));
				rec.total_entries_central_dir = swapShort(nextShort(p));
				rec.size_central_directory = swapLong(nextLong(p));
				rec.offset_start_central_directory = swapLong(nextLong(p));
				rec.zipfile_comment_length = swapShort(nextShort(p));
				
				SInt32 endpos = pos + (bp - buf);
				SInt32 clen = rec.zipfile_comment_length;
							
				if (endpos + end_central_dir_record_size + clen == len) {
					_directoryEndHeaderLocation = endpos;
				    return _directoryEndHeaderLocation;
				}
		    }
		}
	}
    return 0; /* END header not found */
}

/* ---------------------------------------------------------- */ 



/* ---------------------------------------------------------- */
/*
 * main program
 *
 */ 



CPk3::CPk3(CFileArchive *inParent) : CFileArchive(inParent)
{
}

CPk3::CPk3(FSSpec inFileSpec) : CFileArchive(inFileSpec)
{
	_compressionMethodsUsed = 0;
}


CPk3::~CPk3()
{
	dprintf("~CPk3()\n");
	if (_fileStream)
		_fileStream->CloseDataFork();
}

Boolean CPk3::init()
{	
	Boolean     done = false;
	long nowTicks = ::TickCount();

	_fileStream->OpenDataFork(fsRdPerm);	
	_headersCount = 0;
	
	// find start of central directory and jump to it
	_fileStream->SetMarker(FindEndHeader() + 4, streamFrom_Start); // skip signature
	process_end_central_dir(true);
		
	// process central directory headers	
	while (!done)
		done = process_headers(100, true, false, false); // add, open, print
	
	/*
	dprintf("items: %ld time to open: %ld ticks\n", _headersCount, (::TickCount() - nowTicks));
	
	dprintf("compression methods used:\n");
	if (_compressionMethodsUsed & (1 << 0))
		dprintf("	0 - The file is stored (no compression)\n");
	if (_compressionMethodsUsed & (1 << 1))
		dprintf("	1 - The file is Shrunk\n");
	if (_compressionMethodsUsed & (1 << 2))
		dprintf("	2 - The file is Reduced with compression factor 1\n");
	if (_compressionMethodsUsed & (1 << 3))
		dprintf("	3 - The file is Reduced with compression factor 2\n");
	if (_compressionMethodsUsed & (1 << 4))
		dprintf("	4 - The file is Reduced with compression factor 3\n");
	if (_compressionMethodsUsed & (1 << 5))
		dprintf("	5 - The file is Reduced with compression factor 4\n");
	if (_compressionMethodsUsed & (1 << 6))
		dprintf("	6 - The file is Imploded\n");
	if (_compressionMethodsUsed & (1 << 7))
		dprintf("	7 - Reserved for Tokenizing compression algorithm\n");
	if (_compressionMethodsUsed & (1 << 8))
		dprintf("	8 - The file is Deflated\n");
	if (_compressionMethodsUsed & (1 << 9))
		dprintf("	9 - Reserved for enhanced Deflating\n");
	if (_compressionMethodsUsed & (1 << 10))
		dprintf("	10 - PKWARE Date Compression Library Imploding\n");
	*/

	return true;
}

void CPk3::AddEntry(const char *name, UInt32 offset)
{

	string package, file, extension;

	decomposeEntryName(name,  package, file, extension);
	if (file.length() > 0) {
		CFileArchive *thePackage = packageWithName(package.c_str(), true);
		thePackage->addFileWithName(offset, (file + "." + extension).c_str());
	}
}


// called only on the root pak
long CPk3::SizeOfItemAtIndex(const char *itemName, UInt32 offset)
{
#pragma unused (itemName)
	_fileStream->SetMarker(offset, streamFrom_Start);
	process_headers(1, false, false, false);
	return _entryExpandedSize;
}

// called only on the root pak
CPakStream *CPk3::LoadItemAtIndex(CFileArchive *inParent, const char *itemName, UInt32 offset)
{
	Handle pakData = 0;
	Handle compressedData = 0;
	CInflator *theInflator = 0;
	
//	dprintf("CPk3::LoadItemAtIndex %ld\n", offset);
	
	// central directory header loads offset for local file header
	_fileStream->SetMarker(offset, streamFrom_Start);
	process_headers(1, false, true, true);

	// local file header sets marker for file data and loads compression type
	_fileStream->SetMarker(_entryLocalHeaderOffset, streamFrom_Start);
	process_headers(1, false, true, true);

	switch (_entryCompression) {
		case 0:
			{	// No compression
				long pakSize = _entryExpandedSize;
				
				pakData = ::NewHandle(pakSize);
				if (!pakData)
					goto fail;
				::HLock(pakData);
				// FIXME - check for errors getting bytes!
				_fileStream->GetBytes(*pakData, pakSize);
				::HUnlock(pakData);
				
				return new CPakStream(inParent, itemName, offset, pakData);
			}
			break;
			
		case 8:
			{	// Deflation
				theInflator = new CInflator();
				compressedData = ::NewHandle(_entryCompressedSize);
				if (!compressedData)
					goto fail;

				pakData = ::NewHandle(_entryExpandedSize);
				::MoveHHi(pakData);
				if (!pakData)
					goto fail;
					
				::HLock(compressedData);
				_fileStream->GetBytes(*compressedData, _entryCompressedSize);
				::HLock(pakData);
				theInflator->Inflate((unsigned char*)*compressedData, _entryCompressedSize, (unsigned char*)*pakData, _entryExpandedSize);
				::HUnlock(compressedData);
				::DisposeHandle(compressedData);
				::HUnlock(pakData);

				delete theInflator; 
				return new CPakStream(inParent, itemName, offset, pakData);
			}
			break;
			
		default:
			break;
	}		
	
fail:
	if (theInflator)
		delete theInflator;
	if (compressedData)
		::DisposeHandle(compressedData);
	if (pakData)
		::DisposeHandle(pakData);

	return 0;
}
