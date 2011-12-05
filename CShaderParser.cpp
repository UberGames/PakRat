/* 
	CShaderParser.cpp

	Author:			Tom Naughton
	Description:	<describe the CShaderParser class here>
*/

#include <agl.h>
#include <stdlib.h>

#include "CShaderParser.h"

#define SHADER_ARGS_MAX (SHADER_ANIM_FRAMES_MAX+1)
#define LOWERCASE(c) ((c) <= 'Z' && (c) >= 'A' ? (c) + ('a'-'A') : (c))
#define stricmp _stricmp

static void shader_parsefunc(CShader *shader, char **args, shaderfunc_t *func);
static void shader_cull(CShader *shader, shaderpass_t *pass, int numargs, char **args);
static void shader_surfaceparm(CShader *shader, shaderpass_t *pass, int numargs,   char **args);
static void shader_skyparms(CShader *shader, shaderpass_t *pass, int numargs, char **args);
static void shader_nomipmaps(CShader *shader, shaderpass_t *pass, int numargs, char **args);
static void shader_nopicmip(CShader *shader, shaderpass_t *pass, int numargs, char **args);
static void shader_deformvertexes(CShader *shader, shaderpass_t *pass, int numargs, char **args);
static void shader_fogparams (CShader *shader, shaderpass_t *pass, int numargs, char **args);
static void shader_sort (CShader *shader, shaderpass_t *pass, int numargs, char **args);
static void shader_q3map (CShader *shader, shaderpass_t *pass, int numargs, char **args);
static void shader_portal(CShader *shader, shaderpass_t *pass, int numargs, char **args);
static void shader_entitymergable (CShader *shader, shaderpass_t *pass, int numargs, char **args);
static void shader_polygonoffset (CShader *shader, shaderpass_t *pass, int numargs, char **args);


/****************** shader pass keyword functions *******************/

static void shaderpass_map(CShader *shader, shaderpass_t *pass, int numargs, char **args);
static void shaderpass_animmap(CShader *shader, shaderpass_t *pass, int numargs,  char **args);
static void shaderpass_clampmap(CShader *shader, shaderpass_t *pass, int numargs, char **args);
static void shaderpass_rgbgen(CShader *shader, shaderpass_t *pass, int numargs,char **args);
static void shaderpass_blendfunc(CShader *shader, shaderpass_t *pass, int numargs,  char **args);
static void shaderpass_depthfunc(CShader *shader, shaderpass_t *pass, int numargs, char **args);
static void shaderpass_depthwrite(CShader *shader, shaderpass_t *pass, int numargs, char **args);
static void shaderpass_alphafunc(CShader *shader, shaderpass_t *pass, int numargs,  char **args);
static void shaderpass_tcmod(CShader *shader, shaderpass_t *pass, int numargs, char **args);
static void shaderpass_tcgen(CShader *shader, shaderpass_t *pass, int numargs,char **args);
static void shaderpass_alphagen (CShader *shader, shaderpass_t *pass, int numargs,char **args);
static void Syntax(void);


static shaderkey_t shaderpasskeys[] =
{
    {"map", 1, 1, shaderpass_map},
    {"rgbgen", 1, 6, shaderpass_rgbgen},
    {"blendfunc", 1, 2, shaderpass_blendfunc},
    {"depthfunc", 1, 1, shaderpass_depthfunc},
    {"depthwrite", 0, 0, shaderpass_depthwrite},
    {"alphafunc", 1, 1, shaderpass_alphafunc},
    {"tcmod", 2, 7, shaderpass_tcmod},
    {"animmap", 3, SHADER_ARGS_MAX, shaderpass_animmap},
    {"clampmap", 1, 1, shaderpass_clampmap},
    {"tcgen", 1, 10, shaderpass_tcgen},
	{"alphagen",0,10,shaderpass_alphagen},
	
    {NULL, 0, 0, NULL}  // Sentinel 
};


static shaderkey_t shaderkeys[] =
{
    {"cull", 1, 1, shader_cull},
    {"surfaceparm", 1, 1, shader_surfaceparm},
    {"skyparms", 3, 3, shader_skyparms},
    {"nomipmaps", 0, 0, shader_nomipmaps},
	{"nopicmip",0,0,shader_nopicmip},
	{"polygonoffset",0,0,shader_polygonoffset },
	{"sort",1,1,shader_sort},
    {"deformvertexes", 1, 9, shader_deformvertexes},
	{"q3map_lightimage",0,9,shader_q3map},
	{"q3map_globaltexture",0,9,shader_q3map},
	{"q3map_surfacelight",0,9,shader_q3map},
	{"q3map_flare",0,9,shader_q3map},
	{"tesssize",0,9,shader_q3map},
	{"qer_editorimage",0,9,shader_q3map},
	{"qer_trans",0,9,shader_q3map},
	{"qer_nocarve",0,9,shader_q3map},
	{"q3map_sun",0,9,shader_q3map},
	{"q3map_lightsubdivide",0,9,shader_q3map},
	{"light",0,1,shader_q3map }, // (?)
	{"portal",0,0,shader_portal},
	{"entitymergable",0,0,shader_entitymergable},
	{"fogparams",4,4,shader_fogparams},
    {NULL, 0, 0, NULL}  /* Sentinel */
};




CShaderParser::CShaderParser()
{
}

CShaderParser::~CShaderParser()
{
}

void CShaderParser::parseShader(CShader *s)
{
    char *tok;

	// Mark shaderref as 'found' 
	s->found = true;
	
	// Set defaults 
	s->flags = 0;
	s->sort=0;
	s->numpasses=0;
	s->numdeforms=0;
	s->skyheight=512.0f;
	s->flush=SHADER_FLUSH_GENERIC;
	s->cull=SHADER_CULL_FRONT;
	
	// Opening brace 
	tok = nexttok();
	if (tok[0] != '{') Syntax();

	while ((tok = nexttok()) != NULL) {
	    if (tok[0] == '{') {					// Start new pass 
	    
			int pass = s->numpasses++;
			shader_readpass(s, pass);
			
	    } else if (tok[0] == '}') {				// End of shader 
			break;
	    } else {
			shader_parsetok(s, NULL, shaderkeys, tok);
		}
	}
	

	s->Finish();
}

CShader *CShaderParser::shaderWithString(CResourceManager *resources, char *s)
{
	// parsing is destructive, parse a copy
	string str = s;
	curpos = (char*)str.c_str();
	endpos = curpos + strlen(s);
	CShader *shader = new CShader(resources, "unnamed",  SHADER_BSP);
	
	parseShader(shader);
	return shader;
}

void CShaderParser::parse(CResourceManager *resources, const char *inFilename, const char *s, UInt32 len)
{
    const char *tok;
    CShader *theShader;
    string temp = s;
    set<string> skipSet;
	
	curpos = (char*)temp.c_str();
	endpos = curpos + len;

    while ((tok = nexttok()) != NULL) {
		theShader = resources->shaderWithName(tok, false);
		if (!theShader || skipSet.find(tok) != skipSet.end()) {
		    shader_skip();
		    continue;
		}
		dprintf("found %s\n", tok);
		skipSet.insert(tok);
		if (inFilename)
			strcpy(theShader->filename, inFilename);
		parseShader(theShader);

	}
}

CShaderIndexList *CShaderParser::index(const char *s, UInt32 length)
{
    const char *tok;
    const char *start;
    CShaderIndexInfo info;
    CShaderIndexList *shaderNames = new CShaderIndexList();
    string temp = s;
	
	start = curpos = (char*)temp.c_str();
	endpos = curpos + length;

    while ((tok = nexttok()) != NULL) {
		strcpy(info.name, tok);
		info.startIndex = curpos - start - strlen(tok);
		shader_skip();
		info.endIndex = curpos - start;
		shaderNames->push_back(info);
	}
	return shaderNames;
}



char* CShaderParser::nexttok(void)
{
    static char token[256];
    char *tok = (char*)&token;
    int tokenlen = 0;
    
    while (curpos < endpos) {
		// Skip leading whitespace 
		while (*curpos == ' ' || *curpos == '\t' || *curpos == '\n' ||
		      *curpos == '\r')
		    if (++curpos == endpos) return NULL;

		// Check for comment 
		if (curpos[0] == '/' && curpos[1] == '/') {
		    // Skip to end of comment line 
		    while (*curpos != '\n' && *curpos != '\r') {
		    	curpos++;
				if (curpos == endpos) return NULL;
			}
		    // Restart with leading whitespace 
		    continue;
		}

		// Seek to end of token 
		while (*curpos != ' ' && *curpos != '\t' && *curpos != '\n' && *curpos != '\r' && tokenlen < 254) {
			*tok++ = *curpos++;
			tokenlen++;
		    if (curpos >= endpos) 
		    	break;
		}

		// Zero whitespace character and advance by one 
		*tok++ = '\0';
		return (char*)&token;
    }
    return NULL;
}

#warning make this nondestructive
char *CShaderParser::nextarg(void)
{
    char *arg;

    while (curpos < endpos)
    {
	// Skip leading whitespace 
	while (*curpos == ' ' || *curpos == '\t')
	    if (++curpos == endpos) return NULL;

	// Check for newline or comment 
	if (*curpos == '\n' || *curpos == '\r' ||
	    (curpos[0] == '/' && curpos[1] == '/'))
	    return NULL;
	
	// Seek to end of token 
	arg = curpos;
	while (*curpos != ' ' && *curpos != '\t' && *curpos != '\n' &&
	      *curpos != '\r')
	    if (++curpos == endpos) break;

	// Zero whitespace character and advance by one 
	*curpos++ = '\0';
	return arg;
    }
    return NULL;
}




// *************************************************************** 




void CShaderParser::shader_skip(void)
{
    char *tok;
    int brace_count;

    // Opening brace 
    tok = nexttok();
    if (tok[0] != '{') Syntax();

    for (brace_count = 1; brace_count > 0 && curpos < endpos; curpos++)
    {
	if (*curpos == '{')
	    brace_count++;
	else if (*curpos == '}')
	    brace_count--;
    }
}

void CShaderParser::shader_readpass(CShader *shader, UInt32 inPass)
{
    char *tok;

	shaderpass_t *pass = &shader->pass[inPass];
	
    /* Set defaults */
    pass->flags = 0;
    pass->texref = 0;
	pass->anim_numframes=0;
    pass->depthfunc = GL_LEQUAL;
    pass->rgbgen = RGB_GEN_NONE;
    pass->num_tc_mod = 0;
	pass->alpha_gen =ALPHA_GEN_DEFAULT ;
	pass->tc_gen=TC_GEN_BASE;
	pass->tc_mod[0].type=SHADER_TCMOD_NONE;
	pass->num_tc_mod=0;
	
	
    while ((tok = nexttok()) != NULL)  {
    
		if (tok[0] == '}') // End of pass 
		    break;

		else
		    shader_parsetok(shader, pass, shaderpasskeys, tok);
    }
	/*
	while (ptr)
	{
		token =COM_ParseExt (ptr,1);
		
		if (!token[0]) continue;

		if (token[0] == '}')
			break;
		else
			shader_parsetok (shader,pass,shaderpasskeys,token ,ptr);
	}
	*/


	// Check some things 

	if (pass->rgbgen==RGB_GEN_NONE)
	{
		pass->rgbgen=RGB_GEN_IDENTITY;

		;
	}


/*    char *tok;
    shaderpass_t *thePass = &shader->pass[inPass];

    // Set defaults 
    thePass->flags = 0;
    thePass->texref = 0;
    thePass->texflags = 0;
    thePass->depthfunc = GL_LEQUAL;
    thePass->rgbgen = SHADER_GEN_IDENTITY;
    thePass->tcmod = 0;
    
    while ((tok = nexttok()) != NULL)  {
    
		if (tok[0] == '}') // End of pass 
		    break;

		else
		    shader_parsetok(shader, thePass, shaderpasskeys, tok);
    }
*/
}

void CShaderParser::shader_parsetok(CShader *shader, shaderpass_t *pass, shaderkey_t *keys, char *tok)
{
    shaderkey_t *key;
    char *c, *args[SHADER_ARGS_MAX];
    int numargs;

    // Lowercase the token 
    c = tok;
    while (*c++) *c =  LOWERCASE(*c);
    
    // FIXME: This should be done with a hash table! 

    for (key = keys; key->keyword != NULL; key++) {
		if (stricmp(tok, key->keyword) == 0) {
		    for (numargs=0; (c = nextarg()) != NULL; numargs++) {
				// Lowercase the argument 
				args[numargs] = c;
				while (*c) 
					{*c = LOWERCASE(*c); c++;}
		    }
		    if (numargs < key->minargs || numargs > key->maxargs)
				Syntax();
		    
		    if (key->func)
				key->func(shader, pass, numargs, args);
		    return;
		}
    }

    // Unidentified keyword: no error for now, just advance to end of line 
    while (*curpos != '\n' && *curpos != '\r')
		if (++curpos == endpos) break;    
}

#pragma mark -


void Syntax(void)
{
    dprintf("Syntax error\n");
}


void shader_parsefunc(CShader *shader, char **args, shaderfunc_t *func)
{
#pragma unused (shader)
	if (!stricmp(args[0], "sin"))
	    func->func = SHADER_FUNC_SIN;
	else if (!stricmp(args[0], "triangle"))
	    func->func = SHADER_FUNC_TRIANGLE;
	else if (!stricmp(args[0], "square"))
	    func->func = SHADER_FUNC_SQUARE;
	else if (!stricmp(args[0], "sawtooth"))
	    func->func = SHADER_FUNC_SAWTOOTH;
	else if (!stricmp(args[0], "inversesawtooth"))
	    func->func = SHADER_FUNC_INVERSESAWTOOTH;
	else if (!stricmp(args[0], "random"))
	    func->func = SHADER_FUNC_RANDOM;
	else {
	    dprintf("WARNING: unknown shade waveform %s\n", args[0]);
	    func->func = SHADER_FUNC_RANDOM;
	}

	func->args[0] = atof(args[1]);
	func->args[1] = atof(args[2]);
	func->args[2] = atof(args[3]);
	func->args[3] = atof(args[4]);
}

static void
shader_cull(CShader *shader, shaderpass_t *pass, int numargs, char **args)
{
#pragma unused (pass)

	if (!stricmp(args[0], "disable") || !stricmp(args[0], "none"))
	{
		shader->cull=SHADER_CULL_DISABLE;
	}
	else if (numargs>1)
	{
		if (!stricmp (args[1] ,"front"))
		{
			shader->cull=SHADER_CULL_FRONT;
		}
		else if (!stricmp (args[1] ,"back"))
		{
			shader->cull=SHADER_CULL_BACK;
		}

	}
	else
	{
		shader->cull=SHADER_CULL_FRONT;
	}

}

static void
shader_surfaceparm(CShader *shader, shaderpass_t *pass, int numargs,
		   char **args)
{
#pragma unused (pass, numargs)
    if (!stricmp(args[0], "trans"))
	{
		shader->flags |= SHADER_TRANSPARENT;
	}
    else if (!stricmp(args[0], "sky"))
	{
		shader->flags |= SHADER_SKY;
	}
	else if (!stricmp(args[0], "nomipmaps"))
	{
		shader->flags |= SHADER_NOMIPMAPS;
	}
	// Is this needed ??
	/*
	else if (!stricmp(args[0], "nomarks"))
		shader->contents|=SURF_NOMARKS;
	else if (!stricmp(args[0], "nonsolid"))
		shader->contents|=SURF_NONSOLID;
	else if (!stricmp(args[0], "nodraw"))
		shader->contents|=SURF_NODRAW;
	else if (!stricmp(args[0], "nodlight"))
		shader->contents|=SURF_NODLIGHT;
	else if (!stricmp(args[0], "structural"))
		shader->contents|=CONTENTS_STRUCTURAL;
	else if (!stricmp(args[0], "metalsteps"))
		shader->contents|=SURF_METALSTEPS	;
	else if (!stricmp(args[0], "playerclip" ))
		shader->contents|=CONTENTS_PLAYERCLIP;
	else if (!stricmp(args[0], "alphashadow"))
		shader->contents|=SURF_ALPHASHADOW;
	else if (!stricmp(args[0], "nolightmap" ))
		shader->contents|=SURF_NOLIGHTMAP;
	else if (!stricmp(args[0], "noimpact" ))
		shader->contents|=SURF_NOIMPACT;
	else if (!stricmp(args[0] , "lava" ))
		shader->contents|= CONTENTS_LAVA ;
	else if (!stricmp(args[0], "fog" ))
		shader->contents|= CONTENTS_FOG;
	else if (!stricmp(args[0],"nodrop"))
		shader->contents|=CONTENTS_NODROP;
	else if (!stricmp(args[0], "detail"))
		shader->contents|=CONTENTS_DETAIL;
	else if (!stricmp(args[0], "donotenter"))
		shader->contents|=CONTENTS_DONOTENTER;
	else 
	{

		dprintf("Unknown surfaceparam : %s\n",args [0]);
	}
	*/

}

static void
shader_skyparms(CShader *shader, shaderpass_t *pass, int numargs,
		char **args)
{
#pragma unused (pass, numargs)
	shader->flags |= SHADER_SKY;
    shader->skyheight = atof(args[1]);
	// TODO : nearbox , farbox;
}

static void
shader_nomipmaps(CShader *shader, shaderpass_t *pass, int numargs,
		 char **args)
{
#pragma unused (pass, numargs, args)
    shader->flags |= SHADER_NOMIPMAPS;
}

static void
shader_nopicmip(CShader *shader, shaderpass_t *pass, int numargs,
		 char **args)
{
#pragma unused (pass, numargs, args)
    shader->flags |= SHADER_NOPICMIP;
	 shader->flags |= SHADER_NOMIPMAPS;
}

static void
shader_deformvertexes(CShader *shader, shaderpass_t *pass, int numargs,
		      char **args)
{
#pragma unused (pass, numargs )
    shader->flags |= SHADER_DEFORMVERTS;
    


	if (shader->numdeforms == MAX_TC_MOD )
		DebugStr ("\pMAX_TC_MOD exceeded !");   
		
		
	if (!stricmp(args[0], "wave"))
	{
		shader->flags |= SHADER_DEFORMVERTS;
		shader->deform_vertices[shader->numdeforms].deform_params[0] =atof (args[1]);
		shader_parsefunc(shader, &args[2], &shader->deform_vertices[shader->numdeforms].deformv_wavefunc);
		
		shader->deform_vertices[shader->numdeforms].type=DEFORMV_WAVE;
	}
	else if (!stricmp(args[0],"normal"))
	{
		shader->flags |= SHADER_DEFORMVERTS;
		shader->deform_vertices[shader->numdeforms].type=DEFORMV_NORMAL;
		shader->deform_vertices[shader->numdeforms].deform_params[0] =atof (args[1]); // Div
		shader_parsefunc(shader, &args[2], &shader->deform_vertices[shader->numdeforms].deformv_wavefunc);
		
	}
	else if (!stricmp(args[0],"bulge"))
	{
		shader->flags |= SHADER_DEFORMVERTS;
		shader->deform_vertices[shader->numdeforms].type=DEFORMV_BULGE;
	
		shader->deform_vertices[shader->numdeforms].deform_params[0] = atof (args[1]); // Width 
		shader->deform_vertices[shader->numdeforms].deform_params[1] =atof (args[2]); // Height
		shader->deform_vertices[shader->numdeforms].deform_params[2] = atof (args[3]); // Speed 
	}
	else if (!stricmp (args[0] , "move"))
	{
		shader->flags |= SHADER_DEFORMVERTS;
		shader->deform_vertices[shader->numdeforms].type=DEFORMV_MOVE;

		shader->deform_vertices[shader->numdeforms].deform_params[0] = atof (args[1]); // x 
		shader->deform_vertices[shader->numdeforms].deform_params[1] = atof (args[2]); // y
		shader->deform_vertices[shader->numdeforms].deform_params[2] = atof (args[3]); // z
		
		shader_parsefunc(shader, &args[4], &shader->deform_vertices[shader->numdeforms].deformv_wavefunc);
	}
	else if (!stricmp ( args[0] ,"autosprite"))
	{
		shader->flags |= SHADER_DEFORMVERTS;
		shader->deform_vertices[shader->numdeforms].type= DEFORMV_AUTOSPRITE;

	}
	else if (!stricmp (args[0],"autosprite2"))
	{
		shader->flags |= SHADER_DEFORMVERTS;
		shader->deform_vertices[shader->numdeforms].type= DEFORMV_AUTOSPRITE2;
	}
	else 
	{
		shader->flags |= SHADER_DEFORMVERTS;
		shader->deform_vertices[shader->numdeforms].type= DEFORMV_NONE;
#warning Unknown deformv param
		dprintf ("WARNING: Unknown deformv param : %s \n",args[0]);
	}

	shader->numdeforms++;
}

static void shader_fogparams (CShader *shader, shaderpass_t *pass, int numargs,
		      char **args)
{
#pragma unused (pass, numargs)

	shader->flags	|= SHADER_FOG ;

	shader->fog_params[0]=atof (args[0]); // R
	shader->fog_params[1]=atof (args[1]); // G
	shader->fog_params[2]=atof (args[2]); // B
	shader->fog_params[3]=atof (args[3]); // Dist

}

static void shader_sort (CShader *shader, shaderpass_t *pass, int numargs,
		      char **args)
{
#pragma unused (pass, numargs)
	if( !stricmp( args[0], "portal" ) ) {
		shader->sort = SHADER_SORT_PORTAL;
	} else if( !stricmp( args[0], "sky" ) ) {
		shader->sort = SHADER_SORT_SKY;
	} else if( !stricmp( args[0], "opaque" ) ) {
		shader->sort = SHADER_SORT_OPAQUE;
	} else if( !stricmp( args[0], "banner" ) ) {
		shader->sort = SHADER_SORT_BANNER;
	} else if( !stricmp( args[0], "underwater" ) ) {
		shader->sort = SHADER_SORT_UNDERWATER;
	} else if( !stricmp( args[0], "additive" ) ) {
		shader->sort = SHADER_SORT_ADDITIVE;
	} else if( !stricmp( args[0], "nearest" ) ) {
		shader->sort = SHADER_SORT_NEAREST;
	} else {
		shader->sort = atoi(args[0]);
	}

}

static void shader_q3map (CShader *shader, shaderpass_t *pass, int numargs,
		      char **args)
{
#pragma unused (shader, pass, numargs, args)
	// Just do nothing 


}

static void shader_portal(CShader *shader, shaderpass_t *pass, int numargs,
		      char **args)
{
#pragma unused (pass, numargs, args)
	shader->sort = SHADER_SORT_PORTAL;
}

static void shader_entitymergable (CShader *shader, shaderpass_t *pass, int numargs,
		      char **args)
{
#pragma unused (shader, pass, numargs, args)
	// TODO ! (? )

}
static void shader_polygonoffset (CShader *shader, shaderpass_t *pass, int numargs,
		      char **args)
{
#pragma unused (pass, numargs, args)
	shader->flags|=SHADER_POLYGONOFFSET;
}


/****************** shader pass keyword functions *******************/

static void
shaderpass_map(CShader *shader, shaderpass_t *pass, int numargs, char **args)
{
#pragma unused (numargs)
	if (!stricmp(args[0], "$lightmap"))
	{
		pass->tc_gen=TC_GEN_LIGHTMAP;
		pass->flags |= SHADER_LIGHTMAP;

	}
	/*
	else if (!stricmp (args[0],"$whiteimage"))
	{
		pass->tc_gen=TC_GEN_BASE;
		pass->texref=shader->_resources->textureWithName("white",shader->flags );
	}
	*/
	else
    {
			pass->tc_gen=TC_GEN_BASE;
			pass->texref=shader->_resources->textureWithName(args[0],shader->flags );
    }
	


}

static void
shaderpass_animmap(CShader *shader, shaderpass_t *pass, int numargs,
		   char **args)
{
    int i;
	pass->tc_gen= TC_GEN_BASE;
    pass->flags |= SHADER_ANIMMAP;
    pass->anim_fps = atof(args[0]);
    pass->anim_numframes = numargs - 1;
    for (i=1; i < numargs; ++i)
	{

		pass->anim_frames[i-1]=shader->_resources->textureWithName(args[i],shader->flags );
	}
}

static void
shaderpass_clampmap(CShader *shader, shaderpass_t *pass, int numargs,
		    char **args)
{
#pragma unused (numargs)

	pass->tc_gen= TC_GEN_BASE;
    pass->texref = shader->_resources->textureWithName(args[0],shader->flags | SHADER_CLAMP);
}

static void
shaderpass_rgbgen(CShader *shader, shaderpass_t *pass, int numargs,
		  char **args)
{


	if (!stricmp(args[0] , "identitylighting"))
	{
		pass->rgbgen=RGB_GEN_IDENTITY_LIGHTING;

	}
	else if (!stricmp(args[0] ,"identity"))
	{
		pass->rgbgen=RGB_GEN_IDENTITY;
	}
	else if (!stricmp (args[0],"wave"))
	{
		if (numargs!=6)
			Syntax();

		shader_parsefunc(shader, &args[1], &pass->rgbgen_func);
		pass->rgbgen=RGB_GEN_WAVE;
	}
	else if (!stricmp(args[0],"entity"))
	{
		pass->rgbgen=RGB_GEN_ENTITY;
	}
	else if (!stricmp(args[0],"oneMinusEntity"))
	{
		pass->rgbgen=RGB_GEN_ONE_MINUS_ENTITY;
	}
	else if (!stricmp(args[0],"Vertex"))
	{
		pass->rgbgen=RGB_GEN_VERTEX;
	}
	else if (!stricmp(args[0],"oneMinusVertex"))
	{
		pass->rgbgen=RGB_GEN_ONE_MINUS_VERTEX;
	}
	else if (!stricmp(args[0],"lightingDiffuse"))
	{
		pass->rgbgen=RGB_GEN_LIGHTING_DIFFUSE ;

	}
	else if (!stricmp (args[0],"exactvertex"))
	{
		pass->rgbgen=RGB_GEN_EXACT_VERTEX;
	}
	else {
		dprintf ("WARNING : Unknown rgb_gen param : % s \n",args[0]);
	}

}

static void
shaderpass_blendfunc(CShader *shader, shaderpass_t *pass, int numargs,
		     char **args)
{
#pragma unused (shader)
   pass->flags |= SHADER_BLEND;
    
    if (numargs == 1)
    {
	if (!stricmp(args[0], "blend"))
	{
	    pass->blendsrc = GL_SRC_ALPHA;
	    pass->blenddst = GL_ONE_MINUS_SRC_ALPHA;
	}
	else if (!stricmp(args[0], "filter"))
	{
	    pass->blendsrc = GL_DST_COLOR;
	    pass->blenddst = GL_ZERO;
	}
	else if (!stricmp(args[0], "add"))
	{
	    pass->blendsrc = pass->blenddst = GL_ONE;
	}
	else
	    Syntax();
    }
    else
    {
	int i;
	UInt32 *blend;
	for (i=0; i < 2; ++i)
	{
	    blend = i == 0 ? &pass->blendsrc : &pass->blenddst;
	    if (!stricmp(args[i], "gl_zero"))
		*blend = GL_ZERO;
	    else if (!stricmp(args[i], "gl_one"))
		*blend = GL_ONE;
	    else if (!stricmp(args[i], "gl_dst_color"))
		*blend = GL_DST_COLOR;
	    else if (!stricmp(args[i], "gl_one_minus_src_alpha"))
		*blend = GL_ONE_MINUS_SRC_ALPHA;
	    else if (!stricmp(args[i], "gl_src_alpha"))
		*blend = GL_SRC_ALPHA;
	    else if (!stricmp(args[i], "gl_src_color"))
		*blend = GL_SRC_COLOR;
	    else if (!stricmp(args[i], "gl_one_minus_dst_color"))
		*blend = GL_ONE_MINUS_DST_COLOR;
	    else if (!stricmp(args[i], "gl_one_minus_src_color"))
		*blend = GL_ONE_MINUS_SRC_COLOR;
	    else if (!stricmp(args[i], "gl_dst_alpha"))
		*blend = GL_DST_ALPHA;
	    else if (!stricmp(args[i], "gl_one_minus_dst_alpha"))
		*blend = GL_ONE_MINUS_DST_ALPHA;
	    else
		Syntax();
	}
    }
}

static void
shaderpass_depthfunc(CShader *shader, shaderpass_t *pass, int numargs,
		     char **args)
{
#pragma unused (shader, numargs)
    if (!stricmp(args[0], "equal"))
		pass->depthfunc = GL_EQUAL;
    else
		Syntax();
}

static void
shaderpass_depthwrite(CShader *shader, shaderpass_t *pass, int numargs,
		      char **args)
{
#pragma unused (numargs, args)
    /* FIXME: Why oh why is depthwrite enabled in the sky shaders ???? */
    if (shader->flags & SHADER_SKY) return;
    
    shader->flags |= SHADER_DEPTHWRITE;
    pass->flags |= SHADER_DEPTHWRITE;
}

static void
shaderpass_alphafunc(CShader *shader, shaderpass_t *pass, int numargs,
		     char **args)
{
#pragma unused (shader, numargs)
    pass->flags |= SHADER_ALPHAFUNC;
    
    if (!stricmp(args[0], "gt0"))
    {
	pass->alphafunc = GL_GREATER;
	pass->alphafuncref = 0.0f;
    }
	else if (!stricmp (args[0],"lt128"))
	{
	pass->alphafunc = GL_LESS;
	pass->alphafuncref = 0.5f;

	}
    else if (!stricmp(args[0], "ge128"))
    {
	pass->alphafunc = GL_GEQUAL;
	pass->alphafuncref = 0.5f;
    }
    else
	{
		dprintf("WARNING: Unknown alphafunc param : %s \n",args[0]);
	}
}

static void
shaderpass_tcmod(CShader *shader, shaderpass_t *pass, int numargs,
		 char **args)
{
    pass->flags |= SHADER_TCMOD;
    
;

	if (pass->num_tc_mod == MAX_TC_MOD )
		DebugStr ("\pMAX_TC_MOD exceeded !");
	

	if (!stricmp(args[0],"rotate"))
	{
		pass->tc_mod[pass->num_tc_mod].type=SHADER_TCMOD_ROTATE;
		pass->tc_mod[pass->num_tc_mod].args[0] = atof(args[1]);
	}
	else if (!stricmp (args[0],"scale"))
	{
		if (numargs != 3) Syntax();

		pass->tc_mod[pass->num_tc_mod].type=SHADER_TCMOD_SCALE;
		pass->tc_mod[pass->num_tc_mod].args[0]=atof (args[1]);
		pass->tc_mod[pass->num_tc_mod].args[1]=atof (args[2]);
	
	}
	else if (!stricmp(args[0],"scroll"))
	{

		if (numargs != 3) Syntax();

		pass->tc_mod[pass->num_tc_mod].type =SHADER_TCMOD_SCROLL;
		pass->tc_mod[pass->num_tc_mod].args[0]=atof (args[1]);
		pass->tc_mod[pass->num_tc_mod].args[1]=atof (args[2]);

	}
	else if (!stricmp (args[0],"stretch"))
	{
		if (numargs != 6) Syntax();
		pass->tc_mod[pass->num_tc_mod].type =SHADER_TCMOD_STRETCH;
		shader_parsefunc(shader, &args[1], &pass->tc_mod_stretch);

	}
	else if (!stricmp (args[0],"transform"))
	{
			int i;
		if (numargs != 7) Syntax();
		pass->tc_mod[pass->num_tc_mod].type= SHADER_TCMOD_TRANSFORM;
		for (i=0; i < 6; ++i)
			pass->tc_mod[pass->num_tc_mod].args[i] = atof(args[i+1]);
	}
	else if (!stricmp (args[0],"turb"))
	{
		int i, a1=0;
		if (numargs == 5)
			a1 = 1;
		else if (numargs == 6)
			a1 = 2;
		else
			Syntax();
		pass->tc_mod[pass->num_tc_mod].type= SHADER_TCMOD_TURB;
	for (i=0; i < 4; ++i)
	    pass->tc_mod[pass->num_tc_mod].args[i] = atof(args[i+a1]);
   
	}
	else 
	{
		dprintf ("WARNING: Unknown tc_mod : %s \n",args[0]);
		pass->tc_mod[pass->num_tc_mod].type=-1;
	}

	pass->num_tc_mod++ ;
}


static void
shaderpass_tcgen(CShader *shader, shaderpass_t *pass, int numargs,
		 char **args)
{
#pragma unused (shader, numargs)
    
	if (!stricmp(args[0],"base"))
	{
		pass->tc_gen=TC_GEN_BASE;
	}
	else if (!stricmp(args[0],"lightmap"))
	{
		pass->tc_gen=TC_GEN_LIGHTMAP;
	}
	else if (!stricmp(args[0],"environment"))
	{
		pass->tc_gen=TC_GEN_ENVIRONMENT ;
	}
	else if (!stricmp(args[0],"vector"))
	{
		pass->tc_gen=TC_GEN_VECTOR ;

		pass->tc_gen_s[0] = atof(args[1]);
		pass->tc_gen_s[1] = atof(args[2]);
		pass->tc_gen_s[2] = atof(args[3]);
		pass->tc_gen_t[0] = atof(args[4]);
		pass->tc_gen_t[1] = atof(args[5]);
		pass->tc_gen_t[2] = atof(args[6]);
	}
	else
	{
		dprintf ("unknown tcgenparam : %s \n",args[0]);
	}
}

static void shaderpass_alphagen (CShader *shader, shaderpass_t *pass, int numargs,
		 char **args)
{
#pragma unused (shader, numargs)

	if (!stricmp (args[0],"portal"))
	{
		pass->alpha_gen=ALPHA_GEN_PORTAL;
	}
	else if (!stricmp (args[0],"vertex"))
	{
		pass->alpha_gen=ALPHA_GEN_VERTEX ;
	}
	else if (!stricmp (args[0],"entity"))
	{
		pass->alpha_gen=ALPHA_GEN_ENTITY;
	}
	else if (!stricmp (args[0],"lightingspecular"))
	{
		pass->alpha_gen=ALPHA_GEN_LIGHTINGSPECULAR;
	}
	else
	{
		pass->alpha_gen=ALPHA_GEN_DEFAULT;
		dprintf ("unknown alphagen param : %s \n",args[0]);
	}



}