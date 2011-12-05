/* 
	CMd3.cpp

	Author:			Tom Naughton
	Description:	<describe the CMd3 class here>
*/

#include <agl.h>
#include <glut.h>

#include "CMd3.h"
#include "CShader.h"
#include "C3DModelPane.h"
#include "CPakStream.h"
#include "utilities.h"
#include "CPakRatApp.h"
#include "CPreferences.h"
#include "CModelInstance.h"
#include "CResourceManager.h"
#include "CTokenizer.h"
#include "CMd3AnimationInfo.h"
#include "AppConstants.h"


extern "C" {
	#include "matrix.h"
}

CMd3::CMd3(CResourceManager *resources) : C3DModel(resources)
{
	_animationInfo = nil;
	_md3Data = nil;
	_md3 = nil;
	_modelType = unknown_model;
}

CMd3::~CMd3()
{
	dprintf("~CMd3()\n");
	// throw away model
	if (_md3) {
	
		// throw away meshes
		if (_md3->meshes) {
			for (int i=0; i<_md3->header->meshNum; i++) {
				if (_md3->meshes[i])
					CMemoryTracker::safeFree(_md3->meshes[i]);
			}
			CMemoryTracker::safeFree(_md3->meshes);
		}
		CMemoryTracker::safeFree(_md3);
		_md3 = nil;
	}
	
	if (_md3Data)
		CMemoryTracker::safeFree(_md3Data);
	_md3Data = nil;
}

void CMd3::Draw(CModelInstance *instance, renderingAttributes_t *renderingAttributes)
{	
	if (instance->_currentFrame >= _md3->header->boneFrameNum)
		instance->_currentFrame = 0;

	for (int i = 0; i < _md3->header->meshNum; ++i)
		DrawMesh(instance, renderingAttributes, i);	
				
	if(renderingAttributes->_showBoneFrameBox)
		DrawBoneFrameBox(instance, renderingAttributes);
		
	// draw links after the blend phase
	if (renderingAttributes->_pickTag && renderingAttributes->_renderBlend) {
	//	dprintf("\n\ndrawing links...\n");
		DrawLinks(instance, renderingAttributes);
	}
}

Mat4 CMd3::PushTagTransformMatrix(CModelInstance *instance, int tagindex)
{
	Mat4 transform, result;
	float s[16];
	float *m = s;
	
	if (instance->_interpolationFraction != 0.0f) {
		transform = interpolateTransformation(
			tagForFrameAtIndex(instance->_currentFrame, tagindex), 
			tagForFrameAtIndex(instance->_nextFrame, tagindex), 
			instance->_interpolationFraction);
	} else {
		md3_tag_t *tag = tagForFrameAtIndex(instance->_currentFrame, tagindex);
		
	//	result = Mat4::matrixFromQuatPos(tag->Rotation, tag->Position);
		result = Mat4::matrixFromQuat(tag->Rotation);
		transform = result.transpose();
		vec *mv = transform.raw();

		MAT(mv,0,3) = tag->Position[0];
		MAT(mv,1,3) = tag->Position[1];
		MAT(mv,2,3) = tag->Position[2];
	}
					
  	//switch to child coord system and draw child
  	glPushMatrix();
  	glMultMatrixf(transform.raw());
  	
  	return result;
}


void CMd3::DrawLinks(CModelInstance *instance, renderingAttributes_t *rendering) 
{	
	// FIXME
	// flash models have tags (tag_weapon etc.) which
	// overlap and conflict with ancestor tags, shouldn't draw tags
	// on flash models

	glDisable( GL_ALPHA_TEST);
	glDisable( GL_BLEND);
	glDisable( GL_DEPTH_TEST);
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );

	if (instance->_currentFrame >= _md3->header->boneFrameNum)
		instance->_currentFrame = 0;

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

void CMd3::DrawBoneFrameBox(CModelInstance *instance, renderingAttributes_t *rendering) 
{
	
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	
	md3_boneFrame_t	*bf = &_md3->boneframes[instance->_currentFrame];
				
	if (instance->_interpolationFraction != 0.0f) {
		bf = interpolateBoneFrame(
			&_md3->boneframes[instance->_currentFrame], 
			&_md3->boneframes[instance->_nextFrame], 
			instance->_interpolationFraction);
	}
	
	
	float x1=bf->mins.x, y1=bf->mins.y, z1=bf->mins.z,
				x2=bf->maxs.x, y2=bf->maxs.y, z2=bf->maxs.z;
    
    	if (rendering->_renderOpaque) {
    		glDisable(GL_DEPTH_TEST);
			glPointSize(6);
			glColor3f(1,0,0);
			glBegin(GL_POINTS);
			glVertex3f(bf->position.x, bf->position.y, bf->position.z);		
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

void CMd3::DrawVertexNormals(
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
  	for (int i=0;i< vertexNum;i++) {

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

void CMd3::DrawMesh(CModelInstance *instance, renderingAttributes_t *rendering, UInt32 meshnum) 
{
	md3_mesh_t *mesh = _md3->meshes[meshnum];
	if (!mesh || strlen(mesh->header->name) == 0) {
		dprintf("no mesh data\n");
		return;	
	}
		
	md3_vertex_normal_t *currMeshFrame = mesh->vertices + (mesh->header->vertexNum * instance->_currentFrame); 
	md3_vertex_normal_t *nextMeshFrame = mesh->vertices + (mesh->header->vertexNum * instance->_nextFrame);

	CShader *shader = instance->textureForMeshAtIndex(meshnum);
	
	if (!shader) {
		string textureName = instance->textureNameForMesh(mesh->header->name);
	  	if (textureName.length() > 0) {
			shader = resources()->shaderWithName(textureName.c_str());
			instance->_meshTextures[meshnum] = shader;
		}
	}
		
	UInt32 *elem;
	UInt32 k;
	Boolean shouldDraw = C3DModel::shouldDraw(rendering, shader);
	Boolean needsNormals = C3DModel::needsNormals(rendering, shader);
	float frac = instance->_interpolationFraction;
	UInt32 numelems = mesh->header->triangleNum * 3;
	
	// FIXME - shouldn't do this here?
    gShaderArrays.numcolours = gShaderArrays.numverts = gShaderArrays.numelems = 0;
	
	if (!shouldDraw)
		return;
		
	if (rendering->_lighting) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, (float*)gShaderArrays.norms);
	} else {
		glDisableClientState(GL_NORMAL_ARRAY);
	}

	int *outElems = gShaderArrays.elems;
    elem = (UInt32*)mesh->triangles;
    for (k = 0; k < numelems; k++) {
		// FIXME - gShaderArrays.numverts is always zero?
		*outElems++ = gShaderArrays.numverts + *elem++;
    }
    gShaderArrays.numelems = numelems;
	
	float *outVerts = (float*)gShaderArrays.verts;
	float *outTexvecs = (float*)gShaderArrays.tex_st;
	float *outNormals = (float*)gShaderArrays.norms;
	md3_texvec_t *inTexVecs = mesh->texvecs;
	
	
    for (k = 0; k < mesh->header->vertexNum; k++) {
    
		// interpolate and scale vertices	
		*outVerts++ = ((1.0 - frac) * currMeshFrame->x + frac * nextMeshFrame->x) / 64.0f;
		*outVerts++ = ((1.0 - frac) * currMeshFrame->y + frac * nextMeshFrame->y) / 64.0f;
		*outVerts++ = ((1.0 - frac) * currMeshFrame->z + frac * nextMeshFrame->z) / 64.0f;

		*outTexvecs++ = inTexVecs->u;
		*outTexvecs++ = inTexVecs->v;
			
		if (needsNormals) {		
		
			double u = ((1.0f - frac) * currMeshFrame->u + frac * nextMeshFrame->u);
			double v = ((1.0f - frac) * currMeshFrame->v + frac * nextMeshFrame->v);
			double alpha = u * 2.0f * pi / 255.0f;
			double beta =  v * 2.0f * pi / 255.0f;
			
			*outNormals++ = (float)(cos(beta) * sin(alpha)) ;  	  
			*outNormals++ = (float)(sin(beta) * sin(alpha)) ;
			*outNormals++ = (float)cos(alpha) ;
		}
	
		// Push the entity colour (TEST )
		//colour_copy ( shader->shaderRGBA , gShaderArrays.entity_colour [gShaderArrays.numverts]);
		//memset (gShaderArrays.entity_colour [gShaderArrays.numverts],255,4);
		//memset (gShaderArrays.colour [gShaderArrays.numverts],255,4);

		inTexVecs++;
		currMeshFrame++;
		nextMeshFrame++;
		gShaderArrays.numverts++;
    }
    
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
		DrawVertexNormals(rendering, mesh->header->vertexNum, (float*)gShaderArrays.verts, (float*)gShaderArrays.norms);
}



double *CMd3::interpolateMeshFrame(
	md3_vertex_normal_t *currMeshFrame, 
	md3_vertex_normal_t *nextMeshFrame, 
	double frac, UInt32 vertexNum) 
{
	static double *interpolatedMeshFrame = nil;
	static int allocatedVertexNum = 0;
	
	if (!interpolatedMeshFrame || allocatedVertexNum < vertexNum) {
		if (interpolatedMeshFrame)
			CMemoryTracker::safeFree(interpolatedMeshFrame);
		interpolatedMeshFrame = (double*) CMemoryTracker::safeAlloc(4 * vertexNum, sizeof(double), "interpolateMeshFrame", false);	
	}
	double *interp = interpolatedMeshFrame;

	if (frac > 0.1d) {		 
		// interpolate and scale vertices	
		for (int t=0;t<vertexNum;t++) {
			*interp++ = ((1.0 - frac) * currMeshFrame[t].x + frac * nextMeshFrame[t].x) / 64.0f;
			*interp++ = ((1.0 - frac) * currMeshFrame[t].y + frac * nextMeshFrame[t].y) / 64.0f;
			*interp++ = ((1.0 - frac) * currMeshFrame[t].z + frac * nextMeshFrame[t].z) / 64.0f;
			*interp++ = 1.0;
		}
	} else {
		// just scale vertices	
		for (int t=0;t<vertexNum;t++) {
			*interp++ = currMeshFrame[t].x / 64.0f;
			*interp++ = currMeshFrame[t].y / 64.0f;
			*interp++ = currMeshFrame[t].z / 64.0f;
			*interp++ = 1.0;
		}
	}

	return interpolatedMeshFrame;

fail:

	return nil; 
}


md3_boneFrame_t *CMd3::interpolateBoneFrame(md3_boneFrame_t *currBoneFrame, md3_boneFrame_t *nextBoneFrame, float frac) 
{
	static md3_boneFrame_t tmpBoneFrame_1;
	tmpBoneFrame_1.mins.x = (1.0f - frac) * currBoneFrame->mins.x + frac * nextBoneFrame->mins.x;
	tmpBoneFrame_1.maxs.x = (1.0f - frac) * currBoneFrame->maxs.x + frac * nextBoneFrame->maxs.x;
	tmpBoneFrame_1.position.x = (1.0f - frac) * currBoneFrame->position.x + frac * nextBoneFrame->position.x;
	tmpBoneFrame_1.mins.y = (1.0f - frac) * currBoneFrame->mins.y + frac * nextBoneFrame->mins.y;
	tmpBoneFrame_1.maxs.y = (1.0f - frac) * currBoneFrame->maxs.y + frac * nextBoneFrame->maxs.y;
	tmpBoneFrame_1.position.y = (1.0f - frac) * currBoneFrame->position.y + frac * nextBoneFrame->position.y;
	tmpBoneFrame_1.mins.z = (1.0f - frac) * currBoneFrame->mins.z + frac * nextBoneFrame->mins.z;
	tmpBoneFrame_1.maxs.z = (1.0f - frac) * currBoneFrame->maxs.z + frac * nextBoneFrame->maxs.z;
	tmpBoneFrame_1.position.z = (1.0f - frac) * currBoneFrame->position.z + frac * nextBoneFrame->position.z;
		
	return &tmpBoneFrame_1;
}
  

Mat4 CMd3::interpolateTransformation(md3_tag_t *currFrameTag, md3_tag_t *nextFrameTag, float frac) 
{    
	float frac1=1.f-frac;

	Mat4 matrix;

	Vec3 interPos = currFrameTag->Position*frac1 + nextFrameTag->Position*frac;		     // interpoalte position
	Vec4 squat = Vec4::quatSlerp( currFrameTag->Rotation, nextFrameTag->Rotation, frac ); // interpolate rotation matrix		

	matrix = Mat4::matrixFromQuat( squat ).transpose();
	vec *mv = matrix.raw();
	MAT(mv,0,3) = interPos[0];
	MAT(mv,1,3) = interPos[1];
	MAT(mv,2,3) = interPos[2];	
	
/* ??
	matrix = Mat4::matrixFromQuatPos(squat, interPos).transpose();
*/	
	return matrix;
}

#pragma mark -

Boolean CMd3::init(CPakStream *inItem)
{
	_pathname = inItem->pathName();
           
    // lock and load                                                                                                                                                                                                                                                                                                                                                                                                                                                                 	
	_md3Data = (char*) inItem->getData("md3 data");
	if (!_md3Data )
		goto fail;
	
	_md3 = (md3_t*)CMemoryTracker::safeAlloc(1, sizeof(md3_t), "md3");
	if (!_md3)
		goto fail;
		
	// md3 header
	md3_header_t *md3header = (md3_header_t*) _md3Data;
	_md3->header = md3header;

	// check compatibility
	if (md3header->ident != 'IDP3') {
		dprintf("wrong model type\n");
		goto fail;
	}
		
	if (    (swapLong(md3header->version) != 15 ) 
		||  (swapLong(md3header->fileSize) < swapLong(md3header->tagStart) ) 
		||  (swapLong(md3header->fileSize) < swapLong(md3header->meshStart) )) {
		dprintf("corrupt md3 file header\n");
		goto fail;
	}
		
	// swap header
	md3header->version = swapLong(md3header->version);
	md3header->boneFrameNum = swapLong(md3header->boneFrameNum);
	md3header->tagNum = swapLong(md3header->tagNum);
	md3header->meshNum = swapLong(md3header->meshNum);
	md3header->maxTextureNum = swapLong(md3header->maxTextureNum);
	md3header->boneFrameStart = swapLong(md3header->boneFrameStart);
	md3header->tagStart = swapLong(md3header->tagStart);
	md3header->meshStart = swapLong(md3header->meshStart);
	md3header->fileSize = swapLong(md3header->fileSize);
	
/*
    dprintf("md3header->version %d\n", md3header->version);
    dprintf("md3header->name %s\n", md3header->name);
    dprintf("md3header->boneFrameNum %d\n", md3header->boneFrameNum);
    dprintf("md3header->tagNum %d\n", md3header->tagNum);
    dprintf("md3header->meshNum %d\n", md3header->meshNum);
    dprintf("md3header->maxTextureNum %d\n", md3header->maxTextureNum);
    dprintf("md3header->boneFrameStart %d\n", md3header->boneFrameStart);
    dprintf("md3header->tagStart %d\n", md3header->tagStart);
    dprintf("md3header->meshStart %d\n", md3header->meshStart);
    dprintf("md3header->fileSize %d\n", md3header->fileSize);
*/
	
	_md3->boneframes = (md3_boneFrame_t*) (_md3Data + md3header->boneFrameStart);
	_md3->tags = (md3_tag_t*) (_md3Data + md3header->tagStart);
	_md3->meshData = (char*) (_md3Data + md3header->meshStart);

	// swap MD3BoneFrames
	for (int i=0; i< md3header->boneFrameNum; i++) {
		_md3->boneframes[i].mins.x = swapFloat(_md3->boneframes[i].mins.x);
		_md3->boneframes[i].mins.y = swapFloat(_md3->boneframes[i].mins.y);
		_md3->boneframes[i].mins.z = swapFloat(_md3->boneframes[i].mins.z);

		_md3->boneframes[i].maxs.x = swapFloat(_md3->boneframes[i].maxs.x);
		_md3->boneframes[i].maxs.y = swapFloat(_md3->boneframes[i].maxs.y);
		_md3->boneframes[i].maxs.z = swapFloat(_md3->boneframes[i].maxs.z);

		_md3->boneframes[i].position.x = swapFloat(_md3->boneframes[i].position.x);
		_md3->boneframes[i].position.y = swapFloat(_md3->boneframes[i].position.y);
		_md3->boneframes[i].position.z = swapFloat(_md3->boneframes[i].position.z);
		
		_md3->boneframes[i].scale = swapFloat(_md3->boneframes[i].scale);
	}


	// swap MD3Tags 
	for (int i=0; i<md3header->boneFrameNum; i++) {
		for (int j=0; j<md3header->tagNum; j++) {
			md3_tag_t *tag = _md3->tags + (md3header->tagNum * i) + j;
			Mat4 matrix = identity;

			float *f = (float*)&tag->Position;
			Vec3 pos;
			pos[ 0 ]	= swapFloat(*f++);
			pos[ 1 ]	= swapFloat(*f++);
			pos[ 2 ]	= swapFloat(*f++);
			tag->Position = pos;
			
			// cram a quatMatrix in the same place as the 3x3 matrix
			f = (float*)&tag->Rotation;
			float *mv = matrix.m;
			MAT(mv,0,0) = swapFloat(*f++);
			MAT(mv,1,0) = swapFloat(*f++);
			MAT(mv,2,0) = swapFloat(*f++);
			MAT(mv,0,1) = swapFloat(*f++);
			MAT(mv,1,1) = swapFloat(*f++);
			MAT(mv,2,1) = swapFloat(*f++);
			MAT(mv,0,2) = swapFloat(*f++);
			MAT(mv,1,2) = swapFloat(*f++);
			MAT(mv,2,2) = swapFloat(*f++);
			tag->Rotation = Vec4::quatFromMatrix( matrix );
			tag->info.glName = j;

		}
	}	
	

  	// read MD3Meshes
	char *p = (char*) _md3->meshData;
	_md3->meshes = (md3_mesh_t**) CMemoryTracker::safeAlloc (md3header->meshNum, sizeof(md3_mesh_t*), "_md3->meshes");
	if (!_md3->meshes)
		goto fail;
		
	for (int i=0; i<md3header->meshNum; i++) {
		_md3->meshes[i] = loadMesh(p);
		if (!_md3->meshes[i])
			goto fail;
	}
	
	return true;
	
fail:

	return false;
}


CMd3AnimationInfo *CMd3::animationInfo() 
{
	if (!_animationInfo) {
		string package, file, extension;
		// look for animation.cfg file
		decomposeEntryName(_pathname, package, file, extension);
		string animationFileName = package + "animation.cfg";
		_animationInfo = resources()->animationInfoWithName(animationFileName.c_str());
	}
	return _animationInfo;
}


void CMd3::loadMeshTextures(CModelInstance *instance)
{
	md3_header_t *md3header = _md3->header;

	for (int i=0; i<md3header->meshNum; i++) {
		md3_mesh_t  *mesh = _md3->meshes[i]; 
		if (mesh) {
			md3_mesh_header_t *meshHeader = mesh->header;
		
			// load textures
			for (int i=0; i<meshHeader->textureNum; i++) {
				md3_texture_t *texture = mesh->textures + i;

				if (strlen(texture->name)) {
					CShader *t = resources()->shaderWithName(texture->name);
					if (t) {
						instance->setTextureNameForMesh(texture->name, meshHeader->name);
					}
				}
			}
		}
	}
}


md3_mesh_t *CMd3::loadMesh(char *&p)  
{
	md3_mesh_header_t *meshHeader = (md3_mesh_header_t*) p;
	md3_mesh_t *mesh = (md3_mesh_t*)CMemoryTracker::safeAlloc(1, sizeof(md3_mesh_t),"md3_mesh_t");
	if (!mesh)
		goto fail;
		
	mesh->glName = nextGLName();
	mesh->header = meshHeader;
  	
    //start reading mesh
    meshHeader->meshFrameNum = swapLong(meshHeader->meshFrameNum);
    meshHeader->textureNum = swapLong(meshHeader->textureNum);
    meshHeader->vertexNum = swapLong(meshHeader->vertexNum);
    meshHeader->triangleNum = swapLong(meshHeader->triangleNum);
    meshHeader->triangleStart = swapLong(meshHeader->triangleStart);
    meshHeader->textureStart = swapLong(meshHeader->textureStart);
    meshHeader->texVecStart = swapLong(meshHeader->texVecStart);
    meshHeader->vertexStart = swapLong(meshHeader->vertexStart);
    meshHeader->meshSize = swapLong(meshHeader->meshSize);
    
    /*
    dprintf("meshHeader->meshFrameNum %d\n", meshHeader->meshFrameNum);
    dprintf("meshHeader->textureNum %d\n", meshHeader->textureNum);
    dprintf("meshHeader->vertexNum %d\n", meshHeader->vertexNum);
    dprintf("meshHeader->triangleNum %d\n", meshHeader->triangleNum);
    dprintf("meshHeader->triangleStart %d\n", meshHeader->triangleStart);
    dprintf("meshHeader->textureStart %d\n", meshHeader->textureStart);
    dprintf("meshHeader->texVecStart %d\n", meshHeader->texVecStart);
    dprintf("meshHeader->vertexStart %d\n", meshHeader->vertexStart);
    dprintf("meshHeader->meshSize %d\n", meshHeader->meshSize);
	*/
	
	// check for errors
	if (meshHeader->ident != 'IDP3') {
		dprintf("wrong mesh type");
		goto fail;
	}
		
      if ( ( meshHeader->meshSize <= meshHeader->triangleStart ) 
      	|| ( meshHeader->meshSize <= meshHeader->texVecStart )
     	|| ( meshHeader->meshSize <= meshHeader->vertexStart )) {        
		dprintf("corrupt mesh header");
		goto fail;
     }
	
	// locate subcomponents	
	mesh->triangles = (md3_triangle_t*) (p + meshHeader->triangleStart);
	mesh->textures = (md3_texture_t*) (p + meshHeader->textureStart);
	mesh->texvecs = (md3_texvec_t*) (p + meshHeader->texVecStart);
	mesh->vertices = (md3_vertex_normal_t*) (p + meshHeader->vertexStart);

	// read triangles of a mesh
	for (int i=0; i< meshHeader->triangleNum; i++) {
	
		md3_triangle_t *triangle = (md3_triangle_t*) ((char*)mesh->triangles + 12*i);
		triangle->index[0] = swapLong(triangle->index[0]);
		triangle->index[1] = swapLong(triangle->index[1]);
		triangle->index[2] = swapLong(triangle->index[2]);
	}
  
	//read texture coord of a mesh
	for (int i=0; i < meshHeader->vertexNum; i++) {
		md3_texvec_t *texvec = mesh->texvecs + i;
		texvec->u = swapFloat(texvec->u);
		texvec->v = swapFloat(texvec->v);
	}
  
	//read mesh vertex frames  
	for (int i=0; i<meshHeader->meshFrameNum; i++) {
		for (int j=0; j<meshHeader->vertexNum; j++) {
			md3_vertex_normal_t *vertex = mesh->vertices + (meshHeader->vertexNum * i) + j;

			vertex->x = swapShort(vertex->x);
			vertex->y = swapShort(vertex->y);
			vertex->z = swapShort(vertex->z);
		//	dprintf("	%d, %d, %d\n", vertex->x, vertex->y, vertex->z);

			/* FIXME - preconvert vertices?
			vertex.x = (float)din.readShort() / 64.0g;
			newMesh.meshFrames[i][j].y = (float)din.readShort() / 64.0g;
			newMesh.meshFrames[i][j].z = (float)din.readShort() / 64.0g;
			*/
			
			// leave u,v alone
		}
	}

	p += meshHeader->meshSize;

    return mesh;
    
fail:
	
	if (mesh)
		CMemoryTracker::safeFree(mesh);
		
	return nil;
}

inline UInt32 CMd3::tagNum()
{
	return _md3->header->tagNum;
}

inline UInt32 CMd3::meshNum()
{
	return _md3->header->meshNum;
}

ModelType CMd3::modelType()
{
	if (_modelType != unknown_model)
		return _modelType;
	
#if 1	
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
#else		
	// figure out what kind of model we got
	if (tagIndexWithName("tag_torso") >= 0 && tagIndexWithName("tag_head") < 0) {
	
		// leg model - torso, but no head
		_modelType = lower_model;
		
	} else if (tagIndexWithName("tag_torso") >= 0 && tagIndexWithName("tag_head") >= 0) {
	
		// torso model - head and torso
		_modelType = upper_model;
		
	} else if (tagIndexWithName("tag_torso") < 0 && tagIndexWithName("tag_head") >= 0) {
	
		// head model - head, but no torso
		_modelType = head_model;
	} else {
	
		_modelType = other_model;

	}
#endif
		
	/* todo: determine these
	
		_modelType = weapon_model;
		_modelType = barrel_model;
		_modelType = flash_model;
		
	*/

	
	return _modelType;
}

#pragma mark -


inline md3_tag_t *CMd3::tagForFrameAtIndex(short frame, short index)
{
	md3_tag_t *tag =  _md3->tags + _md3->header->tagNum * frame + index;
	return tag;
}

inline string 	CMd3::tagNameForFrameAtIndex(short frame, short index)
{
	md3_tag_t *tag =  _md3->tags + _md3->header->tagNum * frame + index;
	return tag->name;
}

inline int CMd3::tagIndexWithName(const char *name)
{
	for (int j=0; j<_md3->header->tagNum; j++) {
		md3_tag_t *tag = _md3->tags + j;
		if (!strcmp(name, tag->name))
			return j;
	}
	return -1;
}

inline SInt16 CMd3::frameCount() 
{
	return _md3->header->boneFrameNum; 
}

#pragma mark -

Boolean CMd3::canExportFormat(ExportFormatT format)
{
	switch (format) {
		case WAVEFRONT_OBJ_FORMAT:
		case AUTOCAD_DXF_FORMAT:
			return true;
	}
	return false;
}

void CMd3::dxfExportMesh(CModelInstance *instance, LFileStream *file, Mat4 transform, int meshnum)
{
	char s[100];
	SInt32 length;
	
	dprintf("exporting mesh %d\n", meshnum);
	md3_mesh_t *mesh = _md3->meshes[meshnum];
	if (!mesh) {
		dprintf("no mesh data\n");
		return;	
	}

	md3_vertex_normal_t *vertices = mesh->vertices + (mesh->header->vertexNum * instance->_currentFrame);
    for (int t=0; t< mesh->header->triangleNum; t++) {

		char *facedef = "0\r\n3DFACE\r\n8\r\n1\r\n";
	  //  dprintf("0\n3DFACE\n8\n1\n");
	    length = strlen(facedef); file->PutBytes(facedef,length); 
	    
    	for (int v = 0; v < 4 ; v++) {
    	
    		// fourth vertex is same as third
			int index = v;
			if (v == 3) index = 2;
			
		    md3_triangle_t *triangle = mesh->triangles + t;
	    	Vec3 tempVec;
	    	tempVec[0] = vertices[triangle->index[index]].x / 64.0f;
	    	tempVec[1] = vertices[triangle->index[index]].y / 64.0f;
	    	tempVec[2] = vertices[triangle->index[index]].z / 64.0f;
	    	
	    	Vec3 tempVec2 = transform * tempVec;
			sprintf(s, "1%d\r\n%f\r\n", v, tempVec2[0]);
		//	dprintf("1%d\n%f\n", v, tempVec2[0]);
		    length = strlen(s); file->PutBytes(s,length);
			sprintf(s, "2%d\r\n%f\r\n", v, tempVec2[1]);
		//	dprintf("2%d\n%f\n", v, tempVec2[1]);
		    length = strlen(s); file->PutBytes(s,length);
			sprintf(s, "3%d\r\n%f\r\n", v, tempVec2[2]);
		//	dprintf("3%d\n%f\n", v, tempVec2[2]);
		    length = strlen(s); file->PutBytes(s,length);
		}
		
	}
}

void CMd3::objExportMeshVertexData(CModelInstance *instance, LFileStream *file, Mat4 transform, int meshnum) 
{
	char 		s[100];
	SInt32		length;
	
	dprintf("objExportMeshVertexData %d\n", meshnum);
	md3_mesh_t *mesh = _md3->meshes[meshnum];
	if (!mesh) {
		dprintf("no mesh data\n");
		return;	
	}

	md3_vertex_normal_t *vertices = mesh->vertices + (mesh->header->vertexNum * instance->_currentFrame);
	for (int i=0; i<mesh->header->vertexNum; i++) {
		//turn vertex into 4x1 matrix
		Vec4 tempVec;

		tempVec[0] = vertices[i].x / 64.0f;
		tempVec[1] = vertices[i].y / 64.0f;
		tempVec[2] = vertices[i].z / 64.0f;
		tempVec[3] = 1;

		//transform vertex
		Vec4 tempVec2 = transform * tempVec;

		//write vertex			
		sprintf(s, "v %f %f %f\n", tempVec2[0], tempVec2[1], tempVec2[2]);
	    length = strlen(s); file->PutBytes(s,length);

		//write texture coord.
		sprintf(s, "vt %f %f\n", mesh->texvecs[i].u, mesh->texvecs[i].v);
	    length = strlen(s); file->PutBytes(s,length);

		//write vertex normal
		double alpha=vertices[i].u * 2.0 * pi / 255.0;
		double beta=vertices[i].v * 2.0 * pi / 255.0;
		float a = (float) (cos(beta) * sin(alpha));
		float b = (float) (sin(beta) * sin(alpha));
		float c = (float) cos(alpha);

		sprintf(s, "vn %f %f %f\n", a, b, c);
	    length = strlen(s); file->PutBytes(s,length);
	}
}	
		
void CMd3::objExportMeshElementData(LFileStream *file, int meshnum, int &vertexCount, int &meshCount)
{
	char s[100];
	SInt32		length;
	
	dprintf("objExportMeshElementData %d\n", meshnum);
	md3_mesh_t *mesh = _md3->meshes[meshnum];
	if (!mesh) {
		dprintf("no mesh data\n");
		return;	
	}
	
	// FIXME mesh names should be uniqued (in the case of three headed models etc.)
	sprintf(s, "g %s\ns %d\nbevel off\nc_interp off\nd_interp off\n", mesh->header->name, meshCount);		
 	length = strlen(s); file->PutBytes(s,length);

	//md3_vertex_normal_t *vertices = mesh->vertices + (mesh->header->vertexNum * instance->_currentFrame);
    for (int t=0; t< mesh->header->triangleNum; t++) {
		md3_triangle_t *triangle = mesh->triangles + t;

		int a = vertexCount + triangle->index[0] + 1;
		int b = vertexCount + triangle->index[1] + 1;
		int c = vertexCount + triangle->index[2] + 1;
		sprintf(s, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",a,a,a,b,b,b,c,c,c);
	 	length = strlen(s); file->PutBytes(s,length);
	}
	vertexCount += mesh->header->vertexNum;
	meshCount++;
}

