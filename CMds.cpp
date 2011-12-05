/* 
	CMds.cpp

	Author:			Tom Naughton
*/


#include <gl.h>

#include "CMds.h"
#include "CShader.h"
#include "C3DModelPane.h"
#include "CModelInstance.h"
#include "CResourceManager.h"
#include "CPakStream.h"
#include "utilities.h"

#define LL(x) x = swapLong(x);

void MDSHeader::loadFromStream(CPakStream *inItem)
{
	ident			= swapLong(inItem->getLong());
	version			= swapLong(inItem->getLong());
	inItem->getBytes(MAX_QPATH, &name);
	lodScale		= swapFloat(inItem->getFloat());
	lodBias			= swapFloat(inItem->getFloat());
	numFrames		= swapLong(inItem->getLong());
	numBones		= swapLong(inItem->getLong());
	offsetFrames	= swapLong(inItem->getLong());
	offsetBones		= swapLong(inItem->getLong());
	torsoParent		= swapLong(inItem->getLong());
	numSurfaces		= swapLong(inItem->getLong());
	offsetSurfaces	= swapLong(inItem->getLong());
	numTags			= swapLong(inItem->getLong());
	offsetTags		= swapLong(inItem->getLong());
	offsetEnd 		= swapLong(inItem->getLong());
}


Boolean MDSHeader::isValid()
{
	if (ident != ID_HEADER_MDS)
		return false;
	if (version != MDS_VERSION)  {
		dprintf ("Incorrect version! Found %d  but should be 2\n", version);
		return false;
	}
	if (numFrames < 1)  {
		dprintf("MDS does not have any frames!\n");
		return false;
	}
	return true;
}

void MDSHeader::debugDump()
{
	dprintf("MDSHeader\n----------\n");
	dprintf("Ident: %d, version: %d, name: %s\n", ident, version, name);
	dprintf("numFrames: %d, numBones: %d, numTags: %d, numSurfaces: %d, torsoParent: %d\n", numFrames, numBones, numTags, numSurfaces, torsoParent);
	dprintf("offsetFrames: %d, offsetTags: %d, offsetBones: %d, offsetSurfaces: %d, offsetEnd; : %d\n", offsetFrames, offsetTags, offsetBones, offsetSurfaces, offsetEnd );
}





void MDSWeight::loadFromStream(CPakStream *inItem)
{
	boneIndex = swapLong(inItem->getLong()) ;	
	weight = swapFloat(inItem->getFloat());		
	xyz[0] = swapFloat(inItem->getFloat());
	xyz[1] = swapFloat(inItem->getFloat());
	xyz[2] = swapFloat(inItem->getFloat());
}


void MDSVertex::loadFromStream(CPakStream *inItem)
{
	// Read nonweight Vertex data
	//    (sizeof(MDSVertex) - sizeof(MDSWeight))

	normal[0] 		= swapFloat(inItem->getFloat());
	normal[1] 		= swapFloat(inItem->getFloat());
	normal[2] 		= swapFloat(inItem->getFloat());
	texCoords[0] 	= swapFloat(inItem->getFloat());
	texCoords[1] 	= swapFloat(inItem->getFloat());
	numWeights 		= swapLong(inItem->getLong()) ;	
	fixedParent 	= swapLong(inItem->getLong()) ;	
	fixedDist 		= swapFloat(inItem->getFloat()) ;	
	
	// Then all the weights
	
	_weights = new MDSWeight[numWeights]();
	for (int weight = 0; weight < numWeights; weight++)
	{
		// Read MDSWeight
		_weights[weight].loadFromStream(inItem);
	}
}


void MDSPoly::loadFromStream(CPakStream *inItem)
{
	vind[0] = swapLong(inItem->getLong()) ;	
	vind[1] = swapLong(inItem->getLong()) ;	
	vind[2] = swapLong(inItem->getLong()) ;	
}

void MDSBoneRef::loadFromStream(CPakStream *inItem)
{
	Ref = swapLong(inItem->getLong()) ;	
}

void MDSSurface::loadFromStream(CPakStream *inItem)
{
	UInt32 startCursor = inItem->cursorPos();

	ident				= swapLong(inItem->getLong());
	inItem->getBytes(MAX_QPATH, &name);
	inItem->getBytes(MAX_QPATH, &shader);
	shaderIndex			= swapLong(inItem->getLong());
	minLod				= swapLong(inItem->getLong());
	offsetHeader		= swapLong(inItem->getLong());
	numVerts			= swapLong(inItem->getLong());
	offsetVerts			= swapLong(inItem->getLong());
	numTris				= swapLong(inItem->getLong());
	offsetTris			= swapLong(inItem->getLong());
	offsetCollapseMap	= swapLong(inItem->getLong());
	numBoneRefs 		= swapLong(inItem->getLong());
	offsetBoneRefs 		= swapLong(inItem->getLong());
	offsetEnd			= swapLong(inItem->getLong());
	
	// Vertices
	inItem->seek(startCursor + offsetVerts);
	_vertices = new MDSVertex[numVerts]();
	for (int vertex = 0; vertex < numVerts; vertex++)
	{
		// Read MDSVertex
		_vertices[vertex].loadFromStream(inItem);
	}

	inItem->seek(startCursor + offsetTris);
	_triangles = new MDSPoly[numTris]();
	for (int triangle = 0; triangle < numTris; triangle++)
	{
		// Read MDSPoly
		_triangles[triangle].loadFromStream(inItem);
	}

	inItem->seek(startCursor + offsetBoneRefs);
	_bonerefs = new MDSBoneRef[numBoneRefs]();
	for (int bone = 0; bone < numBoneRefs; bone++)
	{
		// Read MDSBoneRef
		_bonerefs[bone].loadFromStream(inItem);
	}


	inItem->seek(startCursor + offsetEnd);
}

void MDSSurface::debugDump()
{
	dprintf("MDSSurface\n----------\n");
	dprintf("	Ident: %d,  name: %s shader: %s shaderIndex: %d\n", 
		ident, name, shader, shaderIndex );
	dprintf("	numVerts: %d, numTris: %d, numBoneRefs: %d\n",
		numVerts, numTris, numBoneRefs );
}

void MDSBone::loadFromStream(CPakStream *inItem)
{
	inItem->getBytes(MAX_QPATH, &name);
	parentIndex 	= swapLong(inItem->getLong());
	torsoWeight		= swapFloat(inItem->getFloat());
	parentDist 		= swapFloat(inItem->getFloat());
	flags			= swapLong(inItem->getLong());
}

void MDSBone::debugDump()
{
	dprintf("MDSBone\n----------\n");
	dprintf(" name: %s\n", name);
}

void MDSBoneFrame::loadFromStream(CPakStream *inItem)
{
	Orientation[0] = swapShort(inItem->getShort());
	Orientation[1] = swapShort(inItem->getShort());
	Orientation[2] = swapShort(inItem->getShort());
	Position[0] = swapShort(inItem->getShort());
	Position[1] = swapShort(inItem->getShort());
	Position[2] = swapShort(inItem->getShort());
}

void MDSBoneFrame::debugDump()
{
	dprintf("MDSBoneFrame\n----------\n");
	dprintf(" orientation %d,%d,%d \n", Orientation[0], Orientation[1], Orientation[2]);
	dprintf(" Position %d,%d,%d \n", Position[0], Position[1], Position[2]);
}


void MDSFrame::loadFromStream(CPakStream *inItem)
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
	parentOffset[0] = swapFloat(inItem->getFloat());
	parentOffset[1] = swapFloat(inItem->getFloat());
	parentOffset[2] = swapFloat(inItem->getFloat());
}

void MDSFrame::debugDump()
{
	dprintf("MDSFrame\n----------\n");
	dprintf(" bboxMin %f,%f,%f \n", bboxMin[0], bboxMin[1], bboxMin[2]);
	dprintf(" bboxMax %f,%f,%f \n", bboxMax[0], bboxMax[1], bboxMax[2]);
	dprintf(" localOrigin %f,%f,%f \n", localOrigin[0], localOrigin[1], localOrigin[2]);
	dprintf(" parentOffset %f,%f,%f \n", parentOffset[0], parentOffset[1], parentOffset[2]);
	dprintf(" radius %f \n", radius);
}

void MDSTag::loadFromStream(CPakStream *inItem)
{
	inItem->getBytes(MAX_QPATH, &name);
	torsoWeight	= swapFloat(inItem->getFloat());
	boneIndex	= swapLong(inItem->getLong());
}



CMds::CMds(CResourceManager *resources) : CMd3(resources)
{
}


CMds::~CMds()
{
}


Boolean CMds::init(CPakStream *inItem)
{
	// Read MDSHeader
	_header = new MDSHeader();
	_header->loadFromStream(inItem);
	_header->debugDump();
	if (!_header->isValid())
		goto fail;
           

	// At _header->offsetBones
	inItem->seek(_header->offsetBones);
	_bones = new MDSBone[_header->numBones]();
	for (int bone = 0; bone < _header->numBones; bone++)
	{
		// Read MDSBone
		_bones[bone].loadFromStream(inItem);
	}

	// Now do Frames
	// At _header->offsetFrames
	inItem->seek(_header->offsetFrames);
	_frames = new MDSFrame[_header->numFrames]();
	_boneFrames = new MDSBoneFrame[_header->numFrames * _header->numBones]();
	for (int frame = 0; frame < _header->numFrames; frame++)
	{
		// Read MDSFrame
		_frames[frame].loadFromStream(inItem);
		
		// Read MDSBoneFrame for each bone
		for (int bone = 0; bone < _header->numBones; bone++)
		{
			_boneFrames[frame * _header->numBones + bone].loadFromStream(inItem);
		}
	}

	// Now do meshes/surfaces/whatever
	// At _header->offsetSurfaces
	inItem->seek(_header->offsetSurfaces);
	_surfaces = new MDSSurface[_header->numSurfaces * _header->torsoParent]();
	for (int lod = 0; lod < _header->torsoParent; lod++)
	{
		// Read LOD
		for (int surface = 0; surface < _header->numSurfaces; surface++)
		{
			// Read MDSSurface
			dprintf("lod %d, surface %d\n", lod, surface);
			_surfaces[_header->numSurfaces * lod + surface].loadFromStream(inItem);
			_surfaces[_header->numSurfaces * lod + surface].debugDump();
		}
	}


	// Now there is some sort of tag reference tacked on the end
	// At _header->offsetTags
	inItem->seek(_header->offsetTags);
	_tags = new MDSTag[_header->numTags]();
	for (int tag = 0; tag < _header->numTags; tag++)
	{
		// Read MDSTag
		_tags[tag].loadFromStream(inItem);
	}

		
	return true;
	
fail:

	return false;
}


void CMds::loadMeshTextures(CModelInstance *instance)
{
#pragma unused (instance)
}

#pragma mark -

void CMds::Draw(CModelInstance *instance, renderingAttributes_t *rendering)
{	

	// iterate over surfaces
	for (int surfaceIndex = 0; surfaceIndex < _header->numSurfaces; surfaceIndex++) {
		DrawSurface(instance, rendering, &_surfaces[surfaceIndex]);
	}
}

void CMds::DrawSurface(CModelInstance *instance, renderingAttributes_t *rendering , MDSSurface *surface)
{	
	MDSPoly 	*triangles = surface->_triangles;
	MDSVertex 	*vertices = surface->_vertices;
	MDSBoneRef 	*bonerefs = surface->_bonerefs;

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, (float*)gShaderArrays.norms);

    gShaderArrays.numcolours = gShaderArrays.numverts = gShaderArrays.numelems = 0;
	int *outElems = gShaderArrays.elems;
	float *outVerts = (float*)gShaderArrays.verts;
	float *outTexvecs = (float*)gShaderArrays.tex_st;
	float *outNormals = (float*)gShaderArrays.norms;
	
	// push the triangles
	for (int j = 0 ; j < surface->numTris ; j++) 
	{
		*outElems++ = gShaderArrays.numverts + triangles->vind[0];
		*outElems++ = gShaderArrays.numverts + triangles->vind[1];
		*outElems++ = gShaderArrays.numverts + triangles->vind[2];
		triangles++;
	}
	gShaderArrays.numelems += surface->numTris * 3;
	
	// push the vertices, normals, and texture coordinates
	for (int j = 0; j < surface->numVerts; j++ ) 
	{
		vec3_t	tempVert, tempNormal;

		vec_clear( tempVert );
		vec_clear( tempNormal );
		
		tempVert[0] = vertices->normal[0];
		tempVert[1] = vertices->normal[1];
		tempVert[2] = vertices->normal[2];

		*outVerts++ = tempVert[0] * 64.0;
		*outVerts++ = tempVert[1] * 64.0;
		*outVerts++ = tempVert[2] * 64.0;

		*outTexvecs++ = vertices->texCoords[0];
		*outTexvecs++ = vertices->texCoords[1];

/*
		double u = vertices->_unknown1;
		double v = vertices->_unknown2;
		double alpha = u * 2.0f * pi / 255.0f;
		double beta =  v * 2.0f * pi / 255.0f;
			
		*outNormals++ = (float)(cos(beta) * sin(alpha)) ;  	  
		*outNormals++ = (float)(sin(beta) * sin(alpha)) ;
		*outNormals++ = (float)cos(alpha) ;
*/

		vertices++;
	}
	gShaderArrays.numverts += surface->numVerts;
	
		glDisable( GL_TEXTURE_2D );
		CShader::wireframeFlush();

#if 0
    if (rendering->_textured)	
		glEnable( GL_TEXTURE_2D );
	else 
		glDisable( GL_TEXTURE_2D );
		
	// apply all the shaders
	Boolean shaderfound = false;
	CShader *shader = resources()->shaderWithName(surface->shader);
    if (shader) {
		shader->renderFlush(rendering, 0);
		shaderfound = true;
	}

	// if shader is missing draw untextured wireframe
	if (!shaderfound && !rendering->_renderTextureCoordinates) {
		glDisable( GL_TEXTURE_2D );
	    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); 
		CShader::wireframeFlush();
	}
	
	if (rendering->_renderOpaque && rendering->_showNormals)
		DrawVertexNormals(rendering, surface->numVerts, (float*)gShaderArrays.verts, (float*)gShaderArrays.norms);
#endif
}


void CMds::DrawLinks(CModelInstance *instance, renderingAttributes_t *rendering) 
{	
}

#pragma mark -


#pragma mark -

inline UInt32 CMds::tagNum()
{
	return 0;
}

inline UInt32 CMds::meshNum()
{
	return 0;
}

ModelType CMds::modelType()
{
	return unknown_model;
}

inline md3_tag_t *CMds::tagForFrameAtIndex(short framenum, short iTagIndex)
{
	md3_tag_t			*dest;
	return dest;
}

string 	CMds::tagNameForFrameAtIndex(short frame, short index)
{
	return "";
}

int CMds::tagIndexWithName(const char *name)
{
	return -1;
}

SInt16 CMds::frameCount() 
{
	return 0;	
}

#pragma mark -

Boolean CMds::canExportFormat(ExportFormatT format)
{
	switch (format) {
		case WAVEFRONT_OBJ_FORMAT:
		case AUTOCAD_DXF_FORMAT:
			return false;
	}
	return false;
}


