/*
	CBsp.cpp

	Author:	Tom Naughton		
	Description:	based on AfterShock


 * Aftershock 3D rendering engine
 * Copyright (C) 1999 Stephen C. Taylor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 
*/

#include <agl.h>
#include <glut.h>
#include <stdlib.h>

#include "CTextures.h"
#include "CPakStream.h"
#include "CGLImage.h"
#include "CBsp.h"
#include "CShader.h"
#include "CResourceManager.h"
#include "CModelInstance.h"
#include "CMd3Controller.h"
#include "CMd3.h"
#include "utilities.h"
#include "cstdlib"

static int face_cmp(const void *a, const void *b);

// Viewport (window) size in pixels 
#define VIEWPORT_W 400
#define VIEWPORT_H 300

#define BSPHEADERID  (*(int*)"IBSP")
#define FAKKHEADERID  (*(int*)"FAKK")
#define BSPVERSION 46
#define FAKKVERSION 12
#define MAX_MISC_MODEL 500 // Room for additional models

#define CLIPPING_LEAF 1
#define CLIPPING_NODE 1
#define CLIPPING_FACE 1
#define DRAW_MAPENTS 1

#define AZ_SCALE 0.3
#define EL_SCALE 0.3
#define MOVE_SPEED 400.0

#define READLUMP(lump, val, success) \
      r_num##val = readlump(lump, "lump", (void**)&r_##val, sizeof(*(r_##val)), success)

#define BSP_TESTVIS(from,to) \
        (*(r_visibility->data + (from)*r_visibility->rowsize + \
           ((to)>>3)) & (1 << ((to) & 7)))


// mapent
   
mapent_class_t mapent_classinit[] =
{
	#include "mapent.h"
};
  
CBsp::CBsp(CResourceManager *inResources)
{
    _resources = inResources;

	g_mapent_numinst = 0;
	g_mapent_nummisc = 0;
	g_mapent_numclasses = 0;
	g_mapent_class = 0;
	g_mapent_inst = 0;

	r_subdivisiontol = 5;
	r_maxmeshlevel = 5;
	r_drawitems=1;
	r_stereo=FALSE;
	r_fullscreen=FALSE;

	move = 0;
	strafe = 0;
    fpstime = 0;
    fpsframes = 0;

	bsplen = 0;
	bspdata = 0;
	r_models  = 0;
	r_verts = 0;
	r_planes = 0;
	r_leafs = 0;
	r_nodes = 0;
	r_faces = 0;
	r_lfaces = 0;
	r_elems = 0;
	r_visibility = 0;
	r_meshes = 0;
	r_skybox = 0;
	facelist.faces  = 0;
	translist.faces = 0;
	r_faceinc = 0;
	skylist = 0;
	g_mapent_class = 0;
	g_mapent_inst = 0;
	entities = 0;
	epairs = 0;
	entity_buf = 0;
	
}


CBsp::~CBsp()
{
    mesh_free_all();
    skybox_free();
    mapent_freeall();
    bsp_free();
    render_finalize();
	entity_free();
}

Boolean CBsp::init(CPakStream *bsp, renderingAttributes_t *renderingAttributes)
{
    float fov_y;
    _rendering = renderingAttributes;
    
    // My GL driver (linux/TNT2) has problems with glEnableClientState(),
    //   but this seems to clear it up.  Go figure. 
    glFinish();
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    g_frametime = 0;
	if (!bsp_read(bsp)) goto fail;
    if (!mesh_create_all()) goto fail;
    if (!skybox_create()) goto fail;
    if (!mapent_loadall(bsp->rootArchive())) goto fail;
    _resources->parseShaders();
     // Make default shaders for those that weren't found 

   if (!render_init()) goto fail;
    
    find_start_pos(_rendering);
    vec_point(_rendering->r_eyedir, _rendering->r_eye_az, _rendering->r_eye_el);

    glEnable(GL_DITHER);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glColor3f(1.0, 1.0, 1.0);

    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glViewport(0, 0, VIEWPORT_W, VIEWPORT_H);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    fov_y = calc_fov(_rendering->r_eyefov, VIEWPORT_W, VIEWPORT_H);
    gluPerspective(fov_y, (float)VIEWPORT_W/(float)VIEWPORT_H,
		   NEAR_GL_PLANE, FAR_GL_PLANE);
    glMatrixMode(GL_MODELVIEW);

    g_starttime = (double)glutGet(GLUT_ELAPSED_TIME) / 1000.0;

    return true;
    
fail:
	return false;
}

void CBsp::mouse_down(renderingAttributes_t *rendering, int x, int y)
{
#pragma unused (rendering)
    ox = x; oy = y;
}


void CBsp::mouse_motion(renderingAttributes_t *rendering, int x, int y)
{

	float el = -(y - oy) * EL_SCALE;
	float az = -(x - ox) * AZ_SCALE;
	
    rendering->r_eye_az = rendering->r_eye_az + az;
    rendering->r_eye_el = rendering->r_eye_el + el;
    
    ox = x; 
    oy = y; 

    vec_point(rendering->r_eyedir, rendering->r_eye_az, rendering->r_eye_el);
}


void CBsp::Draw(renderingAttributes_t *renderingAttributes)
{	
    if (CShader::_animateShaders) g_frametime = (double)glutGet(GLUT_ELAPSED_TIME) / 1000.0 - g_starttime;
    _rendering = renderingAttributes;
	if (_rendering->_wireframe) 
	    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	else 
	    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	    
	
	// FIXME: color buffer clear can be optionally removed 
	#define CLEAR_BITS GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT

	r_leafcount = 0;

	// Find eye cluster for PVS checks 
	if (!_rendering->r_lockpvs)
		_rendering->r_eyecluster = find_cluster(_rendering->r_eyepos);
		
		
//	if (_rendering->r_eyecluster < 0)
//		return;

	// Need to enable depth mask before clear 
	glDepthMask(GL_TRUE);
	glDrawBuffer(GL_BACK_LEFT);
	glClear(CLEAR_BITS);
	if (r_stereo) {
		glDrawBuffer(GL_BACK_RIGHT);
		glClear(CLEAR_BITS);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up camera 

	if(r_stereo)
	{
		vec3_t up, right;
		up[0]=0; up[1]=0; up[2]=1.0;

		// calculate right vector for eye seperation 
		vec_cross(_rendering->r_eyedir, up, right);
		vec_normalize(right);
		vec_scale(right, _rendering->r_eyesep, right);

		// draw right image 
		glDrawBuffer(GL_BACK_RIGHT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		// Set up camera 
		gluLookAt(_rendering->r_eyepos[0]+right[0],_rendering->r_eyepos[1]+right[1],
		_rendering->r_eyepos[2]+right[2],_rendering->r_eyepos[0]+(_rendering->r_eyedir[0]*_rendering->r_focallength),
		_rendering->r_eyepos[1]+(_rendering->r_eyedir[1]*_rendering->r_focallength),
		_rendering->r_eyepos[2]+(_rendering->r_eyedir[2]*_rendering->r_focallength),
		0.0, 0.0, 1.0);
		render_objects();

		// draw left image 
		glDrawBuffer(GL_BACK_LEFT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(_rendering->r_eyepos[0]-right[0],_rendering->r_eyepos[1]-right[1],
		_rendering->r_eyepos[2]-right[2],_rendering->r_eyepos[0]+(_rendering->r_eyedir[0]*_rendering->r_focallength),
		_rendering->r_eyepos[1]+(_rendering->r_eyedir[1]*_rendering->r_focallength),
		_rendering->r_eyepos[2]+(_rendering->r_eyedir[2]*_rendering->r_focallength),
		0.0, 0.0, 1.0);

		render_objects();
	} else {
		gluLookAt(_rendering->r_eyepos[0], _rendering->r_eyepos[1], _rendering->r_eyepos[2],
		_rendering->r_eyepos[0]+_rendering->r_eyedir[0], _rendering->r_eyepos[1]+_rendering->r_eyedir[1],
		_rendering->r_eyepos[2]+_rendering->r_eyedir[2], 0.0, 0.0, 1.0);

		render_objects();
	}

#if 0
    // Enable for speeds reporting (like r_speeds 1) 
    printf("faces: %d, leafs: %d\n", facelist.numfaces + translist.numfaces,
	   r_leafcount);
#endif    

}




void CBsp::move_eye(renderingAttributes_t *rendering, double intervaltime, int move, int strafe)
{
    vec3_t delta;

    if (move)
    {
	vec_copy(rendering->r_eyedir, delta);
	vec_scale(delta, move * MOVE_SPEED * intervaltime, delta);
	vec_add(rendering->r_eyepos, delta, rendering->r_eyepos);
    }
    
    if (strafe)
    {
	float norm = sqrt(rendering->r_eyedir[0]*rendering->r_eyedir[0] + rendering->r_eyedir[1]*rendering->r_eyedir[1]);
	delta[0] = rendering->r_eyedir[1] / norm;
	delta[1] = -rendering->r_eyedir[0] / norm;
	delta[2] = 0.0f;
	vec_scale(delta, strafe * MOVE_SPEED * intervaltime, delta);
	vec_add(rendering->r_eyepos, delta, rendering->r_eyepos);	
    }
}



void CBsp::reshape(renderingAttributes_t *rendering, int width, int height)
{
    float fov_y;
    _width = width;
    _height = height;

    fov_y = calc_fov(rendering->r_eyefov, _width, _height);
    gluPerspective(fov_y, (float)_width/(float)_height,
		   NEAR_GL_PLANE, FAR_GL_PLANE);
}



Boolean CBsp::bsp_read(CPakStream *inItem)
{
	UInt8 *lightmapdata = nil;
	bsplen = inItem->getSize();
	bspdata = (Byte *) inItem->getData("bspdata");
	if (!bspdata) goto fail;
    dprintf("bsp size %dK\n", bsplen/1024);

    bspheader = (struct header*)bspdata;
    bspheader->ver = swapLong(bspheader->ver);
    
    
    // Format seems to be different for FAKK
    
    if (bspheader->id != BSPHEADERID /* && bspheader->id != FAKKHEADERID */) {
		dprintf("Not a bsp file\n");
		goto fail;
	}
    if (bspheader->ver != BSPVERSION /* && bspheader->ver != FAKKVERSION */) {
		dprintf("Bad bsp file version\n");
		goto fail;
	}

    for (int i=0; i<NUM_LUMPS; i++) {
      bspheader->lump[i].fileofs = swapLong(bspheader->lump[i].fileofs);
      bspheader->lump[i].filelen = swapLong(bspheader->lump[i].filelen);
    }
    
    r_numshaders = bspheader->lump[SHADERREFS].filelen / sizeof(shaderref_t);
	
	Boolean success;			    
    READLUMP(PLANES, planes, success);
    if (!success) goto fail;
    READLUMP(NODES, nodes, success);
    if (!success) goto fail;
    READLUMP(LEAFS, leafs, success);
    if (!success) goto fail;
    READLUMP(LFACES, lfaces, success);
    if (!success) goto fail;
    READLUMP(MODELS, models, success);
    if (!success) goto fail;
    READLUMP(VERTS, verts, success);
    if (!success) goto fail;
    READLUMP(ELEMS, elems, success);
    if (!success) goto fail;
    READLUMP(FACES, faces, success);
    if (!success) goto fail;
    (void)readlump(VISIBILITY, "VISIBILITY",  (void**)&r_visibility, 1, success);
    if (!success) goto fail;

    // Now byte-swap the lumps if we're on a big-endian machine... 
    swaplump(PLANES, r_planes);
    swaplump(NODES, r_nodes);
    swaplump(LEAFS, r_leafs);
    swaplump(LFACES, r_lfaces);
    swaplump(MODELS, r_models);
    swaplump(VERTS, r_verts);
    swaplump(ELEMS, r_elems);
    swaplump(FACES, r_faces);
    r_visibility->numclusters = swapLong(r_visibility->numclusters);
    r_visibility->rowsize = swapLong(r_visibility->rowsize);

    entity_parse(bspheader->lump[ENTITIES].filelen, (char*)(bspdata + bspheader->lump[ENTITIES].fileofs));
    if (!_resources->initShaders(r_numshaders, 200, (shaderref_t*) (bspdata + bspheader->lump[SHADERREFS].fileofs)) == noErr)
    	goto fail;
    
   r_lightmapsize = readlump(LIGHTMAPS, "LIGHTMAPS", (void**)&lightmapdata, 1, success);
   if (!success) goto fail;
   if (!_resources->initLightMaps(lightmapdata, r_lightmapsize) == noErr) goto fail;
   
    CMemoryTracker::safeFree(lightmapdata);
    CMemoryTracker::safeFree(bspdata);
    
    bsp_list();
    return true;
    
fail:
	if (lightmapdata)
    	CMemoryTracker::safeFree(lightmapdata);
	if (bspdata)
    	CMemoryTracker::safeFree(bspdata);
    return false;
}

void CBsp::bsp_list(void)
{
    dprintf("Contents of BSP file:\n\n");
    dprintf("num entities     %d\n", g_numentities);
    dprintf("num models       %d\n", r_nummodels);
    dprintf("num shaders      %d\n", r_numshaders);
    dprintf("num planes       %d\n", r_numplanes);
    dprintf("num verts        %d\n", r_numverts);
    dprintf("num vertex elems %d\n", r_numelems);
    dprintf("num leafs        %d\n", r_numleafs);
    dprintf("num nodes        %d\n", r_numnodes);
    dprintf("num faces        %d\n", r_numfaces);    
    dprintf("num lfaces       %d\n", r_numlfaces);
    dprintf("vis. clusters    %d\n", r_visibility->numclusters);
    
}

void CBsp::bsp_free(void)
{
    CMemoryTracker::safeFree(r_models);
    CMemoryTracker::safeFree(r_verts);
    CMemoryTracker::safeFree(r_planes);
    CMemoryTracker::safeFree(r_leafs);
    CMemoryTracker::safeFree(r_nodes);
    CMemoryTracker::safeFree(r_faces);
    CMemoryTracker::safeFree(r_lfaces);
    CMemoryTracker::safeFree(r_elems);
    CMemoryTracker::safeFree(r_visibility);
}


int CBsp::readlump(int lump, const char *name, void** mem, size_t elem, Boolean &success)
{
    unsigned long len = bspheader->lump[lump].filelen;
    int num = len / elem;
    
    dprintf("lump size %dK\n", len/1024);
    *mem = CMemoryTracker::safeAlloc((unsigned long)1, (unsigned long)len, (char*)name);
    if (!*mem) goto fail;
    memcpy(*mem, bspdata + bspheader->lump[lump].fileofs, len);
    

	success = true;
    return num;
fail:
	success = false;
	return 0;
}

void CBsp::swaplump(int lump, void *mem)
{
	if (!mem) return;
	
	int i, len=bspheader->lump[lump].filelen;
	for (i=0; i<len>>2; i++) {
		unsigned *ptr = (unsigned *)mem;
		ptr[i] = swapLong(ptr[i]);
	}
} 

#pragma mark -

// mesh
#define LEVEL_WIDTH(lvl) ((1 << (lvl+1)) + 1)

Boolean  CBsp::mesh_create_all(void)
{
    int i;    
    
    // Count meshes 
    for (r_nummeshes=0; r_nummeshes < r_numfaces; r_nummeshes++)
		if (r_faces[r_nummeshes].facetype != FACETYPE_MESH)
		    break;

    r_meshes = (mesh_t*)CMemoryTracker::safeAlloc(r_nummeshes, sizeof(mesh_t), "r_meshes");
	if (!r_meshes) goto fail;
    for (i=0; i < r_nummeshes; i++) {
		r_meshes[i].points = 0;
		r_meshes[i].elems = 0;
    }
    for (i=0; i < r_nummeshes; i++) {
		if (!mesh_create(&r_faces[i], &r_meshes[i])) goto fail;
    }
    
    return true;
fail:
	return false;
}

void  CBsp::mesh_free_all(void)
{
    int i;

	if (r_meshes) {
	    for (i=0; i < r_nummeshes; i++)  {
			CMemoryTracker::safeFree(r_meshes[i].points);
			// tex_st and lm_st are part of points: don't free 
			CMemoryTracker::safeFree(r_meshes[i].elems);
	    }
	    CMemoryTracker::safeFree(r_meshes);
    }
}

int  CBsp::mesh_find_level(vec3_t *v)
{
    int level;
    vec3_t a, b, dist;

    // Subdivide on the left until tolerance is reached 
    for (level=0; level < r_maxmeshlevel-1; level++)
    {
	// Subdivide on the left 
	vec_avg(v[0], v[1], a);
	vec_avg(v[1], v[2], b);
	vec_avg(a, b, v[2]);

	// Find distance moved 
	vec_sub(v[2], v[1], dist);

	// Check for tolerance 
	if (vec_dot(dist, dist) < r_subdivisiontol * r_subdivisiontol)
	    break;

	// Insert new middle vertex 
	vec_copy(a, v[1]);
    }

    return level;
}

void CBsp::mesh_find_size(int *numcp, vec3_t *cp, int *size)
{
    int u, v, found, level;
    float *a, *b;
    vec3_t test[3];
    
    // Find non-coincident pairs in u direction 
    found = 0;
    for (v=0; v < numcp[1]; v++)
    {
	for (u=0; u < numcp[0]-1; u += 2)
	{
	    a = cp[v * numcp[0] + u];
	    b = cp[v * numcp[0] + u + 2];
	    if (!vec_cmp(a,b))
	    {
		found = 1;
		break;
	    }
	}
	if (found) break;
    }
    if (!found) dprintf("Bad mesh control points\n");

    // Find subdivision level in u 
    vec_copy(a, test[0]);
    vec_copy((a+3), test[1]);
    vec_copy(b, test[2]);
    level = mesh_find_level(test);
    size[0] = (LEVEL_WIDTH(level) - 1) * ((numcp[0]-1) / 2) + 1;
    
    // Find non-coincident pairs in v direction 
    found = 0;
    for (u=0; u < numcp[0]; u++)
    {
	for (v=0; v < numcp[1]-1; v += 2)
	{
	    a = cp[v * numcp[0] + u];
	    b = cp[(v + 2) * numcp[0] + u];
	    if (!vec_cmp(a,b))
	    {
		found = 1;
		break;
	    }
	}
	if (found) break;
    }
    if (!found) dprintf("Bad mesh control points\n");

    // Find subdivision level in v 
    vec_copy(a, test[0]);
    vec_copy((a+numcp[0]*3), test[1]);
    vec_copy(b, test[2]);
    level = mesh_find_level(test);
    size[1] = (LEVEL_WIDTH(level) - 1)* ((numcp[1]-1) / 2) + 1;    
}

void CBsp::mesh_fill_curve_3(int numcp, int size, int stride, vec3_t *p)
{
    int step, halfstep, i, mid;
    vec3_t a, b;

    step = (size-1) / (numcp-1);

    while (step > 0)
    {
	halfstep = step / 2;
	for (i=0; i < size-1; i += step*2)
	{
	    mid = (i+step)*stride;
	    vec_avg(p[i*stride], p[mid], a);
	    vec_avg(p[mid], p[(i+step*2)*stride], b);
	    vec_avg(a, b, p[mid]);

	    if (halfstep > 0)
	    {
		vec_copy(a, p[(i+halfstep)*stride]);
		vec_copy(b, p[(i+3*halfstep)*stride]);
	    }
	}
	
	step /= 2;
    }
}

void CBsp::mesh_fill_curve_2(int numcp, int size, int stride, vec2_t *p)
{
    int step, halfstep, i, mid;
    vec2_t a, b;

    step = (size-1) / (numcp-1);

    while (step > 0)
    {
	halfstep = step / 2;
	for (i=0; i < size-1; i += step*2)
	{
	    mid = (i+step)*stride;
	    vec2_avg(p[i*stride], p[mid], a);
	    vec2_avg(p[mid], p[(i+step*2)*stride], b);
	    vec2_avg(a, b, p[mid]);

	    if (halfstep > 0)
	    {
		vec2_copy(a, p[(i+halfstep)*stride]);
		vec2_copy(b, p[(i+3*halfstep)*stride]);
	    }
	}
	
	step /= 2;
    }
}

void CBsp::mesh_fill_curve_c(int numcp, int size, int stride, colour_t *p)
{
    int step, halfstep, i, mid;
    colour_t a, b;

    step = (size-1) / (numcp-1);

    while (step > 0)
    {
	halfstep = step / 2;
	for (i=0; i < size-1; i += step*2)
	{
	    mid = (i+step)*stride;
	    colour_avg(p[i*stride], p[mid], a);
	    colour_avg(p[mid], p[(i+step*2)*stride], b);
	    colour_avg(a, b, p[mid]);

	    if (halfstep > 0)
	    {
		colour_copy(a, p[(i+halfstep)*stride]);
		colour_copy(b, p[(i+3*halfstep)*stride]);
	    }
	}
	
	step /= 2;
    }
}

void CBsp::mesh_fill_patch_3(int *numcp, int *size, vec3_t *p)
{
    int step, u, v;

    // Fill in control points in v direction 
    step = (size[0]-1) / (numcp[0]-1);    
    for (u = 0; u < size[0]; u += step)
    {
	mesh_fill_curve_3(numcp[1], size[1], size[0], p + u);
    }

    // Fill in the rest in the u direction 
    for (v = 0; v < size[1]; v++)
    {
	mesh_fill_curve_3(numcp[0], size[0], 1, p + v * size[0]);
    }
}

void CBsp::mesh_fill_patch_2(int *numcp, int *size, vec2_t *p)
{
    int step, u, v;

    // Fill in control points in v direction 
    step = (size[0]-1) / (numcp[0]-1);    
    for (u = 0; u < size[0]; u += step)
    {
	mesh_fill_curve_2(numcp[1], size[1], size[0], p + u);
    }

    // Fill in the rest in the u direction 
    for (v = 0; v < size[1]; v++)
    {
	mesh_fill_curve_2(numcp[0], size[0], 1, p + v * size[0]);
    }
}

void CBsp::mesh_fill_patch_c(int *numcp, int *size, colour_t *p)
{
    int step, u, v;

    // Fill in control points in v direction 
    step = (size[0]-1) / (numcp[0]-1);    
    for (u = 0; u < size[0]; u += step)
    {
	mesh_fill_curve_c(numcp[1], size[1], size[0], p + u);
    }

    // Fill in the rest in the u direction 
    for (v = 0; v < size[1]; v++)
    {
	mesh_fill_curve_c(numcp[0], size[0], 1, p + v * size[0]);
    }
}

Boolean CBsp::mesh_create(face_t *face, mesh_t *mesh)
{
    int step[2], size[2], len, i, u, v, p;
    vec3_t *cp;
    vertex_t *vert;

    cp = (vec3_t*)CMemoryTracker::safeAlloc(face->numverts, sizeof(vec3_t), "mesh vertex indices");
	if (!cp) goto fail;
    vert = &r_verts[face->firstvert];
    for (i=0; i < face->numverts; i++)
    {
	vec_copy(vert->v_point, cp[i]);
	vert++;
    }

    // Find the degree of subdivision in the u and v directions 
    mesh_find_size(face->mesh_cp, cp, size);
    CMemoryTracker::safeFree(cp);

    // Allocate space for mesh 
    len = size[0] * size[1];
    mesh->size[0] = size[0];
    mesh->size[1] = size[1];
    mesh->points = (vec3_t*)CMemoryTracker::safeAlloc(len, (sizeof(vec3_t) + 2 * sizeof(texcoord_t) + sizeof(colour_t)), "mesh vertices");
	if (!mesh->points) goto fail;
    mesh->colour = (colour_t*)(mesh->points + len);
    mesh->tex_st = (texcoord_t*)(mesh->colour + len);
    mesh->lm_st = mesh->tex_st + len;

    // Fill in sparse mesh control points 
    step[0] = (size[0]-1) / (face->mesh_cp[0]-1);
    step[1] = (size[1]-1) / (face->mesh_cp[1]-1);
    vert = &r_verts[face->firstvert];
    for (v = 0; v < size[1]; v += step[1])
    {
	for (u = 0; u < size[0]; u += step[0])
	{
	    p = v * size[0] + u;
	    vec_copy(vert->v_point, mesh->points[p]);
	    colour_copy(vert->colour, mesh->colour[p]);
	    vec2_copy(vert->tex_st, mesh->tex_st[p]);
	    vec2_copy(vert->lm_st, mesh->lm_st[p]);
	    vert++;
	}
    }

    // Fill in each mesh 
    mesh_fill_patch_3(face->mesh_cp, size, mesh->points);
    mesh_fill_patch_c(face->mesh_cp, size, mesh->colour);
    mesh_fill_patch_2(face->mesh_cp, size, (vec2_t*)mesh->tex_st);
    mesh_fill_patch_2(face->mesh_cp, size, (vec2_t*)mesh->lm_st);

    // Allocate and fill element table 
    mesh->numelems = (size[0]-1) * (size[1]-1) * 6;
    mesh->elems = (UInt32*)CMemoryTracker::safeAlloc(mesh->numelems, sizeof(UInt32), "mesh->elems");
	if (!mesh->elems) goto fail;

    i = 0;
    for (v = 0; v < size[1]-1; ++v)
    {
	for (u = 0; u < size[0]-1; ++u)
	{
	    mesh->elems[i++] = v * size[0] + u;
	    mesh->elems[i++] = (v+1) * size[0] + u;
	    mesh->elems[i++] = v * size[0] + u + 1;
	    mesh->elems[i++] = v * size[0] + u + 1;
	    mesh->elems[i++] = (v+1) * size[0] + u;
	    mesh->elems[i++] = (v+1) * size[0] + u + 1;
	}
    }
    return true;
fail:
	return false;
}

#pragma mark -

// skybox 
#define SIDE_SIZE 9
#define POINTS_LEN (SIDE_SIZE*SIDE_SIZE)
#define ELEM_LEN ((SIDE_SIZE-1)*(SIDE_SIZE-1)*6)

#define SPHERE_RAD  10.0
#define EYE_RAD      9.0

#define SCALE_S 4.0  // Arbitrary (?) texture scaling factors 
#define SCALE_T 4.0 

Boolean CBsp::skybox_create(void)
{
    int i;

    // Alloc space for skybox verts, etc. 
    r_skybox = (skybox_t*)CMemoryTracker::safeAlloc(1, sizeof(skybox_t), "r_skybox");
	if (!r_skybox) goto fail;
	r_skybox->points[0] = 0;
	r_skybox->tex_st[0] = 0;
	r_skybox->elems = 0;
	
    r_skybox->points[0] = (vec3_t*)CMemoryTracker::safeAlloc(5 * POINTS_LEN, sizeof(vec3_t), "r_skybox->points");
	if (!r_skybox->points[0]) goto fail;
    r_skybox->tex_st[0] = (texcoord_t*)CMemoryTracker::safeAlloc(5 * POINTS_LEN, sizeof(texcoord_t), "r_skybox->tex_st");
 	if (!r_skybox->tex_st[0]) goto fail;
	r_skybox->elems = (UInt32*)CMemoryTracker::safeAlloc(ELEM_LEN, sizeof(UInt32), "r_skybox->elems");
	if (!r_skybox->elems) goto fail;

    r_skybox->numpoints = POINTS_LEN;
    r_skybox->numelems = ELEM_LEN;
    
    for (i=1; i < 5; i++)
    {
	r_skybox->points[i] = r_skybox->points[i-1] + POINTS_LEN;
	r_skybox->tex_st[i] = r_skybox->tex_st[i-1] + POINTS_LEN;
    }

    gen_box();
    gen_elems();
    return true;
fail:
	return false;
}
    

void CBsp::skybox_free(void)
{
	if (r_skybox) {
	    CMemoryTracker::safeFree(r_skybox->points[0]);
	    CMemoryTracker::safeFree(r_skybox->tex_st[0]);
	    CMemoryTracker::safeFree(r_skybox->elems);
	    CMemoryTracker::safeFree(r_skybox);  
    }  
}

void CBsp::gen_elems(void)
{
    int u, v;
    UInt32 *e;

    // Box elems in tristrip order 
    e = r_skybox->elems;
    for (v = 0; v < SIDE_SIZE-1; ++v)
    {
	for (u = 0; u < SIDE_SIZE-1; ++u)
	{
	    *e++ = v * SIDE_SIZE + u;
	    *e++ = (v+1) * SIDE_SIZE + u;
	    *e++ = v * SIDE_SIZE + u + 1;
	    *e++ = v * SIDE_SIZE + u + 1;
	    *e++ = (v+1) * SIDE_SIZE + u;
	    *e++ = (v+1) * SIDE_SIZE + u + 1;	    
	}
    }
}
    
void CBsp::gen_box(void)
{
    vec3_t orig, drow, dcol;
    float size = 1.0f;
    float step = 0.25f;
    
    // Top 
    orig[0] = -size;
    orig[1] = size;
    orig[2] = size;
    drow[0] = 0.0;
    drow[1] = -step;
    drow[2] = 0.0;
    dcol[0] = step;
    dcol[1] = 0.0;
    dcol[2] = 0.0;
    gen_box_side(SKYBOX_TOP, orig, drow, dcol);

    // Front 
    orig[0] = size;
    orig[1] = size;
    orig[2] = size;
    drow[0] = 0.0;
    drow[1] = 0.0;
    drow[2] = -step;
    dcol[0] = -step;
    dcol[1] = 0.0;
    dcol[2] = 0.0;
    gen_box_side(SKYBOX_FRONT, orig, drow, dcol);

    // Right 
    orig[0] = size;
    orig[1] = -size;
    orig[2] = size;
    drow[0] = 0.0;
    drow[1] = 0.0;
    drow[2] = -step;
    dcol[0] = 0.0;
    dcol[1] = step;
    dcol[2] = 0.0;
    gen_box_side(SKYBOX_RIGHT, orig, drow, dcol);

    // Back 
    orig[0] = -size;
    orig[1] = -size;
    orig[2] = size;
    drow[0] = 0.0;
    drow[1] = 0.0;
    drow[2] = -step;
    dcol[0] = step;
    dcol[1] = 0.0;
    dcol[2] = 0.0;
    gen_box_side(SKYBOX_BACK, orig, drow, dcol);

    // Left 
    orig[0] = -size;
    orig[1] = size;
    orig[2] = size;
    drow[0] = 0.0;
    drow[1] = 0.0;
    drow[2] = -step;
    dcol[0] = 0.0;
    dcol[1] = -step;
    dcol[2] = 0.0;
    gen_box_side(SKYBOX_LEFT, orig, drow, dcol);
}

void CBsp::gen_box_side(int side, vec3_t orig, vec3_t drow, vec3_t dcol)
{
    vec3_t pos, w, row, *v;
    texcoord_t *tc;
    float p;
    int r, c;
    float d, b, t;

    // * I don't know exactly what Q3A does for skybox texturing, but this is
    // * at least fairly close.  We tile the texture onto the inside of
    // * a large sphere, and put the camera near the top of the sphere.
    // * We place the box around the camera, and cast rays through the
    // * box verts to the sphere to find the texture coordinates.
     
    
    d = EYE_RAD;     // Sphere center to camera distance  
    b = SPHERE_RAD;  // Sphere radius 
    
    v = &r_skybox->points[side][0];
    tc = &r_skybox->tex_st[side][0];
    vec_copy(orig, row);
    for (r = 0; r < SIDE_SIZE; ++r)
    {
	vec_copy(row, pos);
	for (c = 0; c < SIDE_SIZE; ++c)
	{
	    // pos points from eye to vertex on box 
	    vec_copy(pos, (*v));
	    vec_copy(pos, w);

	    // Normalize pos -> w 
	    p = sqrt(vec_dot(w, w));
	    w[0] /= p;
	    w[1] /= p;
	    w[2] /= p;

	    // Find distance along w to sphere 
	    t = sqrt(d*d*(w[2]*w[2]-1.0) + b*b) - d*w[2];
	    w[0] *= t;
	    w[1] *= t;

	    // Use x and y on sphere as s and t 
	    (*tc)[0] = w[0] / (2.0 * SCALE_S);
	    (*tc)[1] = w[1] / (2.0 * SCALE_T);
	    
	    vec_add(pos, dcol, pos);
	    v++;
	    tc++;
	}
	vec_add(row, drow, row);
    }
}

#pragma mark -

// ui common

void CBsp::find_start_pos(renderingAttributes_t *rendering)
{
    // Find the first spawn point in the entities list 
    int i;
    const char *cname;
    Boolean found = false;
    
    rendering->r_eye_el = rendering->r_eye_az = 0.0;
    rendering->r_eyepos[0] = rendering->r_eyepos[1] = rendering->r_eyepos[2] = 0.0;
    
    // look for info_player_deathmatch
    for (i = 0; !found && i < g_numentities; i++) {
		cname = entity_value(i, "classname");
		if (!strcmp(cname, "info_player_deathmatch")) {
		    rendering->r_eye_az = entity_float(i, "angle");
		    entity_vec3(i, "origin", rendering->r_eyepos);
		    found = true;
		}
    }

    // look for waypoint_
    for (i = 0; !found && i < g_numentities; i++) {
		cname = entity_value(i, "classname");
		if (!strncmp(cname, "waypoint_", strlen("waypoint_"))) {
		    _rendering->r_eye_az = entity_float(i, "angle");
		    entity_vec3(i, "origin", _rendering->r_eyepos);
		    found = true;
		    break;
		}
    }

    // look for info_player_start
    for (i = 0; !found && i < g_numentities; i++) {
		cname = entity_value(i, "classname");
		if (!strcmp(cname, "info_player_start")) {
		    _rendering->r_eye_az = entity_float(i, "angle");
		    entity_vec3(i, "origin", _rendering->r_eyepos);
		    found = true;
		}
    }


}

void CBsp::find_goodie(renderingAttributes_t *rendering, char *goodie)
{

	// cycle through entites with classnames starting with item_ ammo_ or weapon_    
    const char *cname;
    static int index = -1;
    Boolean found = false;
    
//    _rendering->r_eye_el = _rendering->r_eye_az = 0.0;
//    _rendering->r_eyepos[0] = _rendering->r_eyepos[1] = _rendering->r_eyepos[2] = 0.0;
    
    for (int search = 0; search < g_numentities; search++) {
    	
    	// wrap around array
		index++;
    	if (index >= g_numentities)
    		index = 0;

		if (entity_float(index, "notfree"))
		    continue;
    		
		cname = entity_value(index, "classname");

		if (goodie) {
			if(strstr(cname, goodie) == cname) {
				found = true;
		   		break;		
	   		}
		} else {
			if (strstr(cname, "item_") == cname || strstr(cname, "ammo_") == cname || strstr(cname, "weapon_") == cname) {
				found = true;
		   		break;	
	   		}	
	   	}
    }

	if (found) {    
		dprintf("found goodie! %s\n", cname);
		entity_dump(index);
		
	    rendering->r_eye_az = entity_float(index, "angle");
	    entity_vec3(index, "origin", rendering->r_eyepos);
		move_eye(rendering, .11, -1, 0);
    } else {
    	find_start_pos(rendering);
    }
}


float CBsp::calc_fov(float fov_x, float width, float height)
{
    // Borrowed from the Quake 1 source distribution 
    float   a;
    float   x;
    
    #if 0 // debS
    if (fov_x < 1 || fov_x > 179)
	dprintf("Bad fov: %f\n", fov_x);
    #endif
    
    x = width/tan(fov_x/360.0*M_PI);
    a = atan(height/x);
    a = a*360.0/M_PI;
    
    return a;
}




#pragma mark -

// * The front end of the rendering pipeline decides what to draw, and
// * the back end actually draws it.  These functions build a list of faces
// * to draw, sorts it by shader, and sends it to the back end (renderback.c)
 

#define MAX_TRANSPARENT 2000

enum
{
    CLIP_RIGHT_BIT   = 1,
    CLIP_LEFT_BIT    = 1 << 1,
    CLIP_TOP_BIT     = 1 << 2,
    CLIP_BOTTOM_BIT  = 1 << 3,
    CLIP_FAR_BIT     = 1 << 4,
    CLIP_NEAR_BIT    = 1 << 5
};


Boolean CBsp::render_init(void)
{
    long i;
    
    facelist.faces = (rendface_t*)CMemoryTracker::safeAlloc(r_numfaces, sizeof(rendface_t), "facelist.faces");
    if (!facelist.faces) goto fail;
    translist.faces = (rendface_t*)CMemoryTracker::safeAlloc(MAX_TRANSPARENT, sizeof(rendface_t), "translist.faces");
    if (!translist.faces) goto fail;
    r_faceinc = (long*)CMemoryTracker::safeAlloc(r_numfaces, sizeof(long), "r_faceinc");
    if (!r_faceinc) goto fail;
    skylist = (long*)CMemoryTracker::safeAlloc(100, sizeof(long), "skylist");
    if (!skylist) goto fail;
 //   if (!render_backend_init()) goto fail;

    // Find cluster for each mapent 
    for (i=0; i < g_mapent_numinst; i++)
		g_mapent_inst[i].cluster = find_cluster(g_mapent_inst[i].origin);
    return true;
fail:
	return false;
}

void CBsp::render_finalize(void)
{
    CMemoryTracker::safeFree(facelist.faces);
    CMemoryTracker::safeFree(translist.faces);
    CMemoryTracker::safeFree(r_faceinc);
    CMemoryTracker::safeFree(skylist);
}

void CBsp::render_objects(void)
{
    long i;
    
    r_leafcount = 0;

#if 0
    // Useful for exploring face culling effectiveness 
    if (r_lockpvs)
    {
	render_backend(&facelist);
	return;
    }
#endif    

	_rendering->_renderOpaque = true;
	_rendering->_renderBlend = false;

	cos_fov = cos(_rendering->r_eyefov/2.0f * DEG2RAD);

    // Get clip coordinate transformation matrix 
    gen_clipmat();

    facelist.numfaces = translist.numfaces = 0;
    numsky = 0;
    
    // Clear "included" faces list 
    memset(r_faceinc, 0, r_numfaces * sizeof(long));

    // "Walk" the BSP tree to generate a list of faces to render 
    // FIXME: include other models 
    render_walk_model(0);

    // Sort the face list 
    sort_faces();

    // FIXME: Reset depth buffer range based on max/min Z in facelist 
    // Draw sky first 
    if (numsky && !_rendering->_wireframe)
		render_backend_sky(numsky, (int *)skylist);

    // Draw normal faces 
    render_backend(&facelist);

#if DRAW_MAPENTS
    // Draw visible mapents (md3 models) 
	if (r_drawitems) {
		for(i=0; i < g_mapent_numinst; i++)  {
			if (_rendering->r_eyecluster < 0 || BSP_TESTVIS(_rendering->r_eyecluster, g_mapent_inst[i].cluster))
				render_backend_mapent(i);
		}
	}
#endif

	_rendering->_renderOpaque = false;
	_rendering->_renderBlend = true;

    // Draw transparent faces last 
    if (translist.numfaces)
		render_backend(&translist);

#if DRAW_MAPENTS
    // Draw transparent visible mapents (md3 models) 
	if (r_drawitems) {
		for(i=0; i < g_mapent_numinst; i++)  {
		if (_rendering->r_eyecluster < 0 || BSP_TESTVIS(_rendering->r_eyecluster, g_mapent_inst[i].cluster))
				render_backend_mapent(i);
		}
	}
#endif
	glFlush();

}

void CBsp::render_walk_model(long n)
{
    if (n == 0)
		render_walk_node(0, 0);
    else
		// FIXME: models > 0 are just leafs ? 
		dprintf("Models > 0 not supported\n");
}

long CBsp::classify_point(vec3_t p, long plane_n)
{
    // Determine which side of plane p is on 
    plane_t *plane = &r_planes[plane_n];

    return (vec_dot(p, plane->vector) < plane->offset ? -1 : 1);
}


void CBsp::render_walk_node(long n, long accept)
{
	if ( n<0 ) {
	
	   	leaf_t *leaf = &r_leafs[-(n+1)];
	     
		#if CLIPPING_LEAF
		    // Test visibility before bounding box 
		    if (_rendering->r_eyecluster >= 0)  {
				if (! BSP_TESTVIS(_rendering->r_eyecluster, leaf->cluster)) return;
		    }
		    
		    if (!accept)  {
				if (!cliptest_bbox(leaf->bbox)) return;
		    }    

		#endif
		
		r_leafcount++;
	    for (int i = leaf->firstface; i < leaf->firstface + leaf->numfaces; ++i)  {
			render_walk_face(r_lfaces[i]);
	    }
		
	} else {
	
		if (n < 0 || n >= r_numnodes) {
			dprintf("render_walk_node bad index %d\n", n);
			return;
		}
		
	    node_t *node = &r_nodes[n];
	    
	    
		#if CLIPPING_NODE
		    if (!accept) {
		    
				// Test the node bounding box for intersection with the view volume 
				long clipstate = cliptest_bbox(node->bbox);
				
				// If this node is rejected, reject all sub-nodes 
				if (!clipstate) return;
				
				// If this node is trivially accepted, accept all sub-nodes 
				if (clipstate == 2) accept = 1;
				
		    }
		#endif
	    
	    // Classify eye wrt splitting plane 
	    Boolean inFront = inFront = classify_point(_rendering->r_eyepos, node->plane) > 0;

	    if (inFront) {
	    			
			// IN FRONT :
			render_walk_node (node->children[0], accept);
			render_walk_node (node->children[1], accept);
			    
	    } else {
	    
			// IN BACK :
			render_walk_node (node->children[1], accept);
			render_walk_node (node->children[0], accept);
		}	
	}
}


void CBsp::render_walk_face(long n)
{

	if (n < 0 || n >= r_numfaces) {
		dprintf("render_walk_face bad index %d\n", n);
		return;
	}
		
    face_t *face = &r_faces[n];

    // Check if face is already included in the facelist 
    if (r_faceinc[n]) return;
    r_faceinc[n] = 1;

#if CLIPPING_FACE
    if (face->facetype == FACETYPE_NORMAL) {
		// Face plane culling 
		// FIXME: This simple test is clearly not sufficient.
		//   Q3A gets a lot more culling at this point. 
		if (vec_dot(face->v_norm, _rendering->r_eyedir) > cos_fov)
		    return;
		    
    } else if (face->facetype == FACETYPE_MESH)  {
		// Check bounding box for meshes 
		if (!cliptest_bboxf(face->bbox))
		    return;
    }
#endif

    // Check for sky flag 
    if (_resources->shaderAtIndex(face->shader)->flags & SHADER_SKY)  {
		// Push to sky list 
		skylist[numsky++] = n;
    }

    // Check for transparent 
    else if (_resources->shaderAtIndex(face->shader)->isTransparent())  {
    	if (translist.numfaces < MAX_TRANSPARENT) {
			translist.faces[translist.numfaces].face = n;
			translist.faces[translist.numfaces++].sortkey = SORTKEY(face);
		}
		#if debS
			else DebugStr("\pnot enough transparent faces!");
		#endif
    }

    // Normal face 
    else {
	// Push face to facelist 
		if (facelist.numfaces < r_numfaces) {
			facelist.faces[facelist.numfaces].face = n;
			facelist.faces[facelist.numfaces++].sortkey = SORTKEY(face);
		}
		#if debS
			else DebugStr("\pnot enough faces!");
		#endif
    }
}

void CBsp::gen_clipmat(void)
{
    mat4_t modelview[32];
    mat4_t proj[2];

    // Get the modelview and projection matricies from GL 
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)modelview);
    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat*)proj);

    // Multiply to get clip coordinate transformation 
    mat4_mmult(proj[0], modelview[0], clipmat);
}

long CBsp::cliptest_point(vec4_t p)
{
    long mask = 0;
    const float cx = p[0];
    const float cy = p[1];
    const float cz = p[2];
    const float cw = p[3];      
    
    if (cx >  cw) mask |= CLIP_RIGHT_BIT;
    if (cx < -cw) mask |= CLIP_LEFT_BIT;
    if (cy >  cw) mask |= CLIP_TOP_BIT;
    if (cy < -cw) mask |= CLIP_BOTTOM_BIT;
    if (cz >  cw) mask |= CLIP_FAR_BIT;
    if (cz < -cw) mask |= CLIP_NEAR_BIT;

    return mask;
}

// * Test bounding box for intersection with view fustrum.
// * Return val:   0 = reject
// *               1 = accept
// *               2 = trivially accept (entirely in fustrum)
 
long CBsp::cliptest_bboxf(bboxf_t bv)
{
	// no clipping if we're not in a cluster (this averts a crash on OS X)
	if (_rendering->r_eyecluster < 0)
		return 1;
	
    long corner_index[8][3] =
    {
	{0, 1, 2}, {3, 1, 2}, {3, 4, 2}, {0, 4, 2},
	{0, 1, 5}, {3, 1, 5}, {3, 4, 5}, {0, 4, 5}
    };

    vec4_t corner[8];
    long clipcode, clip_or, clip_and, clip_in;
    long i;

    // Check if eye point is contained 
    if (_rendering->r_eyepos[0] >= bv[0] && _rendering->r_eyepos[0] <= bv[3] &&
	_rendering->r_eyepos[1] >= bv[1] && _rendering->r_eyepos[1] <= bv[4] &&
	_rendering->r_eyepos[2] >= bv[2] && _rendering->r_eyepos[2] <= bv[5])
	return 1;
    
    clip_in = clip_or = 0; clip_and = 0xff;
    for (i=0; i < 8; ++i)
    {
	corner[i][0] = bv[corner_index[i][0]];
	corner[i][1] = bv[corner_index[i][1]];
	corner[i][2] = bv[corner_index[i][2]];
	corner[i][3] = 1.0;

	mat4_vmult(clipmat, corner[i], corner[i]);
	clipcode = cliptest_point(corner[i]);
	clip_or |= clipcode;
	clip_and &= clipcode;
	if (!clipcode) clip_in = 1;
    }

    // Check for trival acceptance/rejection 
    if (clip_and) return 0;
    if (!clip_or) return 2;
    if (clip_in) return 1;   // At least one corner in view fustrum 

#if 0
    // FIXME: need something better for this. 
    // Maybe find maximum radius to each corner 
    {
	// Normalize coordinates 
	vec3_t center, rad;
	float cw;

	cw = 1.0f/corner[0][3];
	vec_scale(corner[0], cw, corner[0]);
	corner[0][3] = 1.0;
	cw = 1.0f/corner[6][3];
	vec_scale(corner[6], cw, corner[6]);
	corner[6][3] = 1.0;

	// Check for non-trivial acceptance 
	vec_avg(corner[0], corner[6], center);
	vec_sub(corner[0], center, rad);
	if (sqrt(vec_dot(center, center)) -
	    sqrt(vec_dot(rad, rad)) <= 1.41421356)
	    return 1;
    }
	
    return 0;
#endif
    return 1;
}

long CBsp::cliptest_bbox(bbox_t bbox)
{
    bboxf_t bv;

    bv[0] = (float)bbox[0];
    bv[1] = (float)bbox[1];
    bv[2] = (float)bbox[2];
    bv[3] = (float)bbox[3];
    bv[4] = (float)bbox[4];
    bv[5] = (float)bbox[5];

    return cliptest_bboxf(bv);
}

long CBsp::find_cluster(vec3_t pos)
{
    node_t *node;
    long cluster = -1;
    long leaf = -1;
    
    node = &r_nodes[0];

    // Find the leaf/cluster containing the given position 
    
    while (1)  {
		if (classify_point(pos, node->plane) > 0) {
		    if (node->children[0] < 0) {
				leaf = -(node->children[0] + 1);
				break;
		    } else {
				node = &r_nodes[node->children[0]];
		    }
		} else {
		    if (node->children[1] < 0) {
				leaf = -(node->children[1] + 1);
				break;
		    } else {
				node = &r_nodes[node->children[1]];
		    }
		}	   
	}

	if (leaf >= 0)
		cluster = r_leafs[leaf].cluster;
    return cluster;
}

static int face_cmp(const void *a, const void *b)
{
    return ((rendface_t*)a)->sortkey - ((rendface_t*)b)->sortkey;
}



void CBsp::sort_faces(void)
{
    // FIXME: expand qsort code here to avoid function calls 
    qsort((void*)facelist.faces, (size_t)facelist.numfaces, (size_t)sizeof(rendface_t), face_cmp);
}

#pragma mark -

// The back-end of the rendering pipeline does the actual drawing.
// * All triangles which share a rendering state (shader) are pushed together
// * into the 'arrays' structure.  This includes verts, texcoords, and element
// * numbers.  The renderer is then 'flushed': the rendering state is set
// * and the triangles are drawn.  The arrays and rendering state is then
// * cleared for the next set of triangles.
 

// FIXME: The manner in which faces are "pushed" to the arrays is really
//   absimal.  I'm sure it could be highly optimized. 
// FIXME: It would be nice to have a consistent view of faces, meshes,
//   mapents, etc. so we don't have to have a "push" function for each. 



void CBsp::render_backend(facelist_t *facelist)
{
    int f, shader = -1, lmtex = -1;
    UInt32 key;
    face_t *face;

    gShaderArrays.numcolours = gShaderArrays.numverts = gShaderArrays.numelems = 0;
    key = (UInt32)-1;
    for (f=0; f < facelist->numfaces; ++f)  {
    
		_rendering->_xPos = _rendering->r_eyepos[0]; 
		_rendering->_zPos = _rendering->r_eyepos[1]; 
		_rendering->_yPos = _rendering->r_eyepos[2]; 
		
		_rendering->_rotAngleX = 0;//_rendering->r_eyedir[0] * 1/DEGTORAD_CONST; 
		_rendering->_rotAngleY = 0;//_rendering->r_eyedir[1] * 1/DEGTORAD_CONST;
		_rendering->_rotAngleZ = 0;//_rendering->r_eyedir[2] * 1/DEGTORAD_CONST;
    	
 		_rendering->_rotCorrectionX = 0; 
		_rendering->_rotCorrectionY = 0;
		_rendering->_rotCorrectionZ = 0;
   	
    	
		face = &r_faces[facelist->faces[f].face];

		// Look for faces that share rendering state 
		if (facelist->faces[f].sortkey != key) {
		    // Flush the renderer and reset 
		    if (f) _resources->shaderAtIndex(shader)->renderFlush ( _rendering, lmtex ); //render_flush(shader, lmtex);
		    shader = face->shader;
		    lmtex = face->lm_texnum;
		    key = facelist->faces[f].sortkey;
		}
		
		// Push the face to the triangle arrays 
		switch (face->facetype) {
		    case FACETYPE_NORMAL:
				render_pushface(face);
				break;
		    case FACETYPE_TRISURF:
				render_pushface(face);
				break;
		    case FACETYPE_FLARE:
				render_pushface(face);
				break;
		    case FACETYPE_MESH:
		    	// some meshes are big, flush vertices from buffer first
				if (facelist->faces[f].face < r_nummeshes) {
			    	_resources->shaderAtIndex(shader)->renderFlush(_rendering, lmtex);
					render_pushmesh(&r_meshes[facelist->faces[f].face]);
				}
				 
				break;
		    default:
				render_pushface(face);
		    	dprintf("unknown facetype %d\n", face->facetype);
				break;
		}
		
    }
	_resources->shaderAtIndex(shader)->renderFlush(_rendering, lmtex);
}

void CBsp::render_backend_sky(int numsky, int *skylist)
{
#pragma unused (numsky)
    int s, i, shader;
    float skyheight;
    UInt32 *elem;
    
    shader = r_faces[skylist[0]].shader;
    skyheight = _resources->shaderAtIndex(shader)->skyheight;
    gShaderArrays.numcolours = gShaderArrays.numverts = gShaderArrays.numelems = 0;

    // Center skybox on camera to give the illusion of a larger space 
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(_rendering->r_eyepos[0], _rendering->r_eyepos[1], _rendering->r_eyepos[2]);
    glScalef(skyheight, skyheight, skyheight);

    // FIXME: Need to cull skybox based on face list 
    for (s=0; s < 5; s++)  {
    
		// make sure it will fit
		if (gShaderArrays.numelems + r_skybox->numelems < MAX_ARRAYS_ELEMS 
			&& gShaderArrays.numverts + r_skybox->numpoints < MAX_ARRAYS_VERTS) {

			elem = r_skybox->elems;
			for (i=0; i < r_skybox->numelems; i++) {
			    gShaderArrays.elems[gShaderArrays.numelems++] = gShaderArrays.numverts + *elem++;
			}
			for (i=0; i < r_skybox->numpoints; i++) {
			    vec_copy(r_skybox->points[s][i], gShaderArrays.verts[gShaderArrays.numverts]);
			    gShaderArrays.verts[gShaderArrays.numverts][3] = 1.0f;
			    vec2_copy(r_skybox->tex_st[s][i], gShaderArrays.tex_st[gShaderArrays.numverts]);
			    gShaderArrays.numverts++;
			}
		}
    }
    
	_resources->shaderAtIndex(shader)->renderFlush(_rendering, 0);
    // Restore world space 
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void CBsp::render_backend_mapent(int mapent)
{
    mapent_inst_t *inst = &g_mapent_inst[mapent];
    mapent_class_t *klass = &g_mapent_class[inst->klass];

    gShaderArrays.numcolours = gShaderArrays.numverts = gShaderArrays.numelems = 0;


	for (int i=0; i < klass->numparts; i++)  {
		CModelInstance *instance = inst->instance[i];

		if (instance) {
		
		    // Calculate bob amount 
		  //  funcargs[0] = funcargs[2] = 0.0f;
		  //  funcargs[1] = klass->parts[i].bobheight;
		  //  funcargs[3] = klass->parts[i].bobspeed;
		  //  float bob = (float)CShader::render_func_eval(SHADER_FUNC_SIN, funcargs);
		    
			double base = 0.0f;
			double amplitude = klass->parts[i].bobheight; 
			double phase  = 0.0f;
			double freq  = klass->parts[i].bobspeed;

			float bob = (amplitude * WaveForm(SHADER_FUNC_SIN, freq * (g_frametime + phase))) + base;

		    // Translate to model origin + bob amount 
		    glMatrixMode(GL_MODELVIEW);
		    glPushMatrix();
		    glTranslatef(inst->origin[0], inst->origin[1], inst->origin[2] + bob + klass->parts[i].height);		
		
			// Scale and rotate part 
			double rot = klass->parts[i].rotspeed * g_frametime + inst->angle;
			glScalef(klass->parts[i].scale, klass->parts[i].scale, klass->parts[i].scale); 
			glRotated(rot, 0.0, 0.0, 1.0);
			
			// setup env mapping
			_rendering->_rotAngleZ = rot;			
			_rendering->_xPos = 0;
			_rendering->_yPos = 0;
			_rendering->_zPos = -bob;
			    
			instance->Draw(_rendering);
			    
			// Restore world state 
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		}
    }
}

void CBsp::render_pushface(face_t *face)
{
    long i, *elem;
    vertex_t *vert;
	
	int *outElems = gShaderArrays.elems + gShaderArrays.numelems;
	float *outVerts = (float*) (gShaderArrays.verts + gShaderArrays.numverts);
	float *outTexvecs = (float*) (gShaderArrays.tex_st + gShaderArrays.numverts);
	float *outLMTexvecs = (float*) (gShaderArrays.lm_st + gShaderArrays.numverts);
	float *outNormals = (float*) (gShaderArrays.norms + gShaderArrays.numverts);
	colour_t *colour = gShaderArrays.colour;
	Boolean needColors = false ; // _resources->shaderAtIndex(face->shader)->flags & SHADER_NEEDCOLOURS;
	
	// make sure it will fit
	if (gShaderArrays.numelems + face->numelems < MAX_ARRAYS_ELEMS 
		&& gShaderArrays.numverts + face->numverts <MAX_ARRAYS_VERTS) {
	
	    elem = &r_elems[face->firstelem];
	    for (i = 0; i < face->numelems; ++i) {
	    	*outElems++ = gShaderArrays.numverts + *elem++; 
	    }
	    gShaderArrays.numelems += face->numelems;
	    
	    vert = &r_verts[face->firstvert];
	    for (i = 0; i < face->numverts; ++i)  {

			*outVerts++ = vert->v_point[0];
			*outVerts++ = vert->v_point[1];
			*outVerts++ = vert->v_point[2];
			*outNormals++ = vert->v_norm[0];
			*outNormals++ = vert->v_norm[1];
			*outNormals++ = vert->v_norm[2];
			*outTexvecs++ = vert->tex_st[0];
			*outTexvecs++ = vert->tex_st[1];
			*outLMTexvecs++ = vert->lm_st[0];
			*outLMTexvecs++ = vert->lm_st[1];
			
			if (needColors) {
				(*colour)[0] = vert->colour[0];
				(*colour)[1] = vert->colour[1];
				(*colour)[2] = vert->colour[2];
				(*colour)[3] = vert->colour[3];
				colour++;
				gShaderArrays.numcolours++;
			}

			gShaderArrays.numverts++;
			vert++;
	    }	
	} else {
		// FIXME: need to choose a better size for the vertex array
	}
}


void CBsp::render_pushmesh(mesh_t *mesh)
{
    long u, v, i, *elem;
    long meshverts = mesh->size[0] * mesh->size[1];

	// something's fishy if there are more elements than verts!
	// FIXME: something wrong with mesh_create?
	if (meshverts > mesh->numelems) {
		//dprintf("found a screwy mesh (meshverts > mesh->numelems)\n");
		return;
	}
	
	int *outElems = gShaderArrays.elems + gShaderArrays.numelems;
	float *outVerts = (float*) (gShaderArrays.verts + gShaderArrays.numverts);
	float *outTexvecs = (float*) (gShaderArrays.tex_st + gShaderArrays.numverts);
	float *outLMTexvecs = (float*) (gShaderArrays.lm_st + gShaderArrays.numverts);
	float *outNormals = (float*) (gShaderArrays.norms + gShaderArrays.numverts);

	// make sure it will fit
	if (gShaderArrays.numelems + mesh->numelems < MAX_ARRAYS_ELEMS 
		&& gShaderArrays.numverts + meshverts < MAX_ARRAYS_VERTS) {

	    elem = (long*)mesh->elems;
	    for (i = 0; i < mesh->numelems; ++i) {
	    	*outElems++ = gShaderArrays.numverts + *elem++; 
	    }
	    gShaderArrays.numelems += mesh->numelems;
	    
	    float *points = (float*)mesh->points;
	    float *tex_st = (float*)mesh->tex_st;
	    float *lm_st = (float*)mesh->lm_st;
	    
	    for (v = 0; v < mesh->size[1]; ++v)  {
			for (u = 0; u < mesh->size[0]; ++u) {
			
				*outVerts++ = *points++;
				*outVerts++ = *points++;
				*outVerts++ = *points++;
				
				// FIXME: meshes need normals for environment mapping
				*outNormals++ = 0.0f;
				*outNormals++ = 0.0f;
				*outNormals++ = 0.0f;

				*outTexvecs++ = *tex_st++;
				*outTexvecs++ = *tex_st++;
				*outLMTexvecs++ = *lm_st++;
				*outLMTexvecs++ = *lm_st++;
			    gShaderArrays.numverts++;
			}
	    }
	} else {
		// FIXME: need to choose a better size for the vertex array
		dprintf("	mesh didn't fit\n");
	}
}


void CBsp::render_stripmine(int numelems, int *elems)
{
    int toggle;
    UInt32 a, b, elem;

    // Vertexes are in tristrip order where possible.  If we can't lock
    // * the vertex arrays (glLockArraysEXT), then it's better to send
    // * tristrips instead of triangles (less transformations).
    // * This function looks for and sends tristrips.
     

    // Tristrip order elems look like this:
    // *  0 1 2 2 1 3 2 3 4 4 3 5 4 5 7 7 5 6  <-- elems
    // *    b a a b b a b a a b b a b a a b b  <-- ab pattern
    // *    \ 1 / \ 2 / \ 3 / \ 4 / \ 5 /      <-- baa/bba groups
     
//     dprintf("numelems %d\n", numelems);
    
    elem = 0;
    while (elem + 2 < numelems) {
		toggle = 1;
		glBegin(GL_TRIANGLE_STRIP);
		
		glArrayElement(elems[elem++]);
		b = elems[elem++];
		glArrayElement(b);
		a = elems[elem++];
		glArrayElement(a);
		
		while (elem + 2 < numelems) {
		    if (a != elems[elem] || b != elems[elem+1])
				break;
		    
		    if (toggle) {
				b = elems[elem+2];
				glArrayElement(b);
		    } else {
				a = elems[elem+2];
				glArrayElement(a);
		    }
		    elem += 3;
		    toggle = !toggle;
		}
		glEnd();
    }
}


#pragma mark -
  

Boolean CBsp::mapent_loadall(CFileArchive *pak)
{
    int i, j;
    const char *cname;

    printf("Initializing Map Models\n");
    
    // Count classes 
    g_mapent_numinst = 0;
    g_mapent_numclasses = 0;
    g_mapent_nummisc = 0;
    while (mapent_classinit[g_mapent_numclasses].name)
	g_mapent_numclasses++;
	    
    // Alloc arrays 
    g_mapent_class = (mapent_class_t*)CMemoryTracker::safeAlloc(g_mapent_numclasses + MAX_MISC_MODEL, sizeof(mapent_class_t), "g_mapent_class");
	if (!g_mapent_class) goto fail;
    g_mapent_inst = (mapent_inst_t*)CMemoryTracker::safeAlloc(g_numentities + MAX_MISC_MODEL, sizeof(mapent_inst_t), "g_mapent_inst");
	if (!g_mapent_inst) goto fail;
    memcpy(g_mapent_class, mapent_classinit, g_mapent_numclasses * sizeof(mapent_class_t));
    
    // Look for mapents in entities list 
    for (i=0; i < g_numentities; i++) {
		// Reject notfree instances 
		if (entity_float(i, "notfree"))
		    continue;
			
		cname = entity_value(i, "classname");
		//dprintf("CBsp::mapent_loadall %s\n",cname);
		
		// for misc_model, create new class, instantiate
		if (!strcmp(cname, "func_rotating")) {
			int klass = mapent_func_rotating(i);
			if (klass > 0) {
				mapent_newinst(pak, klass, i);
			}
		// for model, create new class, instantiate
		} else if (!strncmp(cname, "misc_model", strlen("misc_model"))) {
			int klass = mapent_model(i);
			if (klass > 0) {
				mapent_newinst(pak, klass, i);
			}
		// for shared entities, load, instantiate
		} else {
		
			Boolean found = false;
			for (j=0; !found && j < g_mapent_numclasses; j++) {
			    if (!strcmp(cname, g_mapent_class[j].name))  {

					// Make new mapent instance 
					if(mapent_newinst(pak, j, i))
						found = true;
			    }
			}
		}
    }
    return true;
fail:
	return false;
}

void CBsp::mapent_freeall(void)
{
	for(int i=0; i < g_mapent_numinst; i++)  {
	    mapent_inst_t *inst = &g_mapent_inst[i];
	    mapent_class_t *klass = &g_mapent_class[inst->klass];

	    for (int j=0; j < klass->numparts; j++)  {
			CModelInstance *instance = inst->instance[j];
			if (instance) 
				delete instance;
	    }
	}

    CMemoryTracker::safeFree(g_mapent_class);
    CMemoryTracker::safeFree(g_mapent_inst);
}


Boolean CBsp::mapent_newinst(CFileArchive *pak, int klass, int entity)
{
#pragma unused (pak)
    int inst;
    Boolean result = false;

    inst = g_mapent_numinst;
    g_mapent_numinst++;

    g_mapent_inst[inst].klass = klass;
    entity_vec3(entity, "origin", g_mapent_inst[inst].origin);
	g_mapent_inst[inst].angle = entity_float(entity, "angle");
    g_mapent_inst[inst].cluster = -1;

    for (int i=0; i <g_mapent_class[klass].numparts; i++)  {
	    g_mapent_inst[inst].instance[i] = _resources->modelInstanceWithClassName(g_mapent_class[klass].parts[i].md3name);
		result = result || g_mapent_inst[inst].instance[i] != nil;
	}
	return result;
}

int CBsp::mapent_model(int entity)
{
	if (g_mapent_nummisc < MAX_MISC_MODEL)  {
	
		// create new class 
		int klass = g_mapent_numclasses + g_mapent_nummisc++;
		
		g_mapent_class[klass].name =  "misc_model"; 
		g_mapent_class[klass].numparts = 1;
		g_mapent_class[klass].parts[0].bobspeed = 0;
		g_mapent_class[klass].parts[0].md3name = (char*)entity_value(entity, "model");
		g_mapent_class[klass].parts[0].rotspeed = 0.0;
		g_mapent_class[klass].parts[0].scale = 1.0f;
		g_mapent_class[klass].parts[0].height = 0;
		
		return klass;
	}
	return -1;
}

int CBsp::mapent_func_rotating(int entity)
{
	if (g_mapent_nummisc < MAX_MISC_MODEL)  {
	
		// create new class 
		int klass = g_mapent_numclasses + g_mapent_nummisc++;
		
		g_mapent_class[klass].name =  "misc_model"; 
		g_mapent_class[klass].numparts = 1;
		g_mapent_class[klass].parts[0].bobspeed = 25;
		g_mapent_class[klass].parts[0].md3name = (char*)entity_value(entity, "model2");
		g_mapent_class[klass].parts[0].rotspeed = DEFROT;
		g_mapent_class[klass].parts[0].scale = 1.0f;
		g_mapent_class[klass].parts[0].height = 0;
		
		return klass;
	}
	return -1;
}

  
#pragma mark -
    

#define BYTESWAPVEC3(x) x[0] = swapFloat(x[0]); \
                        x[1] = swapFloat(x[1]); \
                        x[2] = swapFloat(x[2])
#define BYTESWAPBBOX(x) x[0] = swapFloat(x[0]); \
                        x[1] = swapFloat(x[1]); \
                        x[2] = swapFloat(x[2]); \
                        x[3] = swapFloat(x[3]); \
                        x[4] = swapFloat(x[4]); \
                        x[5] = swapFloat(x[5]);


Boolean CBsp::entity_parse(int buflen, char *buf)
{
    int i, newlines, pair;
    char *c;
    
    // Save local copy of buf 
    entity_buf = (char*)CMemoryTracker::safeAlloc(1, buflen+1, "entity_buf");
    if (!entity_buf) goto fail;
    memcpy(entity_buf, buf, buflen);
    *(entity_buf + buflen) = 0;
    
    dprintf("%s\n", entity_buf);

    // Count entities and pairs 
    newlines = g_numentities = 0;
    for (i = 0; i < buflen; i++)
    {
	if (entity_buf[i] == '{') g_numentities++;
	if (entity_buf[i] == '\n') newlines++;
    }
    numepairs = newlines - (2 * g_numentities);

    // Alloc structures 
    epairs = (epair_t*)CMemoryTracker::safeAlloc(numepairs, sizeof(epair_t), "epairs");
    if (!epairs) goto fail;
    entities = (entity_t*)CMemoryTracker::safeAlloc(g_numentities, sizeof(entity_t), "entities");
    if (!entities) goto fail;

    c = entity_buf;
    pair = 0;
    for (i = 0; i < g_numentities; i++) {
		entities[i].firstpair = pair;
		entities[i].numpairs = 0;
		
		// Skip to leading quote 
		while (*c != '"') 
			c++;

		while (*c != '}') {
		    epairs[pair].key = ++c;
		    while (*c != '"') c++;
		    *c = '\0';
		    c += 3;

		    epairs[pair].val = c;
		    while (*c != '"') c++;
		    *c = '\0';
		    c += 2;
		    pair++;
		    entities[i].numpairs++;
		}
    }
fail:
	return 0;
}

void CBsp::entity_free(void)
{
    CMemoryTracker::safeFree(entities);
    CMemoryTracker::safeFree(epairs);
    CMemoryTracker::safeFree(entity_buf);
}

const char * CBsp::entity_value(int entity, const char *key)
{
    epair_t *pair;
    int i;

    pair = &epairs[entities[entity].firstpair];
    for (i = 0; i < entities[entity].numpairs; i++, pair++)
    {
	if (!strcmp(key, pair->key))
	    return pair->val;
    }
    return "";
}

void CBsp::entity_dump(int entity)
{
    epair_t *pair;
    int i;

    pair = &epairs[entities[entity].firstpair];
    for (i = 0; i < entities[entity].numpairs; i++, pair++) {
    	dprintf("%s = %s\n", pair->key, pair->val);
    }
}

float CBsp::entity_float(int entity, const char *key)
{
    return atof(entity_value(entity, key));
}

void CBsp::entity_vec3(int entity, const char *key, vec3_t vector)
{
    const char *val;

    val = entity_value(entity, key);
    sscanf(val, "%f %f %f", &vector[0], &vector[1], &vector[2]);
}


void vec_normalize(vec3_t a)
{
       float b=sqrt((a[0]*a[0])+(a[1]*a[1])+(a[2]*a[2]));

       a[0]/=b; a[1]/=b; a[2]/=b;
}

void
vec_point(vec3_t point, float az, float el)
{
    float c = cos(el * DEG2RAD);
    
    point[0] = c * cos(az * DEG2RAD);
    point[1] = c * sin(az * DEG2RAD);
    point[2] = sin(el * DEG2RAD);
}

/* Stolen from Mesa:matrix.c */
#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]

void
mat4_mmult(mat4_t a, mat4_t b, mat4_t product)
{
   int i;
   for (i = 0; i < 4; i++)
   {
      float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
      P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
      P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
      P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
      P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
   }
}

void
mat4_vmult(mat4_t a, vec4_t b, vec4_t product)
{
    float b0=b[0], b1=b[1], b2=b[2], b3=b[3];

    product[0] = A(0,0)*b0 + A(0,1)*b1 + A(0,2)*b2 + A(0,3)*b3;
    product[1] = A(1,0)*b0 + A(1,1)*b1 + A(1,2)*b2 + A(1,3)*b3;
    product[2] = A(2,0)*b0 + A(2,1)*b1 + A(2,2)*b2 + A(2,3)*b3;
    product[3] = A(3,0)*b0 + A(3,1)*b1 + A(3,2)*b2 + A(3,3)*b3;
}
