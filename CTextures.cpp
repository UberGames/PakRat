/* 
	CTextures.cpp

	Author:			Tom Naughton
	Description:	<describe the CTextures class here>
*/

#include "CTextures.h"
#include "CTga.h"
#include "CJPEG.h"
#include "CPcx.h"
#include "CFtx.h"
#include "CWal.h"
#include "CFileArchive.h"
#include "CPakStream.h"
#include "CGLImage.h"
#include "CQTImage.h"


CTextures::CTextures()
{
}


CTextures::~CTextures()
{
}

CGLImage *CTextures::loadTexture(CPakStream *pakItem)
{
	CGLImage *image = nil;
	string package, file, extension;

	if (pakItem) {
		decomposeEntryName(pakItem->pathName(), package, file, extension);
		string type = lowerString(extension);
		
		if (type == "tga") {
			image = new CTga();	
		} else if (type == "pcx") {
			image = new CPcx();		
		} else if (type == "wal") {
			image = new CWal();	
		} else if (type == "ftx") {
			image = new CFtx();	
	/*	
		} else if (type == "jpg") {
			image = new CJPEG();		
		} else if (type == "jpeg") {
			image = new CJPEG();	
	*/	
		} else {
			image = new CQTImage();
		}
		
		if (image) {
			if (!image->initImage(pakItem) == noErr) {
				delete image;
				image = nil;
			}
		}
	}	

	return image;
}


CGLImage *CTextures::loadTexture(CFileArchive *pak, string texture)
{
	CGLImage *glimage = nil;
	CPakStream *pakItem = nil;
	string package, file, extension;
	texture = lowerString(fixSlashes(texture));
	
	if (!pak) {
		dprintf("loadTexture pak is nil!\n");
		return nil;
	}
	
	if (!pak || texture == "$whiteimage" || texture == "$lightmap") {
		return CGLImage::whiteImage();
	}

	// sometimes the file type and/or case is wrong!
	if (texture.length() > 0) {
		decomposeEntryName(texture, package, file, extension);
			
		// try other files formats and the root directory
		// FIXME: this is getting out of hand
		if (!pakItem) 
			pakItem = pak->itemWithPathName((package + file + extension).c_str());
		if (!pakItem) 
			pakItem = pak->itemWithPathName((package + file + ".tga").c_str());
		if (!pakItem) 
			pakItem = pak->itemWithPathName((package + file + ".jpg").c_str());
		if (!pakItem) 
			pakItem = pak->itemWithPathName((package + file + ".pcx").c_str());
		if (!pakItem) 
			pakItem = pak->itemWithPathName((package + file + ".ftx").c_str());
		if (!pakItem) 
			pakItem = pak->itemWithPathName((file + extension).c_str());
		if (!pakItem) 
			pakItem = pak->itemWithPathName((file + ".jpg").c_str());
		if (!pakItem) 
			pakItem = pak->itemWithPathName((file + ".tga").c_str());
		if (!pakItem) 
			pakItem = pak->itemWithPathName((file + ".pcx").c_str());
		if (!pakItem) 
			pakItem = pak->itemWithPathName((file + ".ftx").c_str());
	} 
	
	if (pakItem) {
		glimage = loadTexture(pakItem);
		delete pakItem;
		if (glimage)
			return glimage;
	}

	// if we get here it failed, so clean up
		
fail:
		
	dprintf("could not load texture: %s\n", texture.c_str());	
	return nil;
}

