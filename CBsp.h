/* 
	CBsp.h

	Author:	Tom Naughton		
	Description:	based on AfterShock
*/

/* Aftershock 3D rendering engine
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


#ifndef CBsp_H
#define CBsp_H

#include "CResourceManager.h"
#include "CShader.h"
#include "vec.h"

class CGLImage;
class CResourceManager;
class CModelInstance;
class CMd3;

#define MAX_APATH			64
#define MAX_NUM_PARTS		5

/* BSP lumps in the order they appear in the header */
enum
{
    ENTITIES,
    SHADERREFS,
    PLANES,
    NODES,
    LEAFS,
    LFACES,
    LBRUSHES,
    MODELS,
    BRUSHES,
    BRUSH_SIDES,
    VERTS,
    ELEMS,
    FOG,
    FACES,
    LIGHTMAPS,
    LIGHTGRID,
    VISIBILITY,
    NUM_LUMPS
};

typedef long bbox_t[6];        /* Integer bounding box (mins, maxs)*/
typedef float bboxf_t[6];     /* Float bounding box */

// bsp

enum
{
    FACETYPE_NORMAL   = 1,
    FACETYPE_MESH     = 2,
    FACETYPE_TRISURF  = 3,
    FACETYPE_FLARE    = 4
};

/* Face planes */
typedef struct
{
    vec3_t vector;    /* Normal to plane */
    float offset;  /* Distance to plane along normal */
} plane_t;

/* Nodes in the BSP tree */
typedef struct
{
    int plane;        /* Dividing plane */
    int children[2];  /* Left and right node.  Negatives are leafs */
    bbox_t bbox;
} node_t;

/* Leafs in BSP tree */
typedef struct
{
    int cluster;    /* Visibility cluster number */
    int area;       /* ? */
    bbox_t bbox;
    int firstface, numfaces;
    int firstunknown, numunknowns;
} leaf_t;

/* Faces (or surfaces) */
typedef struct
{
    int shader;      /* Shader reference */
    int unknown1[1];
    int facetype;   /* FACETYPE enum */
    int firstvert, numverts;
    int firstelem, numelems;
    int lm_texnum;    /* lightmap info */
    int lm_offset[2];
    int lm_size[2];
    vec3_t v_orig;   /* FACETYPE_NORMAL only */
    bboxf_t bbox;    /* FACETYPE_MESH only */
    vec3_t v_norm;   /* FACETYPE_NORMAL only */
    int mesh_cp[2];  /* mesh control point dimensions */
} face_t;



// skybox 

/* The skybox has 5 sides (no bottom) */
enum
{
    SKYBOX_TOP    = 0,
    SKYBOX_FRONT,
    SKYBOX_RIGHT,
    SKYBOX_BACK,
    SKYBOX_LEFT
};

typedef struct
{
    int numpoints;
    vec3_t *points[5];     /* World coords */
    texcoord_t *tex_st[5]; /* Skybox mapped texture coords */
    int numelems;
    UInt32 *elems;
} skybox_t;


// Mesh
typedef struct
{
    int size[2];    /* Mesh dimensions, u&v */
    vec3_t *points;
    colour_t *colour;
    texcoord_t *tex_st;
    texcoord_t *lm_st;
    int numelems;
    UInt32 *elems;
} mesh_t;

// mapent

/* A "mapent" is an entity from the BSP file that is displayed on the map,
 * such as a rocketlauncher.  The mapent class ("rocketlauncher") contains
 * 1 or 2 md3 model "parts".  The mapent instance (a particular rocketlauncher)
 * contains location and visibility info.
 */

#define MAPENT_MAX_PARTS 2

typedef struct
{
    char *md3name;
    float rotspeed;   /* Degrees per sec. (signed) */
    float bobheight;  /* Amplitude of bob */
    float bobspeed;   /* Bobs per second */
    float scale;      /* Extra scale info */
    int height;     
} mapent_part_t;    

typedef struct
{
    char *name;       /* Class name */
    int loaded;       /* Flag: this class has been loaded */
    int numparts;
    mapent_part_t parts[MAPENT_MAX_PARTS];
} mapent_class_t;

typedef struct
{
    int klass;       /* mapent class index */
    int cluster;     /* PVS cluster */
    vec3_t origin;   /* World coords */
    float angle;  
    CModelInstance *instance[MAX_NUM_PARTS];
} mapent_inst_t;


typedef struct
{
    int shader;          /* Shader reference */
    int numverts;
    vec3_t *points;
    texcoord_t *tex_st;  /* Texture coords */
    texcoord_t *env_st;  /* Used for environment mapping ? */
    int numelems;
    UInt32 *elems;
} md3mesh_t;


/* Vertex info */
typedef struct
{
    vec3_t v_point;     /* World coords */
    texcoord_t tex_st;  /* Texture coords */
    texcoord_t lm_st;   /* Lightmap texture coords */
    vec3_t v_norm;      /* Normal */
    colour_t colour;    /* Colour used for vertex lighting ? */
} vertex_t;


/* Model 0 is the main map, others are doors, gates, buttons, etc. */
typedef struct
{
    bboxf_t bbox;
    int firstface, numfaces;
    int firstunknown, numunknowns;
} model_t;

/* Potentially visible set (PVS) data */
typedef struct
{
    int numclusters;   /* Number of PVS clusters */
    int rowsize;
    UInt8 data[1];
} visibility_t;

// renderback

typedef struct
{
    int face;
    UInt32 sortkey;
} rendface_t;

/* List of faces to render */
typedef struct
{
    int numfaces;
    rendface_t *faces;
} facelist_t;


// md3

typedef struct
{
    char id[4];
    int version;
    char filename[68];
    int numboneframes;
    int numtags;
    int nummeshes;
    int numskins;
    int bone_offs;
    int tag_offs;
    int mesh_offs;
    int filesize;
} md3header_t;

typedef struct
{
    char name[64];
    vec3_t pos;
    mat3_t rot;
} md3tag_t;

typedef struct
{
    bboxf_t bbox;
    vec3_t pos;
    float scale;
    char creator[16];
} md3boneframe_t;

typedef struct
{
    char id[4];
    char name[68];
    int numframes;
    int numskins;
    int numverts;
    int numtris;
    int elem_offs;
    int skin_offs;
    int tc_offs;
    int vert_offs;
    int meshsize;
} md3mesh_file_t;

typedef struct
{
    signed short vector[3];
    UInt8 tc[2];
} md3vert_t;

// entity

/* Key-value pair */
typedef struct
{
    char *key;
    char *val;
} epair_t;

/* Group of epairs */
typedef struct
{
    int firstpair;
    int numpairs;
} entity_t;


/* Preferentially sort by shader number, then lightmap */
/* FIXME: other things that could go in the sort key include transparency
 * and 'sort' directives from the shader scripts */
#define SORTKEY(face)  (((face)->shader << 16) + (face)->lm_texnum+1)


class CPakStream;

class CBsp
{

public:
	CBsp(CResourceManager *inResources);
	virtual ~CBsp();

	virtual Boolean 	init(CPakStream *inItem, renderingAttributes_t *renderingAttributes);

	// rendering 	
	virtual void 		Draw(renderingAttributes_t *rendering);
	virtual	void		move_eye(renderingAttributes_t *rendering, double intervaltime, int move, int strafe);
	virtual	void		mouse_down(renderingAttributes_t *rendering, int x, int y);
	virtual	void		mouse_motion(renderingAttributes_t *rendering, int x, int y);
	virtual	void		reshape(renderingAttributes_t *rendering, int width, int height);
	
	// ui common
	void find_start_pos(renderingAttributes_t *rendering);
	void find_goodie(renderingAttributes_t *rendering, char *goodie);
	float calc_fov(float fov_x, float width, float height);


private:
 
 	// loaders
	Boolean bsp_read(CPakStream *inItem);
	void bsp_list(void);
	void bsp_free(void);
	int readlump(int lump, const char *name, void** mem, size_t elem, Boolean &success);
	void swaplump(int lump, void *mem);
	
	// rendering 
	Boolean render_init(void);
	void render_finalize(void);
	void render_objects(void);
	void render_scene();
	void render_walk_model(long n);
	long classify_point(vec3_t p, long plane_n);
	void render_walk_node(long n, long accept);
	void render_walk_leaf(long n, long accept);
	void render_walk_face(long n);
	void gen_clipmat(void);
	long cliptest_point(vec4_t p);
	long cliptest_bbox(bbox_t bbox);
	long cliptest_bboxf(bboxf_t bbox);
	long find_cluster(vec3_t pos);	
	void sort_faces(void);
	
	// mesh
	Boolean mesh_create_all(void);
	void mesh_free_all(void);
	int mesh_find_level(vec3_t *v);
	void mesh_find_size(int *numcp, vec3_t *cp, int *size);
	void mesh_fill_curve_3(int numcp, int size, int stride, vec3_t *p);
	void mesh_fill_curve_2(int numcp, int size, int stride, vec2_t *p);
	void mesh_fill_curve_c(int numcp, int size, int stride, colour_t *p);
	void mesh_fill_patch_3(int *numcp, int *size, vec3_t *p);
	void mesh_fill_patch_2(int *numcp, int *size, vec2_t *p);
	void mesh_fill_patch_c(int *numcp, int *size, colour_t *p);
	Boolean mesh_create(face_t *face, mesh_t *mesh);

	// skybox
	Boolean skybox_create(void);
	void skybox_free(void);
	void gen_elems(void);
	void gen_box(void);
	void gen_box_side(int side, vec3_t orig, vec3_t drow, vec3_t dcol);
	
	

	// renderback
	void render_backend(facelist_t *facelist);
	void render_backend_sky(int numsky, int *skylist);
	void render_backend_mapent(int mapent);
	void render_pushface(face_t *face);
	void render_pushface_deformed(int shadernum, face_t *face);
	void render_pushmesh(mesh_t *mesh);
	void render_stripmine(int numelems, int *elems);
	
	// mapent
	Boolean mapent_loadall(CFileArchive *pak);
	void mapent_freeall(void);
	Boolean mapent_newinst(CFileArchive *pak, int klass, int entity);
	int mapent_func_rotating(int entity);
	int mapent_model(int entity);
	
	// md3
	Boolean md3_init(int max_nummodels);
	void md3_free(void);
	int md3_load(CPakStream *inItem);
	
	// entity
	Boolean entity_parse(int buflen, char *buf);
	void entity_free(void);
	const char * entity_value(int entity, const char *key);
	float entity_float(int entity, const char *key);
	void entity_vec3(int entity, const char *key, vec3_t vector);
	void entity_dump(int entity);
	
	long g_mapent_numclasses;
	long g_mapent_nummisc;
	long g_mapent_numinst;
	mapent_class_t *g_mapent_class;
	mapent_inst_t *g_mapent_inst;
	

	mat4_t clipmat;        /* Matrix to go from worldspace to clipspace */
	facelist_t facelist;   /* Faces to be drawn */
	facelist_t translist;  /* Transparent faces to be drawn */
	long r_leafcount;       /* Counts up leafs walked for this scene */
	long *r_faceinc;        /* Flags faces as "included" in the facelist */
	long *skylist;          /* Sky faces hit by walk */
	int numsky;            /* Number of sky faces in list */
	float cos_fov;         /* Cosine of the field of view angle */

	long max_numverts, max_numelems;
	long r_nummodels, r_numverts, r_numplanes, r_numleafs, r_numnodes;
	long r_numshaders, r_numfaces, r_numlfaces, r_numelems;
	long r_lightmapsize;
	float r_sinfov2;
	float r_cosfov2;
	

	model_t *r_models;
	vertex_t *r_verts;
	plane_t *r_planes;
	leaf_t *r_leafs;
	node_t *r_nodes;
	face_t *r_faces;
	long *r_lfaces;
	long *r_elems;
	visibility_t *r_visibility;


	float r_subdivisiontol;
	long r_maxmeshlevel;
	long r_nummeshes;
	mesh_t *r_meshes;


	skybox_t *r_skybox;

	long g_numentities;

	int r_drawitems;
	int r_stereo;
	int r_fullscreen;
	
	// ui
	int ox, oy;
	int move;
	int strafe;
	double fpstime;
	int fpsframes;
	
	// entity
	char *entity_buf;
	epair_t *epairs;
	entity_t *entities;
	int numepairs;
	
	// bsp
	int _width;
	int _height;
	renderingAttributes_t *_rendering;

	Byte *bspdata;
	long bsplen;

	struct header
	{
	    int id, ver;
	    struct { int fileofs, filelen; } lump[NUM_LUMPS];
	} *bspheader;

    CResourceManager *_resources;
};

#endif	// CBsp_H
