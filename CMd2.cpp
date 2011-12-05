/* 
	CMd2.cpp

	Author:			Tom Naughton
	Description:	load and draw Quake II models
*/

#include <agl.h>

#include "CPcx.h"
#include "CMd2.h"
#include "CPakStream.h"
#include "utilities.h"
#include "CPakRatApp.h"
#include "CTextures.h"
#include "CModelInstance.h"



CMd2::CMd2(CResourceManager *resources) : C3DModel(resources)
{
	md2 = nil;
	_modelData = nil;
}


CMd2::~CMd2()
{
	dprintf("~CMd2()\n");
	if (md2)
		CMemoryTracker::safeFree(md2);
	if (_modelData)
		CMemoryTracker::safeFree(_modelData);
	md2 = nil;
}

Boolean CMd2::init(CPakStream *inItem)
{
	mdl2_header_t *md2header;
	char *p;
	int i;

	_glName = nextGLName();
	_modelData = (char*)inItem->getData("md2data");
	md2 = (md2_t*)CMemoryTracker::safeAlloc(1, sizeof(md2_t), "md2");
	if (!md2)
		goto fail;
		
	// md2 header
	md2header = (mdl2_header_t*) _modelData;
	if (!md2header)
		goto fail;
		
	md2->header = md2header;
	md2header->ident = swapLong(md2header->ident);
	md2header->version = swapLong(md2header->version);
	md2header->skinwidth = swapLong(md2header->skinwidth);
	md2header->skinheight = swapLong(md2header->skinheight);
	md2header->framesize = swapLong(md2header->framesize);	// byte size of each frame
	md2header->num_skins = swapLong(md2header->num_skins);
	md2header->num_xyz = swapLong(md2header->num_xyz);
	md2header->num_st = swapLong(md2header->num_st);		//greater than num_xyz for seams
	md2header->num_tris = swapLong(md2header->num_tris);
	md2header->num_glcmds = swapLong(md2header->num_glcmds);	//dwords in strip/fan command list
	md2header->num_frames = swapLong(md2header->num_frames);
	md2header->ofs_skins = swapLong(md2header->ofs_skins);	//each skin is a MAX_SKINNAME string
	md2header->ofs_st = swapLong(md2header->ofs_st);		//byte offset from start for stverts
	md2header->ofs_tris = swapLong(md2header->ofs_tris);	//offset for dtriangles
	md2header->ofs_frames = swapLong(md2header->ofs_frames);	//offset for first frame
	md2header->ofs_glcmds = swapLong(md2header->ofs_glcmds);
	md2header->ofs_end = swapLong(md2header->ofs_end);	//end of file
	
	// skins
	p = _modelData + md2header->ofs_skins;
	for(i = 0; i < md2header->num_skins; i++) {
		CGLImage *texture = loadSkin(inItem, p);
		if (texture) {
			texture->uploadGLImage();
			addTexture(texture);
			p += MAX_SKINNAME;
		}
	}
	
	//  frame info
	md2->frames = (char *) (_modelData + md2header->ofs_frames);

	// GL commands
	md2->glcmds = (long *) (_modelData + md2header->ofs_glcmds);
	
	// triangles
	md2->triangles = (dtriangle_t *) (_modelData + md2header->ofs_tris);
	
	// texture coordinates
	md2->texCoord = (dstvert_t *) (_modelData + md2header->ofs_st);
	

	SwapGLCommands();
	return true;
	
fail:
		
	dprintf("could not load md2\n");
	return false;
}


CGLImage *CMd2::loadSkin(CPakStream *inItem, char *p)
{
	long skinWidth = md2->header->skinwidth;
	long skinHeight = md2->header->skinheight;
	long skinSize = skinWidth * skinHeight;
	CPakStream *pakItem;
	CFileArchive *archive = inItem->rootArchive();
	CGLImage *glImage = nil;
	string skinName = p;
	
	dprintf("CMd2::loadSkin %s\n", p);
	
	if (archive) {
		pakItem = archive->itemWithPathName(skinName.c_str());
		
		// look for the file in the same directory as the model
		if (!pakItem) 
			pakItem = archive->itemWithPathName(fileInDirectory(skinName, inItem->pathName()).c_str());
		
		if (pakItem) {
			glImage = CTextures::loadTexture(pakItem);
			delete pakItem;	
		}
	}
	return glImage;
}

void CMd2::SwapGLCommands()
{
	short vertnum;
	short index;
	long *command;
	short i;
	short vert_index;
	short type;

	command = md2->glcmds;
	
	while ((*command) != 0)
	{
		*command = swapLong(*command);
		if (*command > 0)
			{vertnum = *command; command++; type = 0;}//triangle strip
		else
			{vertnum = - *command; command++; type = 1;}//triangle fan

		if (vertnum<0) 
			vertnum = -vertnum;

		index = 0;
		for (i=0;i<vertnum;i++)
		{
			float *f = (float*)command;
			*f = swapFloat(*f); // u
			*f++;
			*f = swapFloat(*f); // v
			*f++;
			command = (long*) f;
			
			vert_index = swapLong(*command);
			*command++ = vert_index;
						
			index++;
		}
	}
}

string CMd2::frameName(SInt16 framenum)
{
	if (framenum < frameCount()) {
		frame_t *curframe;
		curframe = (frame_t*) ((char*)md2->frames + md2->header->framesize * framenum);
		return string(curframe->name);
	} else {
		return string("outofrange");
	}
}

SInt16 CMd2::frameCount() 
{
	return md2->header->num_frames; 
}

UInt32 CMd2::meshNum()
{
	return 1;
}

void CMd2::DrawTextureMesh (renderingAttributes_t *rendering, CGLImage *texture)
{
	float x,y,z;
	int *elems = (int*)gShaderArrays.elems;
	float *texvecs = (float*)gShaderArrays.tex_st;
    int elem = 0;
    Rect bounds = texture->bounds();
    
	glPushMatrix ();
	x= 0.5;
	y= 0.5;
	z= 0.0;

	if (rendering->_renderTexturePreview) {
		glDisable( GL_LIGHTING );
		glEnable( GL_TEXTURE_2D );
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); 
		glVertex3f(-x, y, z);
		glTexCoord2f(1.0f, 0.0f); 
		glVertex3f( x, y, z);
		glTexCoord2f(1.0f, 1.0f); 
		glVertex3f( x, -y, z);
		glTexCoord2f(0.0f, 1.0f); 
		glVertex3f(-x, -y, z);
		glEnd();	
		if (rendering->_textureMap) {
			if (rendering->_textureMap->NextSize(bounds.right - bounds.left, bounds.bottom - bounds.top))
				rendering->_pickedImage = texture;
			
		}
	}	

	if (rendering->_renderTextureCoordinates) {
		glDisable( GL_LIGHTING );
		glDisable( GL_TEXTURE_2D );
	 	glEnable(GL_LINE_SMOOTH);
	 	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
		glColor3f(1,1,1);
		
		if (rendering->_textureMap)
			rendering->_textureMap->BeginMap();
	  			
		for(int tri = 0; tri < md2->header->num_tris; tri++) {		// tri is which triangle
	    
			if (rendering->_textureMap)
				rendering->_textureMap->BeginTriangle();

		  	glBegin( GL_LINE_LOOP );
			for(int corner = 0; corner < 3; corner++) {
				float s = swapShort(md2->texCoord[swapShort(md2->triangles[tri].index_st[corner])].s);
				float t = swapShort(md2->texCoord[swapShort(md2->triangles[tri].index_st[corner])].t);
				x = (float) s / (float) md2->header->skinwidth;
				y = (float) t / (float) md2->header->skinheight;

				if (rendering->_textureMap)
					rendering->_textureMap->AddVertex(x,y);

		     	x = x - 0.5;
				y = -y + 0.5;
				z = 0.0;
	   			glVertex3f(x ,y ,z);
			}
			glEnd();
			if (rendering->_textureMap)
				rendering->_textureMap->EndTriangle();
		}
    }

	glPopMatrix ();
	glColor4f(1,1,1,1);

}


void CMd2::Draw(CModelInstance *instance, renderingAttributes_t *rendering)
{

	if (rendering->_textured && instance->_textureIndex >= 0 && _textureCount > instance->_textureIndex) {
		CGLImage *glImage = (*_textures)[instance->_textureIndex];
		if (glImage) {
			glImage->bindTexture();
			if ((rendering->_renderTextureCoordinates || rendering->_renderTexturePreview)
				&& _glName == rendering->_pickedName) 
				DrawTextureMesh(rendering, glImage);
				
			if (rendering->_pickedName == _glName)
				rendering->_pickedImage = glImage;
	    }
    } 

	if (!rendering->_renderOpaque)
		return; 
		
	glPushMatrix();
	glDepthMask( GL_TRUE);
	glDisable( GL_BLEND );
	glRotatef( 90.0f, 1.0f ,  0.0f , 0.0f );
	glRotatef( 90.0f, 0.0f ,  1.0f , 0.0f );

	if (!(rendering->_renderTextureCoordinates || rendering->_renderTexturePreview) && rendering->_pickShader && _glName)
		glPushName (_glName);

	short 		vertnum;
	short 		i;
	short 		type;
	Vec3		normal;			// current vertex normal
	Vec3		position;		// current vertex position
	float		interpSet[4];	// set of 4 floats for interpolating (position or normals)

	frame_t *curframe = (frame_t*) ((char*)md2->frames + md2->header->framesize * instance->_currentFrame);
	frame_t *nextframe = (frame_t*) ((char*)md2->frames + md2->header->framesize * instance->_nextFrame);
	long *command = md2->glcmds;

	if (instance->_interpolationFraction > 1.0) 
		instance->_interpolationFraction = 1.0;

	while ((*command) != 0) {

		if (*command > 0)
			{vertnum = *command; command++; type = 0;}//triangle strip
		else
			{vertnum = - *command; command++; type = 1;}//triangle fan
		if (vertnum<0) vertnum = -vertnum;

		if (type==0) 
			glBegin(GL_TRIANGLE_STRIP);
		else
			glBegin(GL_TRIANGLE_FAN);
		
		for (i=0;i<vertnum;i++) {
		
			float u = *((float*)command); command++;
			float v = *((float*)command); command++;
			glTexCoord2f(u, v);
			
			for(int dim = 0; dim < 3; dim++) {
				interpSet[2] = avertexnormals[curframe->verts[ *command ].lightnormalindex][dim];
				interpSet[3] = avertexnormals[nextframe->verts[ *command ].lightnormalindex][dim];
				normal[dim] = Interpolate(interpSet, instance->_interpolationFraction, INTERP_LINEAR);

				interpSet[2] = curframe->verts[ *command ].packedposition[dim] * swapFloat(curframe->scale[dim]) + swapFloat(curframe->translate[dim]) ;
				interpSet[3] = nextframe->verts[ *command ].packedposition[dim] * swapFloat(nextframe->scale[dim]) + swapFloat(nextframe->translate[dim]);

				position[dim] = Interpolate(interpSet, instance->_interpolationFraction, INTERP_LINEAR);
			}
			
			glNormal3f(normal[1], normal[2], normal[0]);
			glVertex3f(position[1], position[2], position[0]);
			command++;
		}
		glEnd();
	}

	if ( !(rendering->_renderTextureCoordinates || rendering->_renderTexturePreview) && rendering->_pickShader && _glName)
		glPopName ();

	glPopMatrix();
}



// this was an attempt to use glDrawElements, it doesn't get the texture coordinates right
// md2 associates the texture coordinates with the triangles, not the vertices
void CMd2::Draw2(CModelInstance *instance, renderingAttributes_t *rendering)
{
	float		interpSet[4];	// set of 4 floats for interpolating (position or normals)

	frame_t *curframe = (frame_t*) ((char*)md2->frames + md2->header->framesize * instance->_currentFrame);
	frame_t *nextframe = (frame_t*) ((char*)md2->frames + md2->header->framesize * instance->_nextFrame);

	glColor3f(1,1,1);
	
	if (rendering->_wireframe) 
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); 
	else 
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    
    if (rendering->_textured)	
		glEnable( GL_TEXTURE_2D );
	else
		glDisable( GL_TEXTURE_2D );
		
	if (rendering->_textured && _textureCount > instance->_textureIndex) {
		CGLImage *glImage = (*_textures)[instance->_textureIndex];
		if (glImage) {
			glImage->bindTexture();
	    }
    } 

	if (rendering->_lighting)
		glEnable( GL_LIGHTING );
	else
		glDisable( GL_LIGHTING );

	if (instance->_interpolationFraction > 1.0) 
		instance->_interpolationFraction = 1.0;
		
//	int			tri;			// triangle iterator
	int			corner;			// triangle corner iterator
	int			dim;			// vector dimension iterator
	float		s,t;			// texture coordinates on skin (0 to skinwidth)
//	float		texel[2];		// texture coord in texture space (0,0 to 1,1)

	UInt32 numelems = md2->header->num_tris * 3;
    gShaderArrays.numcolours = gShaderArrays.numverts = gShaderArrays.numelems = 0;
	int *outElems = gShaderArrays.elems;
	texcoord_t *texCoordLookup = gShaderArrays.lm_st;
    for (int tri = 0; tri < md2->header->num_tris; tri++) {
		for(corner = 0; corner < 3; corner++) {
			int vertIndex =  swapShort(md2->triangles[tri].index_xyz[corner]);
			*outElems++ = vertIndex;
		
			// texture coordinate lookup table
			int texIndex = swapShort(md2->triangles[tri].index_st[corner]);
			texCoordLookup[vertIndex][0] = md2->texCoord[texIndex].s;
			texCoordLookup[vertIndex][1] = md2->texCoord[texIndex].t;
		}
    }
    gShaderArrays.numelems = numelems;
	
	float *outVerts = (float*)gShaderArrays.verts;
	float *outTexvecs = (float*)gShaderArrays.tex_st;
	float *outNormals = (float*)gShaderArrays.norms;
	
	
	for(int vert = 0; vert < md2->header->num_xyz; vert++) {	 	// vert is which vertex

		// dim is dimension x,y,z
		for(dim = 0; dim < 3; dim++) {

			interpSet[2] = avertexnormals[curframe->verts[ vert ].lightnormalindex][dim];
			interpSet[3] = avertexnormals[nextframe->verts[ vert ].lightnormalindex][dim];
			*outNormals++ = Interpolate(interpSet, instance->_interpolationFraction, INTERP_LINEAR);

			interpSet[2] = curframe->verts[ vert ].packedposition[dim] * swapFloat(curframe->scale[dim]) + swapFloat(curframe->translate[dim]) ;
			interpSet[3] = nextframe->verts[ vert ].packedposition[dim] * swapFloat(nextframe->scale[dim]) + swapFloat(nextframe->translate[dim]);

			*outVerts++ = Interpolate(interpSet, instance->_interpolationFraction, INTERP_LINEAR);
			
		}
		s = (float) swapShort(texCoordLookup[ vert ][0]) / (float) md2->header->skinwidth;
		t = (float) swapShort(texCoordLookup[ vert ][1]) / (float) md2->header->skinheight;
		*outTexvecs++ =  s ;
		*outTexvecs++ =  t ;
		
		gShaderArrays.numverts++;
	}

	
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, (float*)gShaderArrays.tex_st);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, (float*)gShaderArrays.norms);
    glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, gShaderArrays.verts);
	glDrawElements(GL_TRIANGLES, gShaderArrays.numelems, GL_UNSIGNED_INT, gShaderArrays.elems);
	
	
	glFlush();
}


// Draw model without using GL commands
// it's slow because it interpolates the same vertices multiple times
void CMd2::Draw3(CModelInstance *instance, renderingAttributes_t *rendering)
{
	Vec3		normal;			// current vertex normal
	Vec3		position;		// current vertex position
	float		interpSet[4];	// set of 4 floats for interpolating (position or normals)

	frame_t *curframe = (frame_t*) ((char*)md2->frames + md2->header->framesize * instance->_currentFrame);
	frame_t *nextframe = (frame_t*) ((char*)md2->frames + md2->header->framesize * instance->_nextFrame);

	glColor3f(1,1,1);
	
	if (rendering->_wireframe) 
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); 
	else 
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    
    if (rendering->_textured)	
		glEnable( GL_TEXTURE_2D );
	else
		glDisable( GL_TEXTURE_2D );
		
	if (rendering->_textured && _textureCount > instance->_textureIndex) {
		CGLImage *glImage = (*_textures)[instance->_textureIndex];
		if (glImage) {
			glImage->bindTexture();
	    }
    } 

	if (rendering->_lighting)
		glEnable( GL_LIGHTING );
	else
		glDisable( GL_LIGHTING );

	if (instance->_interpolationFraction > 1.0) 
		instance->_interpolationFraction = 1.0;
		
	int			tri;			// triangle iterator
	int			corner;			// triangle corner iterator
	int			dim;			// vector dimension iterator
	int			s,t;			// texture coordinates on skin (0 to skinwidth)
	float		texel[2];		// texture coord in texture space (0,0 to 1,1)

	for(tri = 0; tri < md2->header->num_tris; tri++) {		// tri is which triangle
		glBegin(GL_TRIANGLES);
		// 3 verticies per triangle; corner is which corner of triangle tri
		// reverse the order so that the triangles are facing outwards
		for(corner = 0; corner < 3; corner++) {
			// dim is dimension x,y,z
			for(dim = 0; dim < 3; dim++) {
			
				interpSet[2] = avertexnormals[curframe->verts[ swapShort(md2->triangles[tri].index_xyz[corner]) ].lightnormalindex][dim];
				interpSet[3] = avertexnormals[nextframe->verts[ swapShort(md2->triangles[tri].index_xyz[corner]) ].lightnormalindex][dim];
				normal[dim] = Interpolate(interpSet, instance->_interpolationFraction, INTERP_LINEAR);

				interpSet[2] = curframe->verts[ swapShort(md2->triangles[tri].index_xyz[corner]) ].packedposition[dim] * swapFloat(curframe->scale[dim]) + swapFloat(curframe->translate[dim]) ;
				interpSet[3] = nextframe->verts[ swapShort(md2->triangles[tri].index_xyz[corner]) ].packedposition[dim] * swapFloat(nextframe->scale[dim]) + swapFloat(nextframe->translate[dim]);

				position[dim] = Interpolate(interpSet, instance->_interpolationFraction, INTERP_LINEAR);
				
			}
			s = swapShort(md2->texCoord[swapShort(md2->triangles[tri].index_st[corner])].s);
			t = swapShort(md2->texCoord[swapShort(md2->triangles[tri].index_st[corner])].t);
			
			texel[0] = (float) s / (float) md2->header->skinwidth;
			texel[1] = (float) t / (float) md2->header->skinheight;
			glTexCoord2f(texel[0], texel[1]);
			glNormal3f(normal[1], normal[2], normal[0]);
			glVertex3f(position[1], position[2], position[0]);
		}
		glEnd();
	}
	glFlush();
}

#pragma mark -


Boolean CMd2::canExportFormat(ExportFormatT format)
{
	switch (format) {
		case WAVEFRONT_OBJ_FORMAT:
		case AUTOCAD_DXF_FORMAT:
			return true;
	}
	return false;
}

void CMd2::dxfExportMesh(CModelInstance *instance, LFileStream *file, Mat4 transform, int meshnum)
{
#pragma unused (meshnum)
	int			tri;			// triangle iterator
	int			dim;			// vector dimension iterator
	SInt32		length;

	Vec3		normal;			// current vertex normal
	Vec3		position;		// current vertex position
	float		interpSet[4];	// set of 4 floats for interpolating (position or normals)

	frame_t *curframe = (frame_t*) ((char*)md2->frames + md2->header->framesize * instance->_currentFrame);
	frame_t *nextframe = (frame_t*) ((char*)md2->frames + md2->header->framesize * instance->_nextFrame);
	char str[100];


	if (instance->_interpolationFraction > 1.0) 
		instance->_interpolationFraction = 1.0;
	

	for(tri = 0; tri < md2->header->num_tris; tri++)		// tri is which triangle
	{
		char *facedef = "0\r\n3DFACE\r\n8\r\n1\r\n";
		length = strlen(facedef); file->PutBytes(facedef,length);
	    
		for(int v = 0; v < 4; v++)
		{
	    		// fourth vertex is same as third
				int corner = v;
				if (v == 3) corner = 2;
		
			// dim is dimension x,y,z
			for(dim = 0; dim < 3; dim++)
			{
				

				interpSet[2] = avertexnormals[curframe->verts[ swapShort(md2->triangles[tri].index_xyz[corner]) ].lightnormalindex][dim];
				interpSet[3] = avertexnormals[nextframe->verts[ swapShort(md2->triangles[tri].index_xyz[corner]) ].lightnormalindex][dim];
				normal[dim] = Interpolate(interpSet, instance->_interpolationFraction, INTERP_LINEAR);

				interpSet[2] = curframe->verts[ swapShort(md2->triangles[tri].index_xyz[corner]) ].packedposition[dim] * swapFloat(curframe->scale[dim]) + swapFloat(curframe->translate[dim]) ;
				interpSet[3] = nextframe->verts[ swapShort(md2->triangles[tri].index_xyz[corner]) ].packedposition[dim] * swapFloat(nextframe->scale[dim]) + swapFloat(nextframe->translate[dim]);

				position[dim] = Interpolate(interpSet, instance->_interpolationFraction, INTERP_LINEAR);
			}
			Vec3 tempVec = position;
			position = transform * tempVec;
				
			sprintf(str, "1%d\r\n%f\r\n", v, position[0]);
		    length = strlen(str); file->PutBytes(str,length);
			sprintf(str, "2%d\r\n%f\r\n", v, position[1]);
		    length = strlen(str); file->PutBytes(str,length);
			sprintf(str, "3%d\r\n%f\r\n", v, position[2]);
		    length = strlen(str); file->PutBytes(str,length);
		}
	}
}

void CMd2::objExportMeshVertexData(CModelInstance *instance, LFileStream *file, Mat4 transform, int meshnum) 
{
#pragma unused (meshnum)
	int			s,t;			// texture coordinates on skin (0 to skinwidth)
	float		texel[2];		// texture coord in texture space (0,0 to 1,1)
	Vec3		normal;			// current vertex normal
	int			dim;			// vector dimension iterator
	SInt32		length;
	Vec3		position;		// current vertex position
	float		interpSet[4];	// set of 4 floats for interpolating (position or normals)
	frame_t *curframe = (frame_t*) ((char*)md2->frames + md2->header->framesize * instance->_currentFrame);
	frame_t *nextframe = (frame_t*) ((char*)md2->frames + md2->header->framesize * instance->_nextFrame);
	char str[100];


	if (instance->_interpolationFraction > 1.0) 
		instance->_interpolationFraction = 1.0;

	for(int vert = 0; vert < md2->header->num_xyz; vert++) {	 	// vert is which vertex

		// dim is dimension x,y,z
		for(dim = 0; dim < 3; dim++) {

			interpSet[2] = avertexnormals[curframe->verts[ vert ].lightnormalindex][dim];
			interpSet[3] = avertexnormals[nextframe->verts[ vert ].lightnormalindex][dim];
			normal[dim] = Interpolate(interpSet, instance->_interpolationFraction, INTERP_LINEAR);

			interpSet[2] = curframe->verts[ vert ].packedposition[dim] * swapFloat(curframe->scale[dim]) + swapFloat(curframe->translate[dim]) ;
			interpSet[3] = nextframe->verts[ vert ].packedposition[dim] * swapFloat(nextframe->scale[dim]) + swapFloat(nextframe->translate[dim]);

			position[dim] = Interpolate(interpSet, instance->_interpolationFraction, INTERP_LINEAR);
			
		}
		s = swapShort(md2->texCoord[ vert ].s);
		t = swapShort(md2->texCoord[ vert ].t);
		
		texel[0] = (float) s / (float) md2->header->skinwidth;
		texel[1] = (float) t / (float) md2->header->skinheight;
		Vec3 tempVec = position;
		position = transform * tempVec;
		
		//write vertex			
		sprintf(str, "v %f %f %f\n", position[1], position[2], position[0]);
	    length = strlen(str); file->PutBytes(str,length);
		
		//write texture coord.
		sprintf(str, "vt %f %f\n", texel[0], texel[1]);
	    length = strlen(str); file->PutBytes(str,length);

		//write vertex normal
		sprintf(str, "vn %f %f %f\n", normal[1], normal[2], normal[0]);
	    length = strlen(str); file->PutBytes(str,length);
	}
}	
		
void CMd2::objExportMeshElementData(LFileStream *file, int meshnum, int &vertexCount, int &meshCount)
{
#pragma unused (meshnum, vertexCount, meshCount)
	char	 	str[100];
	SInt32		length;
		
	// FIXME mesh names should be uniqued (in the case of three headed models etc.)
	sprintf(str, "g %s\ns %d\nbevel off\nc_interp off\nd_interp off\n", "mdl", 0);		
 	length = strlen(str); file->PutBytes(str,length);

	for(int tri = 0; tri < md2->header->num_tris; tri++)		// tri is which triangle
	{
		int a = swapShort(md2->triangles[tri].index_xyz[0]) + 1;
		int b = swapShort(md2->triangles[tri].index_xyz[1]) + 1;
		int c = swapShort(md2->triangles[tri].index_xyz[2]) + 1;
		sprintf(str, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",a,a,a,b,b,b,c,c,c);
	 	length = strlen(str); file->PutBytes(str,length);
	}
}



