/* 
	CHLMdl.cpp

	Author:			Tom Naughton
	Description:	based on code by Scott Gilroy, converted to C++, added exporting
*/

/***
*
*	Copyright (c) 1998, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <glut.h>
#include <agl.h>

#include "CPakRatApp.h"
#include "CPreferences.h"
#include "CHLMdl.h"
#include "CModelInstance.h"
#include "CPakStream.h"
#include "CGLImage.h"
#include "CPcx.h"
#include "utilities.h"


////////////////////////////////////////////////////////////////////////

vec3_t			g_xformverts[MAXSTUDIOVERTS];	// transformed vertices
vec3_t			g_lightvalues[MAXSTUDIOVERTS];	// light surface normals
vec3_t			*g_pxformverts;
vec3_t			*g_pvlightvalues;

vec3_t			g_lightvec;						// light vector in model reference frame
vec3_t			g_blightvec[MAXSTUDIOBONES];	// light vectors in bone reference frames
int				g_ambientlight;					// ambient world light
float			g_shadelight;					// direct world light
vec3_t			g_lightcolor;

int				g_smodels_total;				// cookie

float			g_bonetransform[MAXSTUDIOBONES][3][4];	// bone transformation matrix

int				g_chrome[MAXSTUDIOVERTS][2];	// texture coords for surface normals
int				g_chromeage[MAXSTUDIOBONES];	// last time chrome vectors were updated
vec3_t			g_chromeup[MAXSTUDIOBONES];		// chrome vector "up" in bone reference frames
vec3_t			g_chromeright[MAXSTUDIOBONES];	// chrome vector "right" in bone reference frames
ViewerSettings 	g_viewerSettings;


////////////////////////////////////////////////////////////////////////

Boolean			bFilterTextures = true;
int 			g_texnum = 3;
vec3_t g_vright = { 50, 50, 0 };		// needs to be set to viewer's right in order for chrome to work
float g_lambert = 1.5;

void drawBox (vec3_t *v);
void InitViewerSettings (void);



CHLMdl::CHLMdl(CResourceManager *resources) : C3DModel(resources)
{
	_mdl = nil;
	_data = nil;
	
	m_sequence = 0;
	m_frame = 0;
	m_bodynum = 0;
	m_skinnum = 0;
	m_mouth = 0;
	m_pmodel = 0;
	m_ptexturehdr = 0;
	
	m_adj[0] = 0.0;
	m_adj[1] = 0.0;
	m_adj[2] = 0.0;
	m_adj[3] = 0.0;
}


CHLMdl::~CHLMdl()
{
	dprintf("~CHLMdl()\n");

}

Boolean CHLMdl::init(CPakStream *inItem)
{	
	InitViewerSettings ();

	// read in all the data
//	_glName = nextGLName();
	_dataSize = inItem->getSize();
	_data = (char*)CMemoryTracker::safeAlloc(1, _dataSize, "half life model data");
	if (!_data) 
		goto fail;
	if(!inItem->getBytes(_dataSize, (char*)_data)) 
		goto fail;
	byte				*pin;
	studiohdr_t			*phdr;
	mstudiotexture_t	*ptexture;
	const char *modelname	= inItem->pathName().c_str();

	pin = (byte *)_data;
	phdr = (studiohdr_t *)pin;
	m_pstudiohdr = phdr;
	m_ptexturehdr = phdr; 
	
	// swap the header
	phdr->version = swapLong(phdr->version);
	phdr->length = swapLong(phdr->numbones);
	for (int i = 0; i < 3; i++) {
		phdr->eyeposition[i] = swapFloat(phdr->eyeposition[i]);
		phdr->min[i] = swapFloat(phdr->min[i]);
		phdr->max[i] = swapFloat(phdr->max[i]);
		phdr->bbmin[i] = swapFloat(phdr->bbmin[i]);
		phdr->bbmax[i] = swapFloat(phdr->bbmax[i]);
	}
	phdr->flags = swapLong(phdr->flags);
	phdr->numbones = swapLong(phdr->numbones);
	phdr->boneindex = swapLong(phdr->boneindex);
	phdr->numbonecontrollers = swapLong(phdr->numbonecontrollers);
	phdr->bonecontrollerindex = swapLong(phdr->bonecontrollerindex);
	phdr->numhitboxes = swapLong(phdr->numhitboxes);
	phdr->hitboxindex = swapLong(phdr->hitboxindex);
	phdr->numseq = swapLong(phdr->numseq);
	phdr->seqindex = swapLong(phdr->seqindex);
	phdr->numseqgroups = swapLong(phdr->numseqgroups);
	phdr->seqgroupindex = swapLong(phdr->seqgroupindex);
	phdr->numtextures = swapLong(phdr->numtextures);
	phdr->textureindex = swapLong(phdr->textureindex);
	phdr->texturedataindex = swapLong(phdr->texturedataindex);
	phdr->numskinref = swapLong(phdr->numskinref);
	phdr->numskinfamilies = swapLong(phdr->numskinfamilies);
	phdr->skinindex = swapLong(phdr->skinindex);
	phdr->numbodyparts = swapLong(phdr->numbodyparts);
	phdr->bodypartindex = swapLong(phdr->bodypartindex);
	phdr->numattachments = swapLong(phdr->numattachments);
	phdr->attachmentindex = swapLong(phdr->attachmentindex);
	phdr->soundtable = swapLong(phdr->soundtable);
	phdr->soundindex = swapLong(phdr->soundindex);
	phdr->soundgroups = swapLong(phdr->soundgroups);
	phdr->soundgroupindex = swapLong(phdr->soundgroupindex);
	phdr->numtransitions = swapLong(phdr->numtransitions);
	phdr->transitionindex = swapLong(phdr->transitionindex);

	// check the version
	ptexture = (mstudiotexture_t *)(pin + phdr->textureindex);

	if (strncmp ((const char *) _data, "IDST", 4) &&
		strncmp ((const char *) _data, "IDSQ", 4))
	{
		goto fail;
	}

	if (!strncmp ((const char *) _data, "IDSQ", 4) && !m_pstudiohdr)
	{
		goto fail;
	}

	// swap the animation sequences
	for (int i = 0; i < m_pstudiohdr->numseq; i++)  {
		mstudioseqdesc_t *pseqdesc = (mstudioseqdesc_t *)((byte *)m_pstudiohdr + m_pstudiohdr->seqindex) + i;
		
		pseqdesc->fps = swapFloat(pseqdesc->fps);
		pseqdesc->flags = swapLong(pseqdesc->flags);
		pseqdesc->activity = swapLong(pseqdesc->activity);
		pseqdesc->actweight = swapLong(pseqdesc->actweight);
		pseqdesc->numevents = swapLong(pseqdesc->numevents);
		pseqdesc->eventindex = swapLong(pseqdesc->fps);
		pseqdesc->numframes = swapLong(pseqdesc->numframes);
		pseqdesc->numpivots = swapLong(pseqdesc->numpivots);
		pseqdesc->pivotindex = swapLong(pseqdesc->pivotindex);
		pseqdesc->motiontype = swapLong(pseqdesc->motiontype);
		pseqdesc->motionbone = swapLong(pseqdesc->motionbone);
		pseqdesc->automoveposindex = swapLong(pseqdesc->automoveposindex);
		pseqdesc->automoveangleindex = swapLong(pseqdesc->automoveangleindex);
		pseqdesc->numblends = swapLong(pseqdesc->numblends);
		pseqdesc->animindex = swapLong(pseqdesc->animindex);
		pseqdesc->blendparent = swapLong(pseqdesc->blendparent);
		pseqdesc->seqgroup = swapLong(pseqdesc->seqgroup);
		pseqdesc->entrynode = swapLong(pseqdesc->entrynode);
		pseqdesc->nodeflags = swapLong(pseqdesc->nodeflags);
		pseqdesc->nextseq = swapLong(pseqdesc->nextseq);
		
		for (int n = 0; n < 3; n++) {
			pseqdesc->linearmovement[n] = swapFloat(pseqdesc->linearmovement[n]);
			pseqdesc->bbmin[n] = swapFloat(pseqdesc->bbmin[n]);
			pseqdesc->bbmax[n] = swapFloat(pseqdesc->bbmax[n]);
		}
		
		for (int n = 0; n < 2; n++) {
			pseqdesc->blendtype[n] = swapLong(pseqdesc->blendtype[n]);
			pseqdesc->blendstart[n] = swapFloat(pseqdesc->blendstart[n]);
			pseqdesc->blendend[n] = swapFloat(pseqdesc->blendend[n]);
		}
	}
	
	// swap the bones
	for (int i = 0; i < m_pstudiohdr->numbones; i++)  {
		mstudiobone_t *pbone = (mstudiobone_t *)((byte *)m_pstudiohdr + m_pstudiohdr->boneindex) + i;
		
		pbone->parent = swapLong(pbone->parent);
		pbone->flags = swapLong(pbone->flags);
		
		for (int n = 0; n < 6; n++) {
			pbone->bonecontroller[n] = swapLong(pbone->bonecontroller[n]);
			pbone->value[n] = swapFloat(pbone->value[n]);
			pbone->scale[n] = swapFloat(pbone->scale[n]);
			
		}
	}
	
	// swap the hit boxes
	for (int i = 0; i < m_pstudiohdr->numhitboxes; i++) {
		mstudiobbox_t *pbboxes = (mstudiobbox_t *) ((byte *) m_pstudiohdr + m_pstudiohdr->hitboxindex) + i;

		pbboxes->bone = swapLong(pbboxes->bone);
		pbboxes->group = swapLong(pbboxes->group);
		
		for (int j = 0; j < 3; j++) {
			pbboxes->bbmin[j] = swapFloat(pbboxes->bbmin[j]);
			pbboxes->bbmax[j] = swapFloat(pbboxes->bbmax[j]);
		}
	}
	
	// swap the attachments
	for (int i = 0; i < m_pstudiohdr->numattachments; i++) {
		mstudioattachment_t *pattachments = (mstudioattachment_t *) ((byte *) m_pstudiohdr + m_pstudiohdr->attachmentindex) + i;

		pattachments->type = swapLong(pattachments->type);
		pattachments->bone = swapLong(pattachments->bone);
		
		for (int j = 0; j < 3; j++) {
			pattachments->org[j] = swapFloat(pattachments->org[j]);
			
			for (int n = 0; n < 3; n++) {
				pattachments->vectors[n][j] = swapFloat(pattachments->vectors[n][j]);
			}
		}
	}
	
	// swap the body parts
	for (int i = 0; i < m_pstudiohdr->numbodyparts; i++) {
		mstudiobodyparts_t   *pbodypart = (mstudiobodyparts_t *)((byte *)m_pstudiohdr + m_pstudiohdr->bodypartindex) + i;

		pbodypart->nummodels = swapLong(pbodypart->nummodels);
		pbodypart->base = swapLong(pbodypart->base);
		pbodypart->modelindex = swapLong(pbodypart->modelindex);
		
		// swap the models
		for (int n = 0; n < pbodypart->nummodels; n++) {
			mstudiomodel_t * model = (mstudiomodel_t *)((byte *)m_pstudiohdr + pbodypart->modelindex) + n;
			
			model->type = swapLong(model->type);
			model->boundingradius = swapFloat(model->boundingradius);
			model->nummesh = swapLong(model->nummesh);
			model->meshindex = swapLong(model->meshindex);
			model->numverts = swapLong(model->numverts);
			model->vertinfoindex = swapLong(model->vertinfoindex);
			model->vertindex = swapLong(model->vertindex);
			model->numnorms = swapLong(model->numnorms);
			model->norminfoindex = swapLong(model->norminfoindex);
			model->normindex = swapLong(model->normindex);
			model->numgroups = swapLong(model->numgroups);
			model->groupindex = swapLong(model->groupindex);
			
			// swap the meshes
			for (int j = 0; j < model->nummesh; j++) {
				mstudiomesh_t *pmesh = (mstudiomesh_t *)((byte *)m_pstudiohdr + model->meshindex) + j;
				
				pmesh->numtris = swapLong(pmesh->numtris);
				pmesh->triindex = swapLong(pmesh->triindex);
				pmesh->skinref = swapLong(pmesh->skinref);
				pmesh->numnorms = swapLong(pmesh->numnorms);
				pmesh->normindex = swapLong(pmesh->normindex);
			}
			
			// swap the vertices
			vec3_t *pstudioverts = (vec3_t *)((byte *)m_pstudiohdr + model->vertindex);
			vec3_t *pstudionorms = (vec3_t *)((byte *)m_pstudiohdr + model->normindex);
			for (int j = 0; j < model->numverts; j++) {
				(*pstudioverts)[0] = swapFloat((*pstudioverts)[0]);
				(*pstudioverts)[1] = swapFloat((*pstudioverts)[1]);
				(*pstudioverts)[2] = swapFloat((*pstudioverts)[2]);
				(*pstudionorms)[0] = swapFloat((*pstudionorms)[0]);
				(*pstudionorms)[1] = swapFloat((*pstudionorms)[1]);
				(*pstudionorms)[2] = swapFloat((*pstudionorms)[2]);
				pstudioverts++;
				pstudionorms++;
			}
		}
	}
	
	// load the textures
	if (phdr->textureindex > 0 && phdr->numtextures <= MAXSTUDIOSKINS)
	{
		for (int i = 0; i < phdr->numtextures; i++)
		{
			ptexture[i].flags = swapLong(ptexture[i].flags);
			ptexture[i].width = swapLong(ptexture[i].width);
			ptexture[i].height = swapLong(ptexture[i].height);
			ptexture[i].index = swapLong(ptexture[i].index);
		
			UploadTexture( &ptexture[i], pin + ptexture[i].index, pin + ptexture[i].width * ptexture[i].height + ptexture[i].index, g_texnum++ );
		}
	}


	// swap more texture stuff 



	// UNDONE: free texture memory


/*	
	
	// preload textures
	if (m_pstudiohdr->numtextures == 0)
	{
		char texturename[256];

		strcpy( texturename, modelname );
		strcpy( &texturename[strlen(texturename) - 4], "T.mdl" );

		m_ptexturehdr = LoadModel( texturename );
		if (!m_ptexturehdr)
		{
			goto fail;
		}
		m_owntexmodel = true;
	}
	else
	{
		m_ptexturehdr = m_pstudiohdr;
		m_owntexmodel = false;
	}

	// preload animations
	if (m_pstudiohdr->numseqgroups > 1)
	{
		for (int i = 1; i < m_pstudiohdr->numseqgroups; i++)
		{
			char seqgroupname[256];

			strcpy( seqgroupname, modelname );
			sprintf( &seqgroupname[strlen(seqgroupname) - 4], "%02d.mdl", i );

			m_panimhdr[i] = LoadModel( seqgroupname );
			if (!m_panimhdr[i])
			{
				goto fail;
			}
		}
	}

	SetSequence (0);
	SetController (0, 0.0f);
	SetController (1, 0.0f);
	SetController (2, 0.0f);
	SetController (3, 0.0f);
	SetMouth (0.0f);

	int n;
	for (n = 0; n < m_pstudiohdr->numbodyparts; n++)
		SetBodygroup (n, 0);

	SetSkin (0);
*/

/*
	vec3_t mins, maxs;
	ExtractBbox (mins, maxs);
	if (mins[2] < 5.0f)
		m_origin[2] = -mins[2];
*/
	return true;
	
fail:

	return false;

}


string CHLMdl::frameName(SInt16 framenum)
{
#pragma unused (framenum)
		return string("outofrange");
}

SInt16 CHLMdl::frameCount() 
{
	return 1; 
}

UInt32 CHLMdl::meshNum()
{
	return 1;
}


void CHLMdl::Draw(CModelInstance *instance, renderingAttributes_t *renderingAttributes)
{
#pragma unused (instance)
	int i;

	if (!m_pstudiohdr)
		return;

	g_smodels_total++; // render data cache cookie

	g_pxformverts = &g_xformverts[0];
	g_pvlightvalues = &g_lightvalues[0];

	if (m_pstudiohdr->numbodyparts == 0)
		return;

	SetUpBones ( );

	SetupLighting( );

	for (i=0 ; i < m_pstudiohdr->numbodyparts ; i++) 
	{
		SetupModel( i );
		if (g_viewerSettings.transparency > 0.0f)
			DrawPoints( );
	}

	// draw bones
	if (renderingAttributes->_showBoneFrameBox) // 
	{
		mstudiobone_t *pbones = (mstudiobone_t *) ((byte *) m_pstudiohdr + m_pstudiohdr->boneindex);
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_DEPTH_TEST);

		for (i = 0; i < m_pstudiohdr->numbones; i++)
		{
			if (pbones[i].parent >= 0)
			{
				glPointSize (3.0f);
				glColor3f (1, 0.7f, 0);
				glBegin (GL_LINES);
				glVertex3f (g_bonetransform[pbones[i].parent][0][3], g_bonetransform[pbones[i].parent][1][3], g_bonetransform[pbones[i].parent][2][3]);
				glVertex3f (g_bonetransform[i][0][3], g_bonetransform[i][1][3], g_bonetransform[i][2][3]);
				glEnd ();

				glColor3f (0, 0, 0.8f);
				glBegin (GL_POINTS);
				if (pbones[pbones[i].parent].parent != -1)
					glVertex3f (g_bonetransform[pbones[i].parent][0][3], g_bonetransform[pbones[i].parent][1][3], g_bonetransform[pbones[i].parent][2][3]);
				glVertex3f (g_bonetransform[i][0][3], g_bonetransform[i][1][3], g_bonetransform[i][2][3]);
				glEnd ();
			}
			else
			{
				// draw parent bone node
				glPointSize (5.0f);
				glColor3f (0.8f, 0, 0);
				glBegin (GL_POINTS);
				glVertex3f (g_bonetransform[i][0][3], g_bonetransform[i][1][3], g_bonetransform[i][2][3]);
				glEnd ();
			}
		}

		glPointSize (1.0f);
	}

	if (renderingAttributes->_showBoneFrameBox) // g_viewerSettings.showAttachments && !g_viewerSettings.use3dfx
	{
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_CULL_FACE);
		glDisable (GL_DEPTH_TEST);
		for (i = 0; i < m_pstudiohdr->numattachments; i++)
		{
			mstudioattachment_t *pattachments = (mstudioattachment_t *) ((byte *) m_pstudiohdr + m_pstudiohdr->attachmentindex);
			vec3_t v[4];
			VectorTransform (pattachments[i].org, g_bonetransform[pattachments[i].bone], v[0]);
			VectorTransform (pattachments[i].vectors[0], g_bonetransform[pattachments[i].bone], v[1]);
			VectorTransform (pattachments[i].vectors[1], g_bonetransform[pattachments[i].bone], v[2]);
			VectorTransform (pattachments[i].vectors[2], g_bonetransform[pattachments[i].bone], v[3]);
			glBegin (GL_LINES);
			glColor3f (1, 0, 0);
			glVertex3fv (v[0]);
			glColor3f (1, 1, 1);
			glVertex3fv (v[1]);
			glColor3f (1, 0, 0);
			glVertex3fv (v[0]);
			glColor3f (1, 1, 1);
			glVertex3fv (v[2]);
			glColor3f (1, 0, 0);
			glVertex3fv (v[0]);
			glColor3f (1, 1, 1);
			glVertex3fv (v[3]);
			glEnd ();

			glPointSize (5);
			glColor3f (0, 1, 0);
			glBegin (GL_POINTS);
			glVertex3fv (v[0]);
			glEnd ();
			glPointSize (1);
		}
	}

	if (renderingAttributes->_showBoneFrameBox) 
	{
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_CULL_FACE);
		if (g_viewerSettings.transparency < 1.0f && !g_viewerSettings.use3dfx)
			glDisable (GL_DEPTH_TEST);
		else
			glEnable (GL_DEPTH_TEST);

		if (g_viewerSettings.use3dfx)
			glColor4f (1, 0, 0, 0.2f);
		else
			glColor4f (1, 0, 0, 0.5f);

		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (i = 0; i < m_pstudiohdr->numhitboxes; i++)
		{
			mstudiobbox_t *pbboxes = (mstudiobbox_t *) ((byte *) m_pstudiohdr + m_pstudiohdr->hitboxindex);
			vec3_t v[8], v2[8], bbmin, bbmax;

			VectorCopy (pbboxes[i].bbmin, bbmin);
			VectorCopy (pbboxes[i].bbmax, bbmax);

			v[0][0] = bbmin[0];
			v[0][1] = bbmax[1];
			v[0][2] = bbmin[2];

			v[1][0] = bbmin[0];
			v[1][1] = bbmin[1];
			v[1][2] = bbmin[2];

			v[2][0] = bbmax[0];
			v[2][1] = bbmax[1];
			v[2][2] = bbmin[2];

			v[3][0] = bbmax[0];
			v[3][1] = bbmin[1];
			v[3][2] = bbmin[2];

			v[4][0] = bbmax[0];
			v[4][1] = bbmax[1];
			v[4][2] = bbmax[2];

			v[5][0] = bbmax[0];
			v[5][1] = bbmin[1];
			v[5][2] = bbmax[2];

			v[6][0] = bbmin[0];
			v[6][1] = bbmax[1];
			v[6][2] = bbmax[2];

			v[7][0] = bbmin[0];
			v[7][1] = bbmin[1];
			v[7][2] = bbmax[2];

			VectorTransform (v[0], g_bonetransform[pbboxes[i].bone], v2[0]);
			VectorTransform (v[1], g_bonetransform[pbboxes[i].bone], v2[1]);
			VectorTransform (v[2], g_bonetransform[pbboxes[i].bone], v2[2]);
			VectorTransform (v[3], g_bonetransform[pbboxes[i].bone], v2[3]);
			VectorTransform (v[4], g_bonetransform[pbboxes[i].bone], v2[4]);
			VectorTransform (v[5], g_bonetransform[pbboxes[i].bone], v2[5]);
			VectorTransform (v[6], g_bonetransform[pbboxes[i].bone], v2[6]);
			VectorTransform (v[7], g_bonetransform[pbboxes[i].bone], v2[7]);
			
			drawBox (v2);
		}
	}
}

#pragma mark -


Boolean CHLMdl::canExportFormat(ExportFormatT format)
{
	switch (format) {
		case WAVEFRONT_OBJ_FORMAT:
		case AUTOCAD_DXF_FORMAT:
			return true;
	}
	return false;
}

void CHLMdl::dxfExportMesh(CModelInstance *instance, LFileStream *file, Mat4 transform, int meshnum)
{
#pragma unused (instance, file, transform, meshnum)
}

void CHLMdl::objExportMeshVertexData(CModelInstance *instance, LFileStream *file, Mat4 transform, int meshnum) 
{
#pragma unused (instance, file, transform, meshnum)
}	
		
void CHLMdl::objExportMeshElementData(LFileStream *file, int meshnum, int &vertexCount, int &meshCount)
{
#pragma unused (file, meshnum, vertexCount, meshCount)
}


#pragma mark -


void CHLMdl::CalcBoneAdj( )
{
	int					i, j;
	float				value;
	mstudiobonecontroller_t *pbonecontroller;
	
	pbonecontroller = (mstudiobonecontroller_t *)((byte *)m_pstudiohdr + m_pstudiohdr->bonecontrollerindex);

	for (j = 0; j < m_pstudiohdr->numbonecontrollers; j++)
	{
		i = pbonecontroller[j].index;
		if (i <= 3)
		{
			// check for 360% wrapping
			if (pbonecontroller[j].type & STUDIO_RLOOP)
			{
				value = m_controller[i] * (360.0/256.0) + pbonecontroller[j].start;
			}
			else 
			{
				value = m_controller[i] / 255.0;
				if (value < 0) value = 0;
				if (value > 1.0) value = 1.0;
				value = (1.0 - value) * pbonecontroller[j].start + value * pbonecontroller[j].end;
			}
			// Con_DPrintf( "%d %d %f : %f\n", m_controller[j], m_prevcontroller[j], value, dadt );
		}
		else
		{
			value = m_mouth / 64.0;
			if (value > 1.0) value = 1.0;
			value = (1.0 - value) * pbonecontroller[j].start + value * pbonecontroller[j].end;
			// Con_DPrintf("%d %f\n", mouthopen, value );
		}
		switch(pbonecontroller[j].type & STUDIO_TYPES)
		{
		case STUDIO_XR:
		case STUDIO_YR:
		case STUDIO_ZR:
			m_adj[j] = value * (Q_PI / 180.0);
			break;
		case STUDIO_X:
		case STUDIO_Y:
		case STUDIO_Z:
			m_adj[j] = value;
			break;
		}
	}
}


void CHLMdl::CalcBoneQuaternion( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *q )
{
	int					j, k;
	vec4_t				q1, q2;
	vec3_t				angle1, angle2;
	mstudioanimvalue_t	*panimvalue;

	for (j = 0; j < 3; j++)
	{
		if (swapShort(panim->offset[j+3]) == 0)
		{
			angle2[j] = angle1[j] = pbone->value[j+3]; // default;
		}
		else
		{
			panimvalue = (mstudioanimvalue_t *)((byte *)panim + swapShort(panim->offset[j+3]));
			k = frame;
			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
			}
			// Bah, missing blend!
			if (panimvalue->num.valid > k)
			{
				angle1[j] = panimvalue[k+1].value;

				if (panimvalue->num.valid > k + 1)
				{
					angle2[j] = panimvalue[k+2].value;
				}
				else
				{
					if (panimvalue->num.total > k + 1)
						angle2[j] = angle1[j];
					else
						angle2[j] = swapShort(panimvalue[panimvalue->num.valid+2].value);
				}
			}
			else
			{
				angle1[j] = swapShort(panimvalue[panimvalue->num.valid].value);
				if (panimvalue->num.total > k + 1)
				{
					angle2[j] = angle1[j];
				}
				else
				{
					angle2[j] = swapShort(panimvalue[panimvalue->num.valid + 2].value);
				}
			}
			angle1[j] = pbone->value[j+3] + angle1[j] * pbone->scale[j+3];
			angle2[j] = pbone->value[j+3] + angle2[j] * pbone->scale[j+3];
		}

		if (pbone->bonecontroller[j+3] != -1)
		{
			angle1[j] += m_adj[pbone->bonecontroller[j+3]];
			angle2[j] += m_adj[pbone->bonecontroller[j+3]];
		}
	}

	if (!VectorCompare( angle1, angle2 ))
	{
		AngleQuaternion( angle1, q1 );
		AngleQuaternion( angle2, q2 );
		QuaternionSlerp( q1, q2, s, q );
	}
	else
	{
		AngleQuaternion( angle1, q );
	}
}


void CHLMdl::CalcBonePosition( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *pos )
{
	int					j, k;
	mstudioanimvalue_t	*panimvalue;

	for (j = 0; j < 3; j++)
	{
		pos[j] = pbone->value[j]; // default;
		if (swapShort(panim->offset[j]) != 0)
		{
			panimvalue = (mstudioanimvalue_t *)((byte *)panim + swapShort(panim->offset[j]));
			
			k = frame;
			// find span of values that includes the frame we want
			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
			}
			// if we're inside the span
			if (panimvalue->num.valid > k)
			{
				// and there's more data in the span
				if (panimvalue->num.valid > k + 1)
				{
					pos[j] += (swapShort(panimvalue[k+1].value) * (1.0 - s) + s * swapShort(panimvalue[k+2].value)) * pbone->scale[j];
				}
				else
				{
					pos[j] += swapShort(panimvalue[k+1].value) * pbone->scale[j];
				}
			}
			else
			{
				// are we at the end of the repeating values section and there's another section with data?
				if (panimvalue->num.total <= k + 1)
				{
					pos[j] += (swapShort(panimvalue[panimvalue->num.valid].value) * (1.0 - s) + s * swapShort(panimvalue[panimvalue->num.valid + 2].value)) * pbone->scale[j];
				}
				else
				{
					pos[j] += swapShort(panimvalue[panimvalue->num.valid].value) * pbone->scale[j];
				}
			}
		}
		if (pbone->bonecontroller[j] != -1)
		{
			pos[j] += m_adj[pbone->bonecontroller[j]];
		}
	}
}


void CHLMdl::CalcRotations ( vec3_t *pos, vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f )
{
	int					i;
	int					frame;
	mstudiobone_t		*pbone;
	float				s;

	frame = (int)f;
	s = (f - frame);

	// add in programatic controllers
	CalcBoneAdj( );

	pbone		= (mstudiobone_t *)((byte *)m_pstudiohdr + m_pstudiohdr->boneindex);
	for (i = 0; i < m_pstudiohdr->numbones; i++, pbone++, panim++) 
	{
		CalcBoneQuaternion( frame, s, pbone, panim, q[i] );
		CalcBonePosition( frame, s, pbone, panim, pos[i] );
	}

	if (pseqdesc->motiontype & STUDIO_X)
		pos[pseqdesc->motionbone][0] = 0.0;
	if (pseqdesc->motiontype & STUDIO_Y)
		pos[pseqdesc->motionbone][1] = 0.0;
	if (pseqdesc->motiontype & STUDIO_Z)
		pos[pseqdesc->motionbone][2] = 0.0;
}


mstudioanim_t * CHLMdl::GetAnim( mstudioseqdesc_t *pseqdesc )
{
	mstudioseqgroup_t	*pseqgroup;
	pseqgroup = (mstudioseqgroup_t *)((byte *)m_pstudiohdr + m_pstudiohdr->seqgroupindex) + pseqdesc->seqgroup;

	if (pseqdesc->seqgroup == 0)
	{
		return (mstudioanim_t *)((byte *)m_pstudiohdr + pseqgroup->data + pseqdesc->animindex);
	}

	return (mstudioanim_t *)((byte *)m_panimhdr[pseqdesc->seqgroup] + pseqdesc->animindex);
}


void CHLMdl::SlerpBones( vec4_t q1[], vec3_t pos1[], vec4_t q2[], vec3_t pos2[], float s )
{
	int			i;
	vec4_t		q3;
	float		s1;

	if (s < 0) s = 0;
	else if (s > 1.0) s = 1.0;

	s1 = 1.0 - s;

	for (i = 0; i < m_pstudiohdr->numbones; i++)
	{
		QuaternionSlerp( q1[i], q2[i], s, q3 );
		q1[i][0] = q3[0];
		q1[i][1] = q3[1];
		q1[i][2] = q3[2];
		q1[i][3] = q3[3];
		pos1[i][0] = pos1[i][0] * s1 + pos2[i][0] * s;
		pos1[i][1] = pos1[i][1] * s1 + pos2[i][1] * s;
		pos1[i][2] = pos1[i][2] * s1 + pos2[i][2] * s;
	}
}


void CHLMdl::AdvanceFrame( float dt )
{
	if (!m_pstudiohdr)
		return;

	mstudioseqdesc_t	*pseqdesc;
	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pstudiohdr + m_pstudiohdr->seqindex) + m_sequence;

	if (dt > 0.1)
		dt = 0.1f;
	m_frame += dt * pseqdesc->fps;

	if (pseqdesc->numframes <= 1)
	{
		m_frame = 0;
	}
	else
	{
		// wrap
		m_frame -= (int)(m_frame / (pseqdesc->numframes - 1)) * (pseqdesc->numframes - 1);
	}
}


int CHLMdl::SetFrame( int nFrame )
{
	if (nFrame == -1)
		return m_frame;

	if (!m_pstudiohdr)
		return 0;

	mstudioseqdesc_t	*pseqdesc;
	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pstudiohdr + m_pstudiohdr->seqindex) + m_sequence;

	m_frame = nFrame;

	if (pseqdesc->numframes <= 1)
	{
		m_frame = 0;
	}
	else
	{
		// wrap
		m_frame -= (int)(m_frame / (pseqdesc->numframes - 1)) * (pseqdesc->numframes - 1);
	}

	return m_frame;
}


void CHLMdl::SetUpBones ( void )
{
	int					i;

	mstudiobone_t		*pbones;
	mstudioseqdesc_t	*pseqdesc;
	mstudioanim_t		*panim;

	static vec3_t		pos[MAXSTUDIOBONES];
	float				bonematrix[3][4];
	static vec4_t		q[MAXSTUDIOBONES];

	static vec3_t		pos2[MAXSTUDIOBONES];
	static vec4_t		q2[MAXSTUDIOBONES];
	static vec3_t		pos3[MAXSTUDIOBONES];
	static vec4_t		q3[MAXSTUDIOBONES];
	static vec3_t		pos4[MAXSTUDIOBONES];
	static vec4_t		q4[MAXSTUDIOBONES];


	if (m_sequence >=  m_pstudiohdr->numseq) {
		m_sequence = 0;
	}

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pstudiohdr + m_pstudiohdr->seqindex) + m_sequence;

	panim = GetAnim( pseqdesc );
	CalcRotations( pos, q, pseqdesc, panim, m_frame );

	if (pseqdesc->numblends > 1)
	{
		float				s;

		panim += m_pstudiohdr->numbones;
		CalcRotations( pos2, q2, pseqdesc, panim, m_frame );
		s = m_blending[0] / 255.0;

		SlerpBones( q, pos, q2, pos2, s );

		if (pseqdesc->numblends == 4)
		{
			panim += m_pstudiohdr->numbones;
			CalcRotations( pos3, q3, pseqdesc, panim, m_frame );

			panim += m_pstudiohdr->numbones;
			CalcRotations( pos4, q4, pseqdesc, panim, m_frame );

			s = m_blending[0] / 255.0;
			SlerpBones( q3, pos3, q4, pos4, s );

			s = m_blending[1] / 255.0;
			SlerpBones( q, pos, q3, pos3, s );
		}
	}

	pbones = (mstudiobone_t *)((byte *)m_pstudiohdr + m_pstudiohdr->boneindex);

	for (i = 0; i < m_pstudiohdr->numbones; i++) {
		QuaternionMatrix( q[i], bonematrix );

		bonematrix[0][3] = pos[i][0];
		bonematrix[1][3] = pos[i][1];
		bonematrix[2][3] = pos[i][2];

		if (pbones[i].parent == -1) {
			memcpy(g_bonetransform[i], bonematrix, sizeof(float) * 12);
		} 
		else {
			R_ConcatTransforms (g_bonetransform[pbones[i].parent], bonematrix, g_bonetransform[i]);
		}
	}
}



/*
================
CHLMdl::TransformFinalVert
================
*/
void CHLMdl::Lighting (float *lv, int bone, int flags, vec3_t normal)
{
	// FIXME: later
	return;
	
	float 	illum;
	float	lightcos;

	illum = g_ambientlight;

	if (flags & STUDIO_NF_FLATSHADE)
	{
		illum += g_shadelight * 0.8;
	} 
	else 
	{
		float r;
		lightcos = DotProduct (normal, g_blightvec[bone]); // -1 colinear, 1 opposite

		if (lightcos > 1.0)
			lightcos = 1;

		illum += g_shadelight;

		r = g_lambert;
		if (r <= 1.0) r = 1.0;

		lightcos = (lightcos + (r - 1.0)) / r; 		// do modified hemispherical lighting
		if (lightcos > 0.0) 
		{
			illum -= g_shadelight * lightcos; 
		}
		if (illum <= 0)
			illum = 0;
	}

	if (illum > 255) 
		illum = 255;
	*lv = illum / 255.0;	// Light from 0 to 1.0
}


void CHLMdl::Chrome (int *pchrome, int bone, vec3_t normal)
{
#pragma unused (pchrome, bone, normal)
/*
	float n;

	if (g_chromeage[bone] != g_smodels_total)
	{
		// calculate vectors from the viewer to the bone. This roughly adjusts for position
		vec3_t chromeupvec;		// g_chrome t vector in world reference frame
		vec3_t chromerightvec;	// g_chrome s vector in world reference frame
		vec3_t tmp;				// vector pointing at bone in world reference frame
		VectorScale( m_origin, -1, tmp );
		tmp[0] += g_bonetransform[bone][0][3];
		tmp[1] += g_bonetransform[bone][1][3];
		tmp[2] += g_bonetransform[bone][2][3];
		VectorNormalize( tmp );
		CrossProduct( tmp, g_vright, chromeupvec );
		VectorNormalize( chromeupvec );
		CrossProduct( tmp, chromeupvec, chromerightvec );
		VectorNormalize( chromerightvec );

		VectorIRotate( chromeupvec, g_bonetransform[bone], g_chromeup[bone] );
		VectorIRotate( chromerightvec, g_bonetransform[bone], g_chromeright[bone] );

		g_chromeage[bone] = g_smodels_total;
	}

	// calc s coord
	n = DotProduct( normal, g_chromeright[bone] );
	pchrome[0] = (n + 1.0) * 32; // FIX: make this a float

	// calc t coord
	n = DotProduct( normal, g_chromeup[bone] );
	pchrome[1] = (n + 1.0) * 32; // FIX: make this a float
*/
}


/*
================
CHLMdl::SetupLighting
	set some global variables based on entity position
inputs:
outputs:
	g_ambientlight
	g_shadelight
================
*/
void CHLMdl::SetupLighting ( )
{
	int i;
	g_ambientlight = 32;
	g_shadelight = 192;

	g_lightvec[0] = 0;
	g_lightvec[1] = 0;
	g_lightvec[2] = -1.0;

	g_lightcolor[0] = g_viewerSettings.lColor[0];
	g_lightcolor[1] = g_viewerSettings.lColor[1];
	g_lightcolor[2] = g_viewerSettings.lColor[2];

	// TODO: only do it for bones that actually have textures
	for (i = 0; i < m_pstudiohdr->numbones; i++)
	{
		VectorIRotate( g_lightvec, g_bonetransform[i], g_blightvec[i] );
	}
}


/*
=================
CHLMdl::SetupModel
	based on the body part, figure out which mesh it should be using.
inputs:
	currententity
outputs:
	pstudiomesh
	pmdl
=================
*/

void CHLMdl::SetupModel ( int bodypart )
{
	int index;

	if (bodypart > m_pstudiohdr->numbodyparts)
	{
		// Con_DPrintf ("CHLMdl::SetupModel: no such bodypart %d\n", bodypart);
		bodypart = 0;
	}

	mstudiobodyparts_t   *pbodypart = (mstudiobodyparts_t *)((byte *)m_pstudiohdr + m_pstudiohdr->bodypartindex) + bodypart;

	index = m_bodynum / pbodypart->base;
	index = index % pbodypart->nummodels;

	m_pmodel = (mstudiomodel_t *)((byte *)m_pstudiohdr + pbodypart->modelindex) + index;
}




/*
================

================
*/
void CHLMdl::DrawPoints ( )
{
	int					i, j;
	mstudiomesh_t		*pmesh;
	byte				*pvertbone;
	byte				*pnormbone;
	vec3_t				*pstudioverts;
	vec3_t				*pstudionorms;
	mstudiotexture_t	*ptexture;
	float 				*av;
	float				*lv;
	float				lv_tmp;
	short				*pskinref;

	pvertbone = ((byte *)m_pstudiohdr + m_pmodel->vertinfoindex);
	pnormbone = ((byte *)m_pstudiohdr + m_pmodel->norminfoindex);
	ptexture = (mstudiotexture_t *)((byte *)m_ptexturehdr + m_ptexturehdr->textureindex);

	pmesh = (mstudiomesh_t *)((byte *)m_pstudiohdr + m_pmodel->meshindex);

	pstudioverts = (vec3_t *)((byte *)m_pstudiohdr + m_pmodel->vertindex);
	pstudionorms = (vec3_t *)((byte *)m_pstudiohdr + m_pmodel->normindex);

	pskinref = (short *)((byte *)m_ptexturehdr + m_ptexturehdr->skinindex);
	if (m_skinnum != 0 && m_skinnum < m_ptexturehdr->numskinfamilies)
		pskinref += (m_skinnum * m_ptexturehdr->numskinref);

	for (i = 0; i < m_pmodel->numverts; i++)
	{
		//vec3_t tmp;
		//VectorScale (pstudioverts[i], 12, tmp);
		VectorTransform (pstudioverts[i], g_bonetransform[pvertbone[i]], g_pxformverts[i]);
	}

	if (g_viewerSettings.transparency < 1.0f)
	{
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

//
// clip and draw all triangles
//


	lv = (float *)g_pvlightvalues;
	for (j = 0; j < m_pmodel->nummesh; j++) 
	{
		int flags = ptexture[pskinref[pmesh[j].skinref]].flags;
		for (i = 0; i < pmesh[j].numnorms; i++, lv += 3, pstudionorms++, pnormbone++)
		{
			Lighting (&lv_tmp, *pnormbone, flags, (float *)pstudionorms);

			// FIX: move this check out of the inner loop
			if (flags & STUDIO_NF_CHROME)
				Chrome( g_chrome[(float (*)[3])lv - g_pvlightvalues], *pnormbone, (float *)pstudionorms );

			lv[0] = lv_tmp * g_lightcolor[0];
			lv[1] = lv_tmp * g_lightcolor[1];
			lv[2] = lv_tmp * g_lightcolor[2];
		}
	}

	// glCullFace(GL_FRONT);

	for (j = 0; j < m_pmodel->nummesh; j++) 
	{
		float s, t;
		short		*ptricmds;

		pmesh = (mstudiomesh_t *)((byte *)m_pstudiohdr + m_pmodel->meshindex) + j;
		ptricmds = (short *)((byte *)m_pstudiohdr + pmesh->triindex);

		s = 1.0/(float)ptexture[pskinref[pmesh->skinref]].width;
		t = 1.0/(float)ptexture[pskinref[pmesh->skinref]].height;

		//glBindTexture( GL_TEXTURE_2D, ptexture[pskinref[pmesh->skinref]].index );
		glBindTexture( GL_TEXTURE_2D, pskinref[pmesh->skinref] + 3);

		if (ptexture[pskinref[pmesh->skinref]].flags & STUDIO_NF_CHROME)
		{
		/*
			while (i = swapShort(*(ptricmds++)))
			{
				if (i < 0)
				{
					glBegin( GL_TRIANGLE_FAN );
					i = -i;
				}
				else
				{
					glBegin( GL_TRIANGLE_STRIP );
				}


				for( ; i > 0; i--, ptricmds += 4)
				{
					// FIX: put these in as integer coords, not floats
					glTexCoord2f(g_chrome[ptricmds[1]][0]*s, g_chrome[ptricmds[1]][1]*t);
					
					lv = g_pvlightvalues[ptricmds[1]];
					glColor4f( lv[0], lv[1], lv[2], g_viewerSettings.transparency);

					av = g_pxformverts[ptricmds[0]];
					glVertex3f(av[0], av[1], av[2]);
				}
				glEnd( );
			}	
		*/
		} 
		else 
		{
			while (i = swapShort(*(ptricmds++)))
			{
				if (i < 0)
				{
					glBegin( GL_TRIANGLE_FAN );
					i = -i;
				}
				else
				{
					glBegin( GL_TRIANGLE_STRIP );
				}


				for( ; i > 0; i--, ptricmds += 4)
				{
					glTexCoord2f(swapShort(ptricmds[2])*s, swapShort(ptricmds[3])*t);
				
			// FIXME:	
			//		lv = g_pvlightvalues[swapShort(ptricmds[1])];
			//		glColor4f( lv[0], lv[1], lv[2], g_viewerSettings.transparency);

					av = g_pxformverts[swapShort(ptricmds[0])];
					glVertex3f((av[0]), (av[1]), (av[2]));
				}
				glEnd( );
			}	
		}
	}
}

#pragma mark -

void CHLMdl::UploadTexture(mstudiotexture_t *ptexture, byte *data, byte *pal, int name)
{
	// unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight;
	int		i, j;
	int		row1[256], row2[256], col1[256], col2[256];
	byte	*pix1, *pix2, *pix3, *pix4;
	byte	*tex, *out;

	// convert texture to power of 2
	int outwidth;
	for (outwidth = 1; outwidth < ptexture->width; outwidth <<= 1)
		;

	//outwidth >>= 1;
	if (outwidth > 256)
		outwidth = 256;

	int outheight;
	for (outheight = 1; outheight < ptexture->height; outheight <<= 1)
		;

	//outheight >>= 1;
	if (outheight > 256)
		outheight = 256;

	tex = out = (byte *)malloc( outwidth * outheight * 4);
	if (!out)
	{
		return;
	}
/*
	int k = 0;
	for (i = 0; i < ptexture->height; i++)
	{
		for (j = 0; j < ptexture->width; j++)
		{

			in[k++] = pal[data[i * ptexture->width + j] * 3 + 0];
			in[k++] = pal[data[i * ptexture->width + j] * 3 + 1];
			in[k++] = pal[data[i * ptexture->width + j] * 3 + 2];
			in[k++] = 0xff;;
		}
	}

	gluScaleImage (GL_RGBA, ptexture->width, ptexture->height, GL_UNSIGNED_BYTE, in, outwidth, outheight, GL_UNSIGNED_BYTE, out);
	free (in);
*/

	for (i = 0; i < outwidth; i++)
	{
		col1[i] = (int) ((i + 0.25) * (ptexture->width / (float)outwidth));
		col2[i] = (int) ((i + 0.75) * (ptexture->width / (float)outwidth));
	}

	for (i = 0; i < outheight; i++)
	{
		row1[i] = (int) ((i + 0.25) * (ptexture->height / (float)outheight)) * ptexture->width;
		row2[i] = (int) ((i + 0.75) * (ptexture->height / (float)outheight)) * ptexture->width;
	}

	// scale down and convert to 32bit RGB
	for (i=0 ; i<outheight ; i++)
	{
		for (j=0 ; j<outwidth ; j++, out += 4)
		{
			pix1 = &pal[data[row1[i] + col1[j]] * 3];
			pix2 = &pal[data[row1[i] + col2[j]] * 3];
			pix3 = &pal[data[row2[i] + col1[j]] * 3];
			pix4 = &pal[data[row2[i] + col2[j]] * 3];

			out[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0])>>2;
			out[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1])>>2;
			out[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2])>>2;
			out[3] = 0xFF;
		}
	}

	glBindTexture( GL_TEXTURE_2D, name ); //g_texnum );		
	glTexImage2D( GL_TEXTURE_2D, 0, 3/*??*/, outwidth, outheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex );
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bFilterTextures ? GL_LINEAR:GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bFilterTextures ? GL_LINEAR:GL_NEAREST);

	// ptexture->width = outwidth;
	// ptexture->height = outheight;
	//ptexture->index = name; //g_texnum;

	free( tex );
}



void
CHLMdl::FreeModel ()
{
	if (m_pstudiohdr)
		free (m_pstudiohdr);

	if (m_ptexturehdr && m_owntexmodel)
		free (m_ptexturehdr);

	m_pstudiohdr = m_ptexturehdr = 0;
	m_owntexmodel = false;

	int i;
	for (i = 0; i < 32; i++)
	{
		if (m_panimhdr[i])
		{
			free (m_panimhdr[i]);
			m_panimhdr[i] = 0;
		}
	}

	// deleting textures
	g_texnum -= 3;
	int textures[MAXSTUDIOSKINS];
	for (i = 0; i < g_texnum; i++)
		textures[i] = i + 3;

	glDeleteTextures (g_texnum, (const GLuint *) textures);

	g_texnum = 3;
}



bool CHLMdl::SaveModel ( char *modelname )
{
	if (!modelname)
		return false;

	if (!m_pstudiohdr)
		return false;

	FILE *file;
	
	file = fopen (modelname, "wb");
	if (!file)
		return false;

	fwrite (m_pstudiohdr, sizeof (byte), m_pstudiohdr->length, file);
	fclose (file);

	// write texture model
	if (m_owntexmodel && m_ptexturehdr)
	{
		char texturename[256];

		strcpy( texturename, modelname );
		strcpy( &texturename[strlen(texturename) - 4], "T.mdl" );

		file = fopen (texturename, "wb");
		if (file)
		{
			fwrite (m_ptexturehdr, sizeof (byte), m_ptexturehdr->length, file);
			fclose (file);
		}
	}

	// write seq groups
	if (m_pstudiohdr->numseqgroups > 1)
	{
		for (int i = 1; i < m_pstudiohdr->numseqgroups; i++)
		{
			char seqgroupname[256];

			strcpy( seqgroupname, modelname );
			sprintf( &seqgroupname[strlen(seqgroupname) - 4], "%02d.mdl", i );

			file = fopen (seqgroupname, "wb");
			if (file)
			{
				fwrite (m_panimhdr[i], sizeof (byte), m_panimhdr[i]->length, file);
				fclose (file);
			}
		}
	}

	return true;
}



////////////////////////////////////////////////////////////////////////

int CHLMdl::GetSequence( )
{
	return m_sequence;
}

int CHLMdl::SetSequence( int iSequence )
{
	if (iSequence > m_pstudiohdr->numseq)
		return m_sequence;

	m_sequence = iSequence;
	m_frame = 0;

	return m_sequence;
}


void CHLMdl::ExtractBbox( float *mins, float *maxs )
{
	mstudioseqdesc_t	*pseqdesc;

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pstudiohdr + m_pstudiohdr->seqindex);
	
	mins[0] = pseqdesc[ m_sequence ].bbmin[0];
	mins[1] = pseqdesc[ m_sequence ].bbmin[1];
	mins[2] = pseqdesc[ m_sequence ].bbmin[2];

	maxs[0] = pseqdesc[ m_sequence ].bbmax[0];
	maxs[1] = pseqdesc[ m_sequence ].bbmax[1];
	maxs[2] = pseqdesc[ m_sequence ].bbmax[2];
}



void CHLMdl::GetSequenceInfo( float *pflFrameRate, float *pflGroundSpeed )
{
	mstudioseqdesc_t	*pseqdesc;

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pstudiohdr + m_pstudiohdr->seqindex) + (int)m_sequence;

	if (pseqdesc->numframes > 1)
	{
		*pflFrameRate = 256 * pseqdesc->fps / (pseqdesc->numframes - 1);
		*pflGroundSpeed = sqrt( pseqdesc->linearmovement[0]*pseqdesc->linearmovement[0]+ pseqdesc->linearmovement[1]*pseqdesc->linearmovement[1]+ pseqdesc->linearmovement[2]*pseqdesc->linearmovement[2] );
		*pflGroundSpeed = *pflGroundSpeed * pseqdesc->fps / (pseqdesc->numframes - 1);
	}
	else
	{
		*pflFrameRate = 256.0;
		*pflGroundSpeed = 0.0;
	}
}



float CHLMdl::SetController( int iController, float flValue )
{
	if (!m_pstudiohdr)
		return 0.0f;

	mstudiobonecontroller_t	*pbonecontroller = (mstudiobonecontroller_t *)((byte *)m_pstudiohdr + m_pstudiohdr->bonecontrollerindex);

	// find first controller that matches the index
	int i;
	for (i = 0; i < m_pstudiohdr->numbonecontrollers; i++, pbonecontroller++)
	{
		if (pbonecontroller->index == iController)
			break;
	}
	if (i >= m_pstudiohdr->numbonecontrollers)
		return flValue;

	// wrap 0..360 if it's a rotational controller
	if (pbonecontroller->type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (pbonecontroller->end < pbonecontroller->start)
			flValue = -flValue;

		// does the controller not wrap?
		if (pbonecontroller->start + 359.0 >= pbonecontroller->end)
		{
			if (flValue > ((pbonecontroller->start + pbonecontroller->end) / 2.0) + 180)
				flValue = flValue - 360;
			if (flValue < ((pbonecontroller->start + pbonecontroller->end) / 2.0) - 180)
				flValue = flValue + 360;
		}
		else
		{
			if (flValue > 360)
				flValue = flValue - (int)(flValue / 360.0) * 360.0;
			else if (flValue < 0)
				flValue = flValue + (int)((flValue / -360.0) + 1) * 360.0;
		}
	}

	int setting = (int) (255 * (flValue - pbonecontroller->start) /
	(pbonecontroller->end - pbonecontroller->start));

	if (setting < 0) setting = 0;
	if (setting > 255) setting = 255;
	m_controller[iController] = setting;

	return setting * (1.0 / 255.0) * (pbonecontroller->end - pbonecontroller->start) + pbonecontroller->start;
}


float CHLMdl::SetMouth( float flValue )
{
	if (!m_pstudiohdr)
		return 0.0f;

	mstudiobonecontroller_t	*pbonecontroller = (mstudiobonecontroller_t *)((byte *)m_pstudiohdr + m_pstudiohdr->bonecontrollerindex);

	// find first controller that matches the mouth
	for (int i = 0; i < m_pstudiohdr->numbonecontrollers; i++, pbonecontroller++)
	{
		if (pbonecontroller->index == 4)
			break;
	}

	// wrap 0..360 if it's a rotational controller
	if (pbonecontroller->type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (pbonecontroller->end < pbonecontroller->start)
			flValue = -flValue;

		// does the controller not wrap?
		if (pbonecontroller->start + 359.0 >= pbonecontroller->end)
		{
			if (flValue > ((pbonecontroller->start + pbonecontroller->end) / 2.0) + 180)
				flValue = flValue - 360;
			if (flValue < ((pbonecontroller->start + pbonecontroller->end) / 2.0) - 180)
				flValue = flValue + 360;
		}
		else
		{
			if (flValue > 360)
				flValue = flValue - (int)(flValue / 360.0) * 360.0;
			else if (flValue < 0)
				flValue = flValue + (int)((flValue / -360.0) + 1) * 360.0;
		}
	}

	int setting = (int) (64 * (flValue - pbonecontroller->start) / (pbonecontroller->end - pbonecontroller->start));

	if (setting < 0) setting = 0;
	if (setting > 64) setting = 64;
	m_mouth = setting;

	return setting * (1.0 / 64.0) * (pbonecontroller->end - pbonecontroller->start) + pbonecontroller->start;
}


float CHLMdl::SetBlending( int iBlender, float flValue )
{
	mstudioseqdesc_t	*pseqdesc;

	if (!m_pstudiohdr)
		return 0.0f;

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pstudiohdr + m_pstudiohdr->seqindex) + (int)m_sequence;

	if (pseqdesc->blendtype[iBlender] == 0)
		return flValue;

	if (pseqdesc->blendtype[iBlender] & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (pseqdesc->blendend[iBlender] < pseqdesc->blendstart[iBlender])
			flValue = -flValue;

		// does the controller not wrap?
		if (pseqdesc->blendstart[iBlender] + 359.0 >= pseqdesc->blendend[iBlender])
		{
			if (flValue > ((pseqdesc->blendstart[iBlender] + pseqdesc->blendend[iBlender]) / 2.0) + 180)
				flValue = flValue - 360;
			if (flValue < ((pseqdesc->blendstart[iBlender] + pseqdesc->blendend[iBlender]) / 2.0) - 180)
				flValue = flValue + 360;
		}
	}

	int setting = (int) (255 * (flValue - pseqdesc->blendstart[iBlender]) / (pseqdesc->blendend[iBlender] - pseqdesc->blendstart[iBlender]));

	if (setting < 0) setting = 0;
	if (setting > 255) setting = 255;

	m_blending[iBlender] = setting;

	return setting * (1.0 / 255.0) * (pseqdesc->blendend[iBlender] - pseqdesc->blendstart[iBlender]) + pseqdesc->blendstart[iBlender];
}



int CHLMdl::SetBodygroup( int iGroup, int iValue )
{
	if (!m_pstudiohdr)
		return 0;

	if (iGroup > m_pstudiohdr->numbodyparts)
		return -1;

	mstudiobodyparts_t *pbodypart = (mstudiobodyparts_t *)((byte *)m_pstudiohdr + m_pstudiohdr->bodypartindex) + iGroup;

	int iCurrent = (m_bodynum / pbodypart->base) % pbodypart->nummodels;

	if (iValue >= pbodypart->nummodels)
		return iCurrent;

	m_bodynum = (m_bodynum - (iCurrent * pbodypart->base) + (iValue * pbodypart->base));

	return iValue;
}


int CHLMdl::SetSkin( int iValue )
{
	if (!m_pstudiohdr)
		return 0;

	if (iValue >= m_pstudiohdr->numskinfamilies)
	{
		return m_skinnum;
	}

	m_skinnum = iValue;

	return iValue;
}



void CHLMdl::scaleMeshes (float scale)
{
	if (!m_pstudiohdr)
		return;

	int i, j, k;

	// scale verts
	int tmp = m_bodynum;
	for (i = 0; i < m_pstudiohdr->numbodyparts; i++)
	{
		mstudiobodyparts_t *pbodypart = (mstudiobodyparts_t *)((byte *)m_pstudiohdr + m_pstudiohdr->bodypartindex) + i;
		for (j = 0; j < pbodypart->nummodels; j++)
		{
			SetBodygroup (i, j);
			SetupModel (i);

			vec3_t *pstudioverts = (vec3_t *)((byte *)m_pstudiohdr + m_pmodel->vertindex);

			for (k = 0; k < m_pmodel->numverts; k++)
				VectorScale (pstudioverts[k], scale, pstudioverts[k]);
		}
	}

	m_bodynum = tmp;

	// scale complex hitboxes
	mstudiobbox_t *pbboxes = (mstudiobbox_t *) ((byte *) m_pstudiohdr + m_pstudiohdr->hitboxindex);
	for (i = 0; i < m_pstudiohdr->numhitboxes; i++)
	{
		VectorScale (pbboxes[i].bbmin, scale, pbboxes[i].bbmin);
		VectorScale (pbboxes[i].bbmax, scale, pbboxes[i].bbmax);
	}

	// scale bounding boxes
	mstudioseqdesc_t *pseqdesc = (mstudioseqdesc_t *)((byte *)m_pstudiohdr + m_pstudiohdr->seqindex);
	for (i = 0; i < m_pstudiohdr->numseq; i++)
	{
		VectorScale (pseqdesc[i].bbmin, scale, pseqdesc[i].bbmin);
		VectorScale (pseqdesc[i].bbmax, scale, pseqdesc[i].bbmax);
	}

	// maybe scale exeposition, pivots, attachments
}



void CHLMdl::scaleBones (float scale)
{
	if (!m_pstudiohdr)
		return;

	mstudiobone_t *pbones = (mstudiobone_t *) ((byte *) m_pstudiohdr + m_pstudiohdr->boneindex);
	for (int i = 0; i < m_pstudiohdr->numbones; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			pbones[i].value[j] *= scale;
			pbones[i].scale[j] *= scale;
		}
	}	
}

#pragma mark -


void InitViewerSettings (void)
{
	memset (&g_viewerSettings, 0, sizeof (ViewerSettings));
	g_viewerSettings.rot[0] = -90.0f;
	g_viewerSettings.trans[3] = 50.0f;
	g_viewerSettings.renderMode = RM_TEXTURED;
	g_viewerSettings.transparency = 1.0f;

	g_viewerSettings.gColor[0] = 0.85f;
	g_viewerSettings.gColor[1] = 0.85f;
	g_viewerSettings.gColor[2] = 0.69f;

	g_viewerSettings.lColor[0] = 1.0f;
	g_viewerSettings.lColor[1] = 1.0f;
	g_viewerSettings.lColor[2] = 1.0f;

	g_viewerSettings.speedScale = 1.0f;
	g_viewerSettings.textureLimit = 256;

	g_viewerSettings.textureScale = 1.0f;
}

void drawBox (vec3_t *v)
{
	glBegin (GL_QUAD_STRIP);
	for (int i = 0; i < 10; i++)
		glVertex3fv (v[i & 7]);
	glEnd ();
	
	glBegin  (GL_QUAD_STRIP);
	glVertex3fv (v[6]);
	glVertex3fv (v[0]);
	glVertex3fv (v[4]);
	glVertex3fv (v[2]);
	glEnd ();

	glBegin  (GL_QUAD_STRIP);
	glVertex3fv (v[1]);
	glVertex3fv (v[7]);
	glVertex3fv (v[3]);
	glVertex3fv (v[5]);
	glEnd ();

}


