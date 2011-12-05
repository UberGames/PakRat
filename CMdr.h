/* 
	CMdr.h

	Author:			Tom Naughton
	Description:	Based on mrview
*/

/*
Copyright (C) Matthew 'pagan' Baranowski & Sander 'FireStorm' van Rossen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef CMdr_H
#define CMdr_H

// FIXME: should inherit from a common abstract base class.
#include "CMd3.h"
#include "Md4.h"



class CMdr: public CMd3
{
public:
	CMdr(CResourceManager *resources);
	virtual ~CMdr();

	virtual Boolean 	init(CPakStream *inItem);
	virtual void 		Draw(CModelInstance *instance, renderingAttributes_t *renderingAttributes);
	virtual void 		DrawLinks(CModelInstance *instance, renderingAttributes_t *rendering);
	virtual void 		loadMeshTextures(CModelInstance *instance);

	virtual SInt16		frameCount() ;
	virtual UInt32 		tagNum();
	virtual UInt32 		meshNum();
	virtual ModelType	modelType();
	virtual int 		tagIndexWithName(const char *name);
	virtual string 		tagNameForFrameAtIndex(short frame, short index);
	virtual md3_tag_t 	*tagForFrameAtIndex(short frame, short index);

	virtual Boolean 	canExportFormat(ExportFormatT format); 

protected:

	void 				RB_SurfaceAnim( md4Surface_t *surface, CModelInstance *instance,  CShader *shader, renderingAttributes_t *renderingAttributes )	;
	void 				MC_Compress(const float mat[3][4],unsigned char * comp);
	void				MC_UnCompress(float mat[3][4],const unsigned char * comp);
//	void 				DrawVertexNormals(renderingAttributes_t *renderingAttributes, int vertexNum, float *vertices, float *normals); 
	void 				DrawBoneFrameBox(renderingAttributes_t *rendering, CModelInstance *instance, md4Frame_t *frame);
	md4Frame_t 			*interpolateBoneFrame(md4Frame_t *currBoneFrame, md4Frame_t *nextBoneFrame, float frac);

	md4_t				*_md4;
	char				*_md4Data;
	int 				_lod;

	typedef vector<md3_tag_t*> tagArray;
	tagArray	*_convertedTags;
	
};

#endif	// CMdr_H
