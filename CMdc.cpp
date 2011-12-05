/* 
	CMdc.cpp

	Author:			Tom Naughton
	Description:	Based on Return to Castle Wolfenstein MDC Importer for gmax
	
--------------------------------------------------------------------
-- Return to Castle Wolfenstein MDC Importer for gmax
-- Copyright (c) Chris Cookson, 2001
--
-- cjcookson@hotmail.com
--
-- For gmax Scripts, News, Articles and More check out:
--
-- http://gmax.digitalarenas.com
--
-- Thanks to Ryan Feltrin
-- 
-- Version history:
--	1.1 Tag animation support
-- 	1.0 First public release
--
-- Known issues:
--	* None yet
--------------------------------------------------------------------
*/


#include <gl.h>

#include "CMdc.h"
#include "CShader.h"
#include "C3DModelPane.h"
#include "CModelInstance.h"
#include "CResourceManager.h"
#include "CPakStream.h"
#include "utilities.h"


MDC_Surface::~MDC_Surface()
{
	delete _triangles;
	delete _shaders;
	delete _texCoords;
	delete _vertices;
}

void MDC_Surface::loadFromStream(CPakStream *inItem)
{
	UInt32 startCursor = inItem->cursorPos();

	ident					= swapLong(inItem->getLong());
	inItem->getBytes(64, &name);
	flags					= swapLong(inItem->getLong());
	numCompFrames			= swapLong(inItem->getLong());
	numBaseFrames			= swapLong(inItem->getLong());
	numShaders				= swapLong(inItem->getLong());
	numVertices				= swapLong(inItem->getLong());
	numTriangles			= swapLong(inItem->getLong());
	offsetTriangles			= swapLong(inItem->getLong());
	offsetShaders			= swapLong(inItem->getLong());
	offsetTexCoords			= swapLong(inItem->getLong());
	offsetVertices			= swapLong(inItem->getLong());
	offsetCompVerts			= swapLong(inItem->getLong());
	offsetFrameBaseFrames	= swapLong(inItem->getLong());
	offsetFrameCompFrames	= swapLong(inItem->getLong());
	offsetEnd				= swapLong(inItem->getLong());
	
	// load triangles
	inItem->seek(startCursor + offsetTriangles);
	_triangles = new MDC_Triangle[numTriangles]();
	for (int i = 0 ; i < numTriangles; i++) {
		_triangles[i].loadFromStream(inItem);
	}
	
	// load shaders
	inItem->seek(startCursor + offsetShaders);
	_shaders = new MDC_Shader[numShaders]();
	for (int i = 0 ; i < numShaders; i++) {
		_shaders[i].loadFromStream(inItem);
	}
	
	// load texture coordinates
	inItem->seek(startCursor + offsetTexCoords);
	_texCoords = new MDC_TexCoord[numVertices]();
	for (int i = 0 ; i < numVertices; i++) {
		_texCoords[i].loadFromStream(inItem);
		//_texCoords[i].debugDump();
	}
	
	// load vertices
	inItem->seek(startCursor + offsetVertices);
	_vertices = new MDC_Vertex[numVertices]();
	for (int i = 0 ; i < numVertices; i++) {
		_vertices[i].loadFromStream(inItem);
	}
	
	// CompVerts?
	// FrameBaseFrames?
	// FrameCompFrames?
	
	inItem->seek(startCursor + offsetEnd);
}

Boolean MDC_Surface::isValid()
{
	return (ident == 0x07);
}

void MDC_Surface::debugDump()
{
	dprintf( "MDC_Surface\n-----------\n");
	dprintf( "ident = %d, name = %s, flags = %d\n", ident, name, flags);
	dprintf( "numCompFrames = %d, numBaseFrames = %d, numShaders = %d\n", numCompFrames, numBaseFrames, numShaders);
	dprintf( "numVertices = %d, numTriangles = %d\n", numVertices, numTriangles);
	dprintf( "ofsTris = %d, ofsShaders = %d, ofsTexCoords = %d, ofsVertices = %d, ofsBaseFrames = %d, ofsCompFrames = %d\n",
		offsetTriangles, offsetShaders, offsetTexCoords, offsetVertices,
		offsetFrameBaseFrames, offsetFrameCompFrames);
}


void MDC_Triangle::loadFromStream(CPakStream *inItem)
{
	indices[0] = swapLong(inItem->getLong()) ;
	indices[1] = swapLong(inItem->getLong()) ;
	indices[2] = swapLong(inItem->getLong()) ;
}

void MDC_Vertex::loadFromStream(CPakStream *inItem)
{
	position[0] = (float)swapShort(inItem->getShort());
	position[1] = (float)swapShort(inItem->getShort());
	position[2] = (float)swapShort(inItem->getShort());
	
	u = inItem->getByte();
	v = inItem->getByte();
}


void MDC_TexCoord::loadFromStream(CPakStream *inItem)
{
	stu[0] = swapFloat(inItem->getFloat());
	stu[1] = swapFloat(inItem->getFloat());
	stu[2] = 0.0;
}

void MDC_TexCoord::debugDump()
{
	dprintf( "MDC_TexCoord\n-----------\n");
	dprintf( "stu[0] = %f, stu[1] = %f, stu[2] = %f\n", stu[0], stu[1], stu[2]);
}



void MDC_Shader::loadFromStream(CPakStream *inItem)
{
	inItem->getBytes(64, &name);
	flags	= swapLong(inItem->getLong());
}


void MDC_TagName::loadFromStream(CPakStream *inItem)
{
	inItem->getBytes(64, &name);
}

void MDC_TagName::debugDump()
{
	dprintf("MDC_TagName name: %s\n", name);
}

void MDC_TagFrame::loadFromStream(CPakStream *inItem)
{
	xyz[0] = swapShort(inItem->getShort());
	xyz[1] = swapShort(inItem->getShort());
	xyz[2] = swapShort(inItem->getShort());
	angles[0] = swapShort(inItem->getShort())  * (360.0 / 32767.0);
	angles[1] = swapShort(inItem->getShort())  * (360.0 / 32767.0);
	angles[2] = swapShort(inItem->getShort())  * (360.0 / 32767.0);
}


void MDC_Frame::loadFromStream(CPakStream *inItem)
{
	bboxMin[0] = swapFloat(inItem->getFloat());
	bboxMin[1] = swapFloat(inItem->getFloat());
	bboxMin[2] = swapFloat(inItem->getFloat());
	
	bboxMax[0] = swapFloat(inItem->getFloat());
	bboxMax[1] = swapFloat(inItem->getFloat());
	bboxMax[2] = swapFloat(inItem->getFloat());
	
	localOrigin[0] = swapFloat(inItem->getFloat());
	localOrigin[1] = swapFloat(inItem->getFloat());
	localOrigin[2] = swapFloat(inItem->getFloat());
	
	radius = swapFloat(inItem->getFloat());
	inItem->getBytes(16, &name);
}

void MDC_Frame::debugDump()
{
	dprintf("MDC_Frame name: %s\n", name);
}



void MDC_Header::loadFromStream(CPakStream *inItem)
{
	ident					= inItem->getLong();
	version					= swapLong(inItem->getLong());
	inItem->getBytes(64, &name);
	flags					= swapLong(inItem->getLong());
	numFrames				= swapLong(inItem->getLong());
	numTags					= swapLong(inItem->getLong());
	numSurfaces				= swapLong(inItem->getLong());
	numSkins				= swapLong(inItem->getLong());
	offsetFrames			= swapLong(inItem->getLong());
	offsetTagNames			= swapLong(inItem->getLong());
	offsetTags				= swapLong(inItem->getLong());
	offsetSurfaces			= swapLong(inItem->getLong());
	offsetEnd				= swapLong(inItem->getLong());
}

Boolean MDC_Header::isValid()
{
	if (ident != 'IDPC')
		return false;
	if (version != 0x2)  {
		dprintf ("Incorrect version! Found %d  but should be 2\n", version);
		return false;
	}
	if (numFrames < 1)  {
		dprintf("MDC does not have any frames!\n");
		return false;
	}
	return true;
}

void MDC_Header::debugDump()
{
	dprintf("MDC_Header\n----------\n");
	dprintf("Ident: %d, version: %d, name: %s\n", ident, version, name);
	dprintf("Flags: %d, numFrames: %d, numTags: %d, numSurfaces: %d, numSkins: %d\n", flags, numFrames, numTags, numSurfaces, numSkins);
	dprintf("ofsFrames: %d, ofsTagNames: %d, ofsTags: %d, ofsSurfs: %d, ofsEnd: %d\n", offsetFrames, offsetTagNames, offsetTags, offsetSurfaces, offsetEnd);
}



CMdc::CMdc(CResourceManager *resources) : CMd3(resources)
{
	_animationInfo = nil;
	_modelType = unknown_model;

	_header = nil;
	_frames = nil;
	_tagNames = nil;
	_surfaces = nil;
}


CMdc::~CMdc()
{
	delete _header;
	/* FIXME: delete these
	delete _frames;
	delete _tagNames;
	delete _tagFrames;
	delete _surfaces;
		
	if (_convertedTags) {
		for (tagArray::iterator e = _convertedTags->begin(); e != _convertedTags->end(); e++) 
			if (*e) 
				delete *e;
		//FIXME: leak?
		//delete _convertedTags;
	}
	*/
}


Boolean CMdc::init(CPakStream *inItem)
{
     // load header    
	_header = new MDC_Header();
	_header->loadFromStream(inItem);
	_header->debugDump();
	if (!_header->isValid())
		goto fail;

	// load frames
	inItem->seek(_header->offsetFrames);
	_frames = new MDC_Frame[_header->numFrames]();
	for (int i = 0 ; i < _header->numFrames; i++) {
		_frames[i].loadFromStream(inItem);
		_frames[i].debugDump();
	}
	
	// load tag names
	inItem->seek(_header->offsetTagNames);
	_tagNames = new MDC_TagName[_header->numTags]();
	for (int i = 0 ; i < _header->numTags; i++) {
		_tagNames[i].loadFromStream(inItem);
		_tagNames[i].debugDump();
	}
		
	// process tags
	inItem->seek(_header->offsetTags);
	_tagFrames = new MDC_TagFrame[_header->numTags * _header->numFrames];
	for (int frame = 0; frame < _header->numFrames; frame++) {
		for (int tag = 0; tag < _header->numTags; tag++) {
			UInt32 tagIndex = frame * _header->numFrames + tag;
				_tagFrames[tagIndex].loadFromStream(inItem);
		}
	}
	
	// load surfaces
	inItem->seek(_header->offsetSurfaces);
	_surfaces = new MDC_Surface[_header->numSurfaces]();
	for (int surfaceIndex = 0 ; surfaceIndex < _header->numSurfaces; surfaceIndex++) {
		MDC_Surface *surface = &_surfaces[surfaceIndex];
		surface->loadFromStream(inItem);
		surface->debugDump();
	}

	int tagCount = frameCount() * tagNum();
	_convertedTags = new tagArray[tagCount];
	for(int i = 0; i < tagCount; i++)
		_convertedTags->insert(_convertedTags->end(), (md3_tag_t*)nil );

	return true;
	
fail:

	return false;
}


void CMdc::loadMeshTextures(CModelInstance *instance)
{
#pragma unused (instance)
	for (int surfaceIndex = 0; surfaceIndex < _header->numSurfaces; surfaceIndex++) {
		
		MDC_Surface *surface = &_surfaces[surfaceIndex];
		for (int j = 0; j < surface->numShaders; j++ ) 
		{
			MDC_Shader 	*mdc_shader = &surface->_shaders[j];
			dprintf("shaderWithName %s\n", mdc_shader->name);
			CShader *shader = resources()->shaderWithName(mdc_shader->name);
		}
	}
}

#pragma mark -

void CMdc::Draw(CModelInstance *instance, renderingAttributes_t *rendering)
{	

	// iterate over surfaces
	for (int surfaceIndex = 0; surfaceIndex < _header->numSurfaces; surfaceIndex++) {
		DrawSurface(instance, rendering, &_surfaces[surfaceIndex]);
	}
}

void CMdc::DrawSurface(CModelInstance *instance, renderingAttributes_t *rendering , MDC_Surface *surface)
{	
	MDC_Triangle 	*triangles = surface->_triangles;
	MDC_Vertex 		*vertices = surface->_vertices;
	MDC_TexCoord 	*texCoords = surface->_texCoords;

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, (float*)gShaderArrays.norms);

    gShaderArrays.numcolours = gShaderArrays.numverts = gShaderArrays.numelems = 0;
	int *outElems = gShaderArrays.elems;
	float *outVerts = (float*)gShaderArrays.verts;
	float *outTexvecs = (float*)gShaderArrays.tex_st;
	float *outNormals = (float*)gShaderArrays.norms;
	
	// push the triangles
	for (int j = 0 ; j < surface->numTriangles ; j++) 
	{
		*outElems++ = gShaderArrays.numverts + triangles->indices[0];
		*outElems++ = gShaderArrays.numverts + triangles->indices[1];
		*outElems++ = gShaderArrays.numverts + triangles->indices[2];
		triangles++;
	}
	gShaderArrays.numelems += surface->numTriangles * 3;
	
	// push the vertices, normals, and texture coordinates
	for (int j = 0; j < surface->numVertices; j++ ) 
	{
		vec3_t	tempVert, tempNormal;

		vec_clear( tempVert );
		vec_clear( tempNormal );
		
		tempVert[0] = vertices->position[0];
		tempVert[1] = vertices->position[1];
		tempVert[2] = vertices->position[2];

		*outVerts++ = tempVert[0] / 64.0;
		*outVerts++ = tempVert[1] / 64.0;
		*outVerts++ = tempVert[2] / 64.0;

		*outTexvecs++ = texCoords->stu[0];
		*outTexvecs++ = texCoords->stu[1];

		double u = vertices->u;
		double v = vertices->v;
		double alpha = u * 2.0f * pi / 255.0f;
		double beta =  v * 2.0f * pi / 255.0f;
			
		*outNormals++ = (float)(cos(beta) * sin(alpha)) ;  	  
		*outNormals++ = (float)(sin(beta) * sin(alpha)) ;
		*outNormals++ = (float)cos(alpha) ;


		vertices++;
		texCoords++;
	}
	gShaderArrays.numverts += surface->numVertices;
	
	
    if (rendering->_textured)	
		glEnable( GL_TEXTURE_2D );
	else 
		glDisable( GL_TEXTURE_2D );
		
	// apply all the shaders
	Boolean shaderfound = false;
	for (int j = 0; j < surface->numShaders; j++ ) 
	{
		MDC_Shader 	*mdc_shader = &surface->_shaders[j];
		
		CShader *shader = resources()->shaderWithName(mdc_shader->name);
	    if (shader) {
			shader->renderFlush(rendering, 0);
			shaderfound = true;
		}
	}

	// if shader is missing draw untextured wireframe
	if (!shaderfound && !rendering->_renderTextureCoordinates) {
		glDisable( GL_TEXTURE_2D );
	    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); 
		CShader::wireframeFlush();
	}
	
	if (rendering->_renderOpaque && rendering->_showNormals)
		DrawVertexNormals(rendering, surface->numVertices, (float*)gShaderArrays.verts, (float*)gShaderArrays.norms);
}

void CMdc::DrawLinks(CModelInstance *instance, renderingAttributes_t *rendering) 
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


inline UInt32 CMdc::meshNum()
{
	return _header->numSurfaces;
}

ModelType CMdc::modelType()
{
	_modelType = unknown_model;
	return _modelType;
}

SInt16 CMdc::frameCount() 
{
	return _header->numFrames;
}

inline UInt32 CMdc::tagNum()
{
	return _header->numTags;
}

inline md3_tag_t *CMdc::tagForFrameAtIndex(short framenum, short iTagIndex)
{
	md3_tag_t			*dest;
	
	if ( framenum >= frameCount()  )  {
		// it is possible to have a bad frame while changing models, so don't error
		framenum = frameCount()  - 1;
	}
	
	dest = (*_convertedTags)[framenum * tagNum() +iTagIndex];
	if (!dest) {
	
		// create new tag
		dest = new md3_tag_t;
		dest->info.glName = iTagIndex;
		strcpy(dest->name, _tagNames[iTagIndex].name);
		(*_convertedTags)[framenum * tagNum() +iTagIndex] = dest;

		dest->Position[0]  = _tagFrames[iTagIndex].xyz[0] / 64.0;			
		dest->Position[1]  = _tagFrames[iTagIndex].xyz[1] / 64.0;		
		dest->Position[2]  = _tagFrames[iTagIndex].xyz[2] / 64.0;		
		
		Mat4 matrix = identity;
		
/* FIXME: figure this out

		// cram a quatMatrix in the same place as the 3x3 matrix
		f = (float*)&tag->Rotation;
		float *mv = matrix.m;
		MAT(mv,0,0) = swapFloat(*f++);
		MAT(mv,1,0) = swapFloat(*f++);
		MAT(mv,2,0) = swapFloat(*f++);
*/
		dest->Rotation = Vec4::quatFromMatrix( matrix );
		
	}

	return dest;
}

string 	CMdc::tagNameForFrameAtIndex(short frame, short index)
{
	return _tagNames[index].name;
}

int CMdc::tagIndexWithName(const char *name)
{
	for (int i = 0 ; i < tagNum() ; i++ ) {
		if ( !strcmp( _tagNames[i].name, name ) )
			return i;
	}
	return -1;
}

#pragma mark -

Boolean CMdc::canExportFormat(ExportFormatT format)
{
	switch (format) {
		case WAVEFRONT_OBJ_FORMAT:
		case AUTOCAD_DXF_FORMAT:
			return false;
	}
	return false;
}

