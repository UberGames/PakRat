/* 
	CMdr.cpp

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

#include <gl.h>

#include "CMdr.h"
#include "CShader.h"
#include "C3DModelPane.h"
#include "CModelInstance.h"
#include "CResourceManager.h"
#include "CPakStream.h"
#include "utilities.h"

#define LL(x) x = swapLong(x);

CMdr::CMdr(CResourceManager *resources) : CMd3(resources)
{
	_animationInfo = nil;
	_md4Data = nil;
	_md4 = nil;
	_lod = 0;
	_modelType = unknown_model;
}


CMdr::~CMdr()
{
	if (_convertedTags) {
		for (tagArray::iterator e = _convertedTags->begin(); e != _convertedTags->end(); e++) 
			if (*e) 
				delete *e;
		//FIXME: leak?
		//delete _convertedTags;
	}

	if (_md4) {
		CMemoryTracker::safeFree(_md4);
		_md4 = nil;
	}
	
	if (_md4Data)
		CMemoryTracker::safeFree(_md4Data);
}


Boolean CMdr::init(CPakStream *inItem)
{
	_pathname = inItem->pathName();
           
    // lock and load                                                                                                                                                                                                                                                                                                                                                                                                                                                                 	
	_md4Data = (char*) inItem->getData("md3 data");
	if (!_md4Data )
		goto fail;
	
	_md4 = (md4_t*)CMemoryTracker::safeAlloc(1, sizeof(md4_t), "md4");
	if (!_md4)
		goto fail;
		
	// md3 header
	md4Header_t *md4header = (md4Header_t*) _md4Data;
	_md4->header = md4header;

	// check compatibility
	if (md4header->ident != 'RDM5') {
		dprintf("wrong model type\n");
		goto fail;
	}

	// swap header
	md4header->version = swapLong(md4header->version);
	md4header->numFrames = swapLong(md4header->numFrames);
	md4header->numBones = swapLong(md4header->numBones);
	md4header->ofsFrames = swapLong(md4header->ofsFrames);
	md4header->numLODs = swapLong(md4header->numLODs);
	md4header->ofsLODs = swapLong(md4header->ofsLODs);
	md4header->numTags = swapLong(md4header->numTags);
	md4header->ofsTags = swapLong(md4header->ofsTags);
	md4header->ofsEnd = swapLong(md4header->ofsEnd);

	if (md4header->version != 2 ) {
		dprintf("corrupt md3 file header\n");
		goto fail;
	}
		
	
    dprintf("md4header->version %d\n", md4header->version);
    dprintf("md4header->name %s\n", md4header->name);
    dprintf("md4header->numFrames %d\n", md4header->numFrames);
    dprintf("md4header->numBones %d\n", md4header->numBones);
    dprintf("md4header->ofsFrames %d\n", md4header->ofsFrames);
    dprintf("md4header->numLODs %d\n", md4header->numLODs);
    dprintf("md4header->ofsLODs %d\n", md4header->ofsLODs);
    dprintf("md4header->numTags %d\n", md4header->numTags);
    dprintf("md4header->ofsTags %d\n", md4header->ofsTags);
    dprintf("md4header->ofsEnd %d\n", md4header->ofsEnd);

	if (md4header->ofsFrames<0) {
	
		// compressed model...
		//
		
		// swap all the frames
		int frameSize = (int)( &((md4CompFrame_t *)0)->bones[ md4header->numBones ] );
		md4CompFrame_t *frame;
		for (int i = 0 ; i < md4header->numFrames ; i++, frame++) 
		{
			frame = (md4CompFrame_t *) ( (byte *)md4header + (-md4header->ofsFrames) + i * frameSize );
    		frame->radius = swapFloat( frame->radius );
			for ( int j = 0 ; j < 3 ; j++ ) {
				frame->bounds[0][j] = swapFloat( frame->bounds[0][j] );
				frame->bounds[1][j] = swapFloat( frame->bounds[1][j] );
	    		frame->localOrigin[j] = swapFloat( frame->localOrigin[j] );
			}
			
			// the bones are arrays of shorts
			for ( int j = 0 ; j < (int) (md4header->numBones * sizeof( md4CompBone_t ) / 2) ; j++ ) 
			{
				((unsigned short *)frame->bones)[j] = swapShort( ((unsigned short *)frame->bones)[j] );
			}
		}
		
	} else {
	
		// uncompressed model...
		//
    
		// swap all the frames
		int frameSize = (int)( &((md4Frame_t *)0)->bones[ md4header->numBones ] );
		md4Frame_t *frame;
		for (int i = 0 ; i < md4header->numFrames ; i++, frame++) 
		{
			frame = (md4Frame_t *) ( (byte *)md4header + md4header->ofsFrames + i * frameSize );
    		frame->radius = swapFloat( frame->radius );
			for ( int j = 0 ; j < 3 ; j++ ) {
				frame->bounds[0][j] = swapFloat( frame->bounds[0][j] );
				frame->bounds[1][j] = swapFloat( frame->bounds[1][j] );
	    		frame->localOrigin[j] = swapFloat( frame->localOrigin[j] );
			}
			for ( int j = 0 ; j < (int) (md4header->numBones * sizeof( md4Bone_t ) / 4) ; j++ ) 
			{
				((float *)frame->bones)[j] = swapFloat( ((float *)frame->bones)[j] );
			}
		}
	}

	// swap all the LOD's
	md4LOD_t *lod = (md4LOD_t *) ( (byte *)md4header + md4header->ofsLODs );
	md4Surface_t *surf;
	for (int l = 0 ; l < md4header->numLODs ; l++) {
		LL(lod->numSurfaces);
		LL(lod->ofsSurfaces);
		LL(lod->ofsEnd);

		// swap all the surfaces
		surf = (md4Surface_t *) ( (byte *)lod + lod->ofsSurfaces );
		for (int i = 0 ; i < lod->numSurfaces ; i++) {
			LL(surf->numTriangles);
			LL(surf->ofsTriangles);
			LL(surf->numVerts);
			LL(surf->ofsVerts);
			LL(surf->ofsEnd);
			LL(surf->ofsHeader);
			
			if ( surf->numVerts > SHADER_MAX_VERTEXES ) 
			{
				dprintf ("R_LoadMD4: %s has more than %i verts on a surface (%i)",
					"mod_name", SHADER_MAX_VERTEXES, surf->numVerts );
				goto fail;
			}
			if ( surf->numTriangles*3 > SHADER_MAX_INDEXES ) 
			{
				dprintf ("R_LoadMD4: %s has more than %i triangles on a surface (%i)",
						"mod_name", SHADER_MAX_INDEXES / 3, surf->numTriangles );
				goto fail;
			}
		
			// change to surface identifier
			surf->ident = SF_MD4;

			// lowercase the surface name so skin compares are faster
			//strlwr( surf->name );

			// strip off a trailing _1 or _2
			// this is a crutch until i update carcass
			int j = strlen( surf->name );
			if ( j > 2 && surf->name[j-2] == '_' ) {
				surf->name[j-2] = 0;
			}

			// swap all the triangles
			md4Triangle_t *tri = (md4Triangle_t *) ( (byte *)surf + surf->ofsTriangles );
			for ( j = 0 ; j < surf->numTriangles ; j++, tri++ ) {
				LL(tri->indexes[0]);
				LL(tri->indexes[1]);
				LL(tri->indexes[2]);
			}

			// swap all the vertexes
			md4Vertex_t *v = (md4Vertex_t *) ( (byte *)surf + surf->ofsVerts );
			for ( j = 0 ; j < surf->numVerts ; j++ ) {
				v->normal[0] = swapFloat( v->normal[0] );
				v->normal[1] = swapFloat( v->normal[1] );
				v->normal[2] = swapFloat( v->normal[2] );

				v->texCoords[0] = swapFloat( v->texCoords[0] );
				v->texCoords[1] = swapFloat( v->texCoords[1] );

				v->numWeights = swapLong( v->numWeights );

				for (int k = 0 ; k < v->numWeights ; k++ ) {
					v->weights[k].boneIndex = swapLong( v->weights[k].boneIndex );
					v->weights[k].boneWeight = swapFloat( v->weights[k].boneWeight );
				   v->weights[k].offset[0] = swapFloat( v->weights[k].offset[0] );
				   v->weights[k].offset[1] = swapFloat( v->weights[k].offset[1] );
				   v->weights[k].offset[2] = swapFloat( v->weights[k].offset[2] );
				}
				v = (md4Vertex_t *)&v->weights[v->numWeights];
			}

			// find the next surface
			surf = (md4Surface_t *)( (byte *)surf + surf->ofsEnd );
		}

		// find the next LOD
		lod = (md4LOD_t *)( (byte *)lod + lod->ofsEnd );
	}
	
	md4Tag_t *tag = (md4Tag_t *) ( (byte *)md4header + md4header->ofsTags );
	for (int i = 0 ; i < md4header->numTags ; i++) {
		LL(tag->boneIndex);
		tag++;
	}

	int tagCount = frameCount() * tagNum();
	_convertedTags = new tagArray[tagCount];
	for(int i = 0; i < tagCount; i++)
		_convertedTags->insert(_convertedTags->end(), (md3_tag_t*)nil );
	
	return true;
	
fail:

	return false;
}


void CMdr::loadMeshTextures(CModelInstance *instance)
{
#pragma unused (instance)
}

#pragma mark -

void CMdr::Draw(CModelInstance *instance, renderingAttributes_t *rendering)
{	
	md4Header_t *md4header = _md4->header;
	md4LOD_t *lod = (md4LOD_t *) ( (byte *)md4header + md4header->ofsLODs );


	if (_lod >= md4header->numLODs)
		_lod  = md4header->numLODs-1;				
	
	for ( int i=0; i<_lod; i++) {
		lod = (md4LOD_t*)( (byte *)lod + lod->ofsEnd );
	}

	md4Surface_t * surface = (md4Surface_t *)( (byte *)lod + lod->ofsSurfaces );
	for ( int i = 0 ; i < lod->numSurfaces ; i++ ) {
	
		CShader *shader = instance->textureForMeshAtIndex(i);
		if (!shader) {
			string textureName = instance->textureNameForMesh(surface->name);
		  	if (textureName.length() > 0) {
				shader = resources()->shaderWithName(textureName.c_str());
				instance->_meshTextures[i] = shader;
			}
		}
		
		RB_SurfaceAnim( surface, instance, shader , rendering);						
		surface = (md4Surface_t *)( (byte *)surface + surface->ofsEnd );
	}
	
}

void CMdr::RB_SurfaceAnim( md4Surface_t *surface, CModelInstance *instance,  CShader *shader , renderingAttributes_t *rendering)	
{
	int				i, j, k;
	float			frontlerp, backlerp;
	int				*triangles;
	int				indexes;
	int				baseIndex, baseVertex;
	int				numVerts;
	md4Vertex_t		*v;
	md4Bone_t		bones[MD4_MAX_BONES];
	md4Bone_t		*bonePtr, *bone;
	md4Header_t		*header;
	md4Frame_t		*frame;
	md4Frame_t		*oldFrame;

	int				frameSize;

    gShaderArrays.numcolours = gShaderArrays.numverts = gShaderArrays.numelems = 0;
	Boolean shouldDraw = C3DModel::shouldDraw(rendering, shader);
	Boolean needsNormals = C3DModel::needsNormals(rendering, shader);

	int iFrame		= instance->_currentFrame;
	int iNextFrame	= instance->_nextFrame;


	if (!shouldDraw)
		return;
		
	if (rendering->_lighting) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, (float*)gShaderArrays.norms);
	} else {
		glDisableClientState(GL_NORMAL_ARRAY);
	}

	// don't lerp if lerping off, or this is the only frame, or the last frame...
	//
	if (0) 
	{
		backlerp	= 0;	// if backlerp is 0, lerping is off and frontlerp is never used
		frontlerp	= 1;
	} 
	else  
	{
		backlerp	= instance->_interpolationFraction;
		frontlerp	= 1.0f - backlerp;
	}
	header = (md4Header_t *)((byte *)surface + surface->ofsHeader);

	Boolean bCompressed = header->ofsFrames<0;
	if (bCompressed)
	{
		// compressed model...
		//

		frameSize	= (int)( &((md4CompFrame_t *)0)->bones[ header->numBones ] );
		frame		= (md4Frame_t *)((byte *)header + (-header->ofsFrames) + iFrame		* frameSize );
		oldFrame	= (md4Frame_t *)((byte *)header + (-header->ofsFrames) + iNextFrame	* frameSize );
	}
	else
	{
		frameSize	= (int)( &((md4Frame_t *)0)->bones[ header->numBones ] );
		frame		= (md4Frame_t *)((byte *)header + header->ofsFrames + iFrame	 * frameSize );
		oldFrame	= (md4Frame_t *)((byte *)header + header->ofsFrames + iNextFrame * frameSize );
	}

	int *outElems = gShaderArrays.elems;
	float *outVerts = (float*)gShaderArrays.verts;
	float *outTexvecs = (float*)gShaderArrays.tex_st;
	float *outNormals = (float*)gShaderArrays.norms;

	triangles	= (int *) ((byte *)surface + surface->ofsTriangles);
	indexes		= surface->numTriangles * 3;
	baseIndex	= gShaderArrays.numelems;
	baseVertex	= gShaderArrays.numverts;
	for (j = 0 ; j < indexes ; j++) 
	{
		*outElems++ = baseVertex + triangles[j];
	}
	gShaderArrays.numelems += indexes;

	//
	// lerp all the needed bones
	//
	if ( !backlerp && !bCompressed) 
	{
		// no lerping needed
		bonePtr = frame->bones;
	} 
	else 
	{
		bonePtr = bones;
		if (bCompressed)
		{
			md4CompFrame_t	*cframe = (md4CompFrame_t *)frame;
			md4CompFrame_t	*coldFrame = (md4CompFrame_t *)oldFrame;

			for ( i = 0 ; i < header->numBones ; i++ )
			{
				if ( !backlerp )
					MC_UnCompress(bonePtr[i].matrix,cframe->bones[i].Comp);
				else
				{
					md4Bone_t tbone[2];

					MC_UnCompress(tbone[0].matrix,cframe->bones[i].Comp);
					MC_UnCompress(tbone[1].matrix,coldFrame->bones[i].Comp);
					for ( j = 0 ; j < 12 ; j++ )
						((float *)&bonePtr[i])[j] = frontlerp * ((float *)&tbone[0])[j] + backlerp * ((float *)&tbone[1])[j];
				}
			}
		}
		else
		{
			for ( i = 0 ; i < header->numBones*12 ; i++ ) 
			{
				((float *)bonePtr)[i] = frontlerp * ((float *)frame->bones)[i] + backlerp * ((float *)oldFrame->bones)[i];
			}
		}
	}

	//
	// deform the vertexes by the lerped bones
	//
	numVerts = surface->numVerts;
	v = (md4Vertex_t *) ((byte *)surface + surface->ofsVerts);
	for ( j = 0; j < numVerts; j++ ) 
	{
		vec3_t	tempVert, tempNormal;
		md4Weight_t	*w;

		vec_clear( tempVert );
		vec_clear( tempNormal );
		w = v->weights;
		for ( k = 0 ; k < v->numWeights ; k++, w++ ) 
		{
			bone = bonePtr + w->boneIndex;
			
			tempVert[0] += w->boneWeight * ( vec_dot( bone->matrix[0], w->offset ) + bone->matrix[0][3] );
			tempVert[1] += w->boneWeight * ( vec_dot( bone->matrix[1], w->offset ) + bone->matrix[1][3] );
			tempVert[2] += w->boneWeight * ( vec_dot( bone->matrix[2], w->offset ) + bone->matrix[2][3] );
			
			tempNormal[0] += w->boneWeight * vec_dot( bone->matrix[0], v->normal );
			tempNormal[1] += w->boneWeight * vec_dot( bone->matrix[1], v->normal );
			tempNormal[2] += w->boneWeight * vec_dot( bone->matrix[2], v->normal );
			
		}

		*outVerts++ = tempVert[0];
		*outVerts++ = tempVert[1];
		*outVerts++ = tempVert[2];

		*outTexvecs++ = v->texCoords[0];
		*outTexvecs++ = v->texCoords[1];

		*outNormals++ = tempNormal[0];
		*outNormals++ = tempNormal[1];
		*outNormals++ = tempNormal[2];

		v = (md4Vertex_t *)&v->weights[v->numWeights];
	}
	gShaderArrays.numverts += numVerts;

    if (rendering->_textured)	
		glEnable( GL_TEXTURE_2D );
	else 
		glDisable( GL_TEXTURE_2D );
		
    if (shader)
		shader->renderFlush(rendering, 0);
	else if (!rendering->_renderTextureCoordinates) {
	
		// if shader is missing draw untextured wireframe
		glDisable( GL_TEXTURE_2D );
	    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); 
		CShader::wireframeFlush();
	}
	
	if (rendering->_renderOpaque && rendering->_showNormals)
		DrawVertexNormals(rendering, numVerts, (float*)gShaderArrays.verts, (float*)gShaderArrays.norms);
	
	if(rendering->_showBoneFrameBox)
		DrawBoneFrameBox(rendering, instance,
			interpolateBoneFrame(frame, oldFrame, instance->_interpolationFraction));

}

md4Frame_t *CMdr::interpolateBoneFrame(md4Frame_t *currBoneFrame, md4Frame_t *nextBoneFrame, float frac) 
{
	static md4Frame_t tmpBoneFrame_1;
	tmpBoneFrame_1.bounds[0][0] = (1.0f - frac) * currBoneFrame->bounds[0][0] + frac * nextBoneFrame->bounds[0][0];
	tmpBoneFrame_1.bounds[1][0] = (1.0f - frac) * currBoneFrame->bounds[1][0] + frac * nextBoneFrame->bounds[1][0];
	tmpBoneFrame_1.localOrigin[0] = (1.0f - frac) * currBoneFrame->localOrigin[0] + frac * nextBoneFrame->localOrigin[0];
	tmpBoneFrame_1.bounds[0][1] = (1.0f - frac) * currBoneFrame->bounds[0][1] + frac * nextBoneFrame->bounds[0][1];
	tmpBoneFrame_1.bounds[1][1] = (1.0f - frac) * currBoneFrame->bounds[1][1] + frac * nextBoneFrame->bounds[1][1];
	tmpBoneFrame_1.localOrigin[1] = (1.0f - frac) * currBoneFrame->localOrigin[1] + frac * nextBoneFrame->localOrigin[1];
	tmpBoneFrame_1.bounds[0][2] = (1.0f - frac) * currBoneFrame->bounds[0][2] + frac * nextBoneFrame->bounds[0][2];
	tmpBoneFrame_1.bounds[1][2] = (1.0f - frac) * currBoneFrame->bounds[1][2] + frac * nextBoneFrame->bounds[1][2];
	tmpBoneFrame_1.localOrigin[2] = (1.0f - frac) * currBoneFrame->localOrigin[2] + frac * nextBoneFrame->localOrigin[2];
		
	return &tmpBoneFrame_1;
}

void CMdr::DrawBoneFrameBox(renderingAttributes_t *rendering, CModelInstance *instance, md4Frame_t *frame) 
{
#pragma unused (instance)
	
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	
	/*
	md3_boneFrame_t	*bf = &_md3->boneframes[instance->_currentFrame];
				
	if (instance->_interpolationFraction != 0.0f) {
		bf = interpolateBoneFrame(
			&_md3->boneframes[instance->_currentFrame], 
			&_md3->boneframes[instance->_nextFrame], 
			instance->_interpolationFraction);
	}
	*/
	
	
	float x1=frame->bounds[0][0], y1=frame->bounds[0][1], z1=frame->bounds[0][2],
				x2=frame->bounds[1][0], y2=frame->bounds[1][1], z2=frame->bounds[1][2];
    
    	if (rendering->_renderOpaque) {
    		glDisable(GL_DEPTH_TEST);
			glPointSize(6);
			glColor3f(1,0,0);
			glBegin(GL_POINTS);
			glVertex3f(frame->localOrigin[0], frame->localOrigin[1], frame->localOrigin[2]);		
			glEnd();
			glPointSize(1);
    		glEnable(GL_DEPTH_TEST);
		}
		
    	if (rendering->_renderOpaque) {
			glColor3f(0,1,0);
			glBegin(GL_LINE_LOOP);
			glVertex3f(x1,y1,z1);
			glVertex3f(x1,y1,z2);
			glVertex3f(x1,y2,z2);
			glVertex3f(x1,y2,z1);
			glEnd();
			
			glBegin(GL_LINE_LOOP);
			glVertex3f(x2,y2,z2);
			glVertex3f(x2,y1,z2);
			glVertex3f(x2,y1,z1);
			glVertex3f(x2,y2,z1);
			glEnd();
			
			glBegin(GL_LINES);
			glVertex3f(x1,y1,z1);
			glVertex3f(x2,y1,z1);
			
			glVertex3f(x1,y1,z2);
			glVertex3f(x2,y1,z2);
			
			glVertex3f(x1,y2,z2);
			glVertex3f(x2,y2,z2);
			
			glVertex3f(x1,y2,z1);
			glVertex3f(x2,y2,z1);
			glEnd();
		}
		
    if (rendering->_textured)	
		glEnable( GL_TEXTURE_2D );
		

	if (rendering->_lighting)
		glEnable( GL_LIGHTING );
		
	glColor4f(1,1,1,1);
} 


/*
void CMdr::DrawVertexNormals(
	renderingAttributes_t *renderingAttributes, 
	int vertexNum, 
	float *vertices, 
	float *normals) 
{

  	float x, y, z;
  	float nx, ny, nz;

	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	glColor3f(1,1,0);

  	glBegin( GL_LINES );
  	for (int i=0;i<vertexNum;i++) {

		x = *vertices++;
		y = *vertices++;
		z = *vertices++;
		
		nx = *normals++ + x;
		ny = *normals++ + y;
		nz = *normals++ + z;
		
		glVertex3f(x ,y ,z);
		glVertex3f(nx ,ny ,nz);
  	}
	glEnd();
	
    if (renderingAttributes->_textured)	
		glEnable( GL_TEXTURE_2D );

	if (renderingAttributes->_lighting)
		glEnable( GL_LIGHTING );
		
	glColor4f(1,1,1,1);
}
*/

void CMdr::DrawLinks(CModelInstance *instance, renderingAttributes_t *rendering) 
{	
	glDisable( GL_ALPHA_TEST);
	glDisable( GL_BLEND);
	glDisable( GL_DEPTH_TEST);
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );

	// draw linked models
	for (int j=0; j< tagNum() ; j++) {
		CModelInstance *child = instance->_links[j];
		md3_tag_t *tag = tagForFrameAtIndex(instance->_currentFrame, j);
		
		PushTagTransformMatrix(instance, j);
		
		// don't draw the link if it's the root and has a parent
		Boolean drawLink = rendering->_pickTag;
		
		// FIXME - this doesn't work for HK model
		if (tag->Position[0] == 0.0 &&
			tag->Position[1] == 0.0 &&
			tag->Position[2] == 0.0) {
			if (drawLink && instance->parent()) 
				drawLink = false;
		}
		
		if (drawLink) {

			
			glPushName (tag->info.glName + instance->glName());

			if (rendering->_pickedName == tag->info.glName + instance->glName()) {
				glColor4f(1,0,0,1);
			} else {
				glColor4f(1,1,1,1);
			}
			
			glPointSize(6);
			glBegin(GL_POINTS);
			glVertex3f(0, 0, 0);		
			glEnd();
			glPointSize(1);
			
			glRasterPos2f(0, 0);
			glPrint("  %s", &tag->name[0]);
			
			glPopName();

		}
		
		glPopMatrix();
	}

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST);
}

#pragma mark -

#define MC_BITS_X (16)
#define MC_BITS_Y (16)
#define MC_BITS_Z (16)
#define MC_BITS_VECT (16)

#define MC_SCALE_X (1.0f/64)
#define MC_SCALE_Y (1.0f/64)
#define MC_SCALE_Z (1.0f/64)


// currently 11
#define MC_COMP_BYTES (((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT*9)+7)/8)

#define MC_MASK_X ((1<<(MC_BITS_X))-1)
#define MC_MASK_Y ((1<<(MC_BITS_Y))-1)
#define MC_MASK_Z ((1<<(MC_BITS_Z))-1)
#define MC_MASK_VECT ((1<<(MC_BITS_VECT))-1)

#define MC_SCALE_VECT (1.0f/(float)((1<<(MC_BITS_VECT-1))-2))

#define MC_POS_X (0)
#define MC_SHIFT_X (0)

#define MC_POS_Y ((((MC_BITS_X))/8))
#define MC_SHIFT_Y ((((MC_BITS_X)%8)))

#define MC_POS_Z ((((MC_BITS_X+MC_BITS_Y))/8))
#define MC_SHIFT_Z ((((MC_BITS_X+MC_BITS_Y)%8)))

#define MC_POS_V11 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z))/8))
#define MC_SHIFT_V11 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z)%8)))

#define MC_POS_V12 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT))/8))
#define MC_SHIFT_V12 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT)%8)))

#define MC_POS_V13 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT*2))/8))
#define MC_SHIFT_V13 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT*2)%8)))

#define MC_POS_V21 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT*3))/8))
#define MC_SHIFT_V21 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT*3)%8)))

#define MC_POS_V22 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT*4))/8))
#define MC_SHIFT_V22 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT*4)%8)))

#define MC_POS_V23 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT*5))/8))
#define MC_SHIFT_V23 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT*5)%8)))

#define MC_POS_V31 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT*6))/8))
#define MC_SHIFT_V31 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT*6)%8)))

#define MC_POS_V32 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT*7))/8))
#define MC_SHIFT_V32 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT*7)%8)))

#define MC_POS_V33 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT*8))/8))
#define MC_SHIFT_V33 ((((MC_BITS_X+MC_BITS_Y+MC_BITS_Z+MC_BITS_VECT*8)%8)))


void CMdr::MC_Compress(const float mat[3][4],unsigned char * comp)
{
	int i,val;
	for (i=0;i<MC_COMP_BYTES;i++)
		comp[i]=0;

	val=(int)(mat[0][3]/MC_SCALE_X);
	val+=1<<(MC_BITS_X-1);
	if (val>=(1<<MC_BITS_X))
		val=(1<<MC_BITS_X)-1;
	if (val<0)
		val=0;
	*(unsigned int *)(comp+MC_POS_X)|=((unsigned int)(val))<<MC_SHIFT_X;

	val=(int)(mat[1][3]/MC_SCALE_Y);
	val+=1<<(MC_BITS_Y-1);
	if (val>=(1<<MC_BITS_Y))
		val=(1<<MC_BITS_Y)-1;
	if (val<0)
		val=0;
	*(unsigned int *)(comp+MC_POS_Y)|=((unsigned int)(val))<<MC_SHIFT_Y;

	val=(int)(mat[2][3]/MC_SCALE_Z);
	val+=1<<(MC_BITS_Z-1);
	if (val>=(1<<MC_BITS_Z))
		val=(1<<MC_BITS_Z)-1;
	if (val<0)
		val=0;
	*(unsigned int *)(comp+MC_POS_Z)|=((unsigned int)(val))<<MC_SHIFT_Z;


	val=(int)(mat[0][0]/MC_SCALE_VECT);
	val+=1<<(MC_BITS_VECT-1);
	if (val>=(1<<MC_BITS_VECT))
		val=(1<<MC_BITS_VECT)-1;
	if (val<0)
		val=0;
	*(unsigned int *)(comp+MC_POS_V11)|=((unsigned int)(val))<<MC_SHIFT_V11;

	val=(int)(mat[0][1]/MC_SCALE_VECT);
	val+=1<<(MC_BITS_VECT-1);
	if (val>=(1<<MC_BITS_VECT))
		val=(1<<MC_BITS_VECT)-1;
	if (val<0)
		val=0;
	*(unsigned int *)(comp+MC_POS_V12)|=((unsigned int)(val))<<MC_SHIFT_V12;

	val=(int)(mat[0][2]/MC_SCALE_VECT);
	val+=1<<(MC_BITS_VECT-1);
	if (val>=(1<<MC_BITS_VECT))
		val=(1<<MC_BITS_VECT)-1;
	if (val<0)
		val=0;
	*(unsigned int *)(comp+MC_POS_V13)|=((unsigned int)(val))<<MC_SHIFT_V13;


	val=(int)(mat[1][0]/MC_SCALE_VECT);
	val+=1<<(MC_BITS_VECT-1);
	if (val>=(1<<MC_BITS_VECT))
		val=(1<<MC_BITS_VECT)-1;
	if (val<0)
		val=0;
	*(unsigned int *)(comp+MC_POS_V21)|=((unsigned int)(val))<<MC_SHIFT_V21;

	val=(int)(mat[1][1]/MC_SCALE_VECT);
	val+=1<<(MC_BITS_VECT-1);
	if (val>=(1<<MC_BITS_VECT))
		val=(1<<MC_BITS_VECT)-1;
	if (val<0)
		val=0;
	*(unsigned int *)(comp+MC_POS_V22)|=((unsigned int)(val))<<MC_SHIFT_V22;

	val=(int)(mat[1][2]/MC_SCALE_VECT);
	val+=1<<(MC_BITS_VECT-1);
	if (val>=(1<<MC_BITS_VECT))
		val=(1<<MC_BITS_VECT)-1;
	if (val<0)
		val=0;
	*(unsigned int *)(comp+MC_POS_V23)|=((unsigned int)(val))<<MC_SHIFT_V23;

	val=(int)(mat[2][0]/MC_SCALE_VECT);
	val+=1<<(MC_BITS_VECT-1);
	if (val>=(1<<MC_BITS_VECT))
		val=(1<<MC_BITS_VECT)-1;
	if (val<0)
		val=0;
	*(unsigned int *)(comp+MC_POS_V31)|=((unsigned int)(val))<<MC_SHIFT_V31;

	val=(int)(mat[2][1]/MC_SCALE_VECT);
	val+=1<<(MC_BITS_VECT-1);
	if (val>=(1<<MC_BITS_VECT))
		val=(1<<MC_BITS_VECT)-1;
	if (val<0)
		val=0;
	*(unsigned int *)(comp+MC_POS_V32)|=((unsigned int)(val))<<MC_SHIFT_V32;

	val=(int)(mat[2][2]/MC_SCALE_VECT);
	val+=1<<(MC_BITS_VECT-1);
	if (val>=(1<<MC_BITS_VECT))
		val=(1<<MC_BITS_VECT)-1;
	if (val<0)
		val=0;
	*(unsigned int *)(comp+MC_POS_V33)|=((unsigned int)(val))<<MC_SHIFT_V33;
}

void CMdr::MC_UnCompress(float mat[3][4],const unsigned char * comp)
{
	int val;

	val=(int)((unsigned short *)(comp))[0];
	val-=1<<(MC_BITS_X-1);
	mat[0][3]=((float)(val))*MC_SCALE_X;

	val=(int)((unsigned short *)(comp))[1];
	val-=1<<(MC_BITS_Y-1);
	mat[1][3]=((float)(val))*MC_SCALE_Y;

	val=(int)((unsigned short *)(comp))[2];
	val-=1<<(MC_BITS_Z-1);
	mat[2][3]=((float)(val))*MC_SCALE_Z;

	val=(int)((unsigned short *)(comp))[3];
	val-=1<<(MC_BITS_VECT-1);
	mat[0][0]=((float)(val))*MC_SCALE_VECT;

	val=(int)((unsigned short *)(comp))[4];
	val-=1<<(MC_BITS_VECT-1);
	mat[0][1]=((float)(val))*MC_SCALE_VECT;

	val=(int)((unsigned short *)(comp))[5];
	val-=1<<(MC_BITS_VECT-1);
	mat[0][2]=((float)(val))*MC_SCALE_VECT;


	val=(int)((unsigned short *)(comp))[6];
	val-=1<<(MC_BITS_VECT-1);
	mat[1][0]=((float)(val))*MC_SCALE_VECT;

	val=(int)((unsigned short *)(comp))[7];
	val-=1<<(MC_BITS_VECT-1);
	mat[1][1]=((float)(val))*MC_SCALE_VECT;

	val=(int)((unsigned short *)(comp))[8];
	val-=1<<(MC_BITS_VECT-1);
	mat[1][2]=((float)(val))*MC_SCALE_VECT;


	val=(int)((unsigned short *)(comp))[9];
	val-=1<<(MC_BITS_VECT-1);
	mat[2][0]=((float)(val))*MC_SCALE_VECT;

	val=(int)((unsigned short *)(comp))[10];
	val-=1<<(MC_BITS_VECT-1);
	mat[2][1]=((float)(val))*MC_SCALE_VECT;

	val=(int)((unsigned short *)(comp))[11];
	val-=1<<(MC_BITS_VECT-1);
	mat[2][2]=((float)(val))*MC_SCALE_VECT;

}

#pragma mark -

inline UInt32 CMdr::tagNum()
{
	return _md4->header->numTags;
}

inline UInt32 CMdr::meshNum()
{
	md4Header_t *md4header = _md4->header;
	md4LOD_t *lod = (md4LOD_t *) ( (byte *)md4header + md4header->ofsLODs );


	if (_lod >= md4header->numLODs)
		_lod  = md4header->numLODs-1;				
	
	for ( int i=0; i<_lod; i++) {
		lod = (md4LOD_t*)( (byte *)lod + lod->ofsEnd );
	}

	return lod->numSurfaces;
}

ModelType CMdr::modelType()
{
	if (_modelType != unknown_model)
		return _modelType;
	
	string package, file, extension;
	decomposeEntryName(pathName(), package, file, extension);
	if (stringStartsWith(lowerString(file), string("upper"))) 
		_modelType = upper_model;
	else if (stringStartsWith(lowerString(file), string("lower"))) 
		_modelType = lower_model;
	else if (stringStartsWith(lowerString(file), string("head"))) 
		_modelType = head_model;
	else 
		_modelType = other_model;

	return _modelType;
}

inline md3_tag_t *CMdr::tagForFrameAtIndex(short framenum, short iTagIndex)
{
	md3_tag_t			*dest;
	int					i;
	int					frameSize;
	md4CompFrame_t		*cframe;
	md4Frame_t			*frame;
	md4Tag_t			*tag;
	md4Header_t 		*md4header = _md4->header;
	
	if ( framenum >= frameCount()  )  {
		// it is possible to have a bad frame while changing models, so don't error
		framenum = frameCount()  - 1;
	}
	
	dest = (*_convertedTags)[framenum * tagNum() +iTagIndex];
	if (dest) {
		return dest;
	} else {
		dest = new md3_tag_t;
		dest->Rotation.set(0,0,0,0);
		dest->info.glName = iTagIndex;
		vec_clear( dest->Position );
		strcpy(dest->name,"");
		(*_convertedTags)[framenum * tagNum() +iTagIndex] = dest;
	}

	tag = (md4Tag_t *)((byte *)md4header + md4header->ofsTags);
	for ( i = 0 ; i < tagNum() ; i++, tag++ ) {
		if (iTagIndex == i) {
			strcpy(dest->name,tag->name);

			if (md4header->ofsFrames <0) {
			
				// compressed model...
				//
				frameSize = (int)( &((md4CompFrame_t *)0)->bones[ md4header->numBones ] );
				cframe = (md4CompFrame_t *)((byte *)md4header + (-md4header->ofsFrames) + framenum * frameSize );

				md4Bone_t Bone;

				MC_UnCompress(Bone.matrix, cframe->bones[tag->boneIndex].Comp);
				
				Mat4 matrix = identity;
				float *mv = matrix.m;
				MAT(mv,0,0) = Bone.matrix[0][0];
				MAT(mv,1,0) = Bone.matrix[1][0];
				MAT(mv,2,0) = Bone.matrix[2][0];
				MAT(mv,0,1) = Bone.matrix[0][1];
				MAT(mv,1,1) = Bone.matrix[1][1];
				MAT(mv,2,1) = Bone.matrix[2][1];
				MAT(mv,0,2) = Bone.matrix[0][2];
				MAT(mv,1,2) = Bone.matrix[1][2];
				MAT(mv,2,2) = Bone.matrix[2][2];
				dest->Rotation = Vec4::quatFromMatrix( matrix );

				dest->Position[0]=Bone.matrix[0][3];
				dest->Position[1]=Bone.matrix[1][3];
				dest->Position[2]=Bone.matrix[2][3];				
				
			} else {
				// uncompressed model...
				//
				frameSize = (int)( &((md4Frame_t *)0)->bones[ md4header->numBones ] );
				frame = (md4Frame_t *)((byte *)md4header + md4header->ofsFrames + framenum * frameSize );
				
				Mat4 matrix = identity;
				float *mv = matrix.m;
				MAT(mv,0,0) = frame->bones[tag->boneIndex].matrix[0][0];
				MAT(mv,1,0) = frame->bones[tag->boneIndex].matrix[1][0];
				MAT(mv,2,0) = frame->bones[tag->boneIndex].matrix[2][0];
				MAT(mv,0,1) = frame->bones[tag->boneIndex].matrix[0][1];
				MAT(mv,1,1) = frame->bones[tag->boneIndex].matrix[1][1];
				MAT(mv,2,1) = frame->bones[tag->boneIndex].matrix[2][1];
				MAT(mv,0,2) = frame->bones[tag->boneIndex].matrix[0][2];
				MAT(mv,1,2) = frame->bones[tag->boneIndex].matrix[1][2];
				MAT(mv,2,2) = frame->bones[tag->boneIndex].matrix[2][2];
				dest->Rotation = Vec4::quatFromMatrix( matrix );

				dest->Position[0]=frame->bones[tag->boneIndex].matrix[0][3];
				dest->Position[1]=frame->bones[tag->boneIndex].matrix[1][3];
				dest->Position[2]=frame->bones[tag->boneIndex].matrix[2][3];	
							
			}
		}
	}
	
	return dest;
}

string 	CMdr::tagNameForFrameAtIndex(short frame, short index)
{
	md3_tag_t *tag =  tagForFrameAtIndex(frame, index);
	return tag->name;
}

int CMdr::tagIndexWithName(const char *name)
{
	md4Header_t 		*md4header = _md4->header;
	md4Tag_t * tag = (md4Tag_t *)((byte *)md4header + md4header->ofsTags);
	for (int i = 0 ; i < tagNum() ; i++, tag++ ) {
		if ( !strcmp( tag->name, name ) )
			return i;
	}
	return -1;
}

SInt16 CMdr::frameCount() 
{
	return _md4->header->numFrames;	
}

#pragma mark -

Boolean CMdr::canExportFormat(ExportFormatT format)
{
	switch (format) {
		case WAVEFRONT_OBJ_FORMAT:
		case AUTOCAD_DXF_FORMAT:
			return false;
	}
	return false;
}


