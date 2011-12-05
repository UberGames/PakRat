/* 
	C3DModel.cpp

	Author:			Tom Naughton
	Description:	<describe the C3DModel class here>
*/

#include "C3DModel.h"
#include "CPakStream.h"
#include "CGLImage.h"
#include "CResourceManager.h"

float	avertexnormals[NUMVERTEXNORMALS][3] =
	{
			#include "anorms.h"		// big fat table of normals
	};


C3DModel::C3DModel(CResourceManager *resources)
{
	_textures = new vector<CGLImage*>(10);
	_resources = resources;
	_textureCount = 0;
}

C3DModel::~C3DModel()
{
	// throw away all the textures
	if (_textures) {
		vector<CGLImage*>::iterator e = _textures->begin();
		while (e != _textures->end()) {
			CGLImage* image = *e;
			if (image)
				delete image;
			e++;
		}	
		delete _textures;
	}
}


SInt16 C3DModel::frameCount()
{
	return 0;
}

UInt32 C3DModel::tagNum()
{
	return 0;
}

UInt32 C3DModel::meshNum()
{
	return 0;
}

ModelType C3DModel::modelType()
{
	return unknown_model;
}

Boolean C3DModel::shouldDraw(renderingAttributes_t *rendering, CShader *shader)
{
	Boolean result = rendering->_renderOpaque;
	if (shader) {
		result = (shader->isTransparent() && rendering->_renderBlend) 
			|| (!shader->isTransparent() && rendering->_renderOpaque)
			|| rendering->_renderTextureCoordinates
			|| rendering->_renderTexturePreview;
	}
	return result;
}

Boolean C3DModel::needsNormals(renderingAttributes_t *rendering, CShader *shader)
{
	Boolean result = rendering->_lighting || rendering->_showNormals;
	if (shader)
		result = result || shader->needsNormals();
	return result;
}

void C3DModel::loadMeshTextures(CModelInstance *instance)
{
#pragma unused (instance)
}

int C3DModel::tagIndexWithName(const char *name)
{
#pragma unused (name)
	return -1;
}

string C3DModel::tagNameForFrameAtIndex(short frame, short index)
{
#pragma unused (frame, index)
	return "";
}

md3_tag_t *C3DModel::tagForFrameAtIndex(short frame, short index)
{
#pragma unused (frame, index)
	return 0;
}


void C3DModel::DrawLinks(CModelInstance *instance, renderingAttributes_t *rendering) 
{	
#pragma unused (instance, rendering)
}

Mat4 C3DModel::PushTagTransformMatrix(CModelInstance *instance, int tagindex)
{
#pragma unused (instance, tagindex)
	Mat4 result;
	return result;
}

Boolean C3DModel::canExportFormat(ExportFormatT format)
{
#pragma unused (format)
	return false;
}


 CMd3AnimationInfo *C3DModel::animationInfo() 
{
	return 0;
}

void C3DModel::addTexture(CGLImage* texture)
{
	(*_textures)[_textureCount] = texture;
	_textureCount++;
}


CGLImage *C3DModel::textureWithName(string textureName, string skinnname) 
{
#pragma unused (textureName,  skinnname)
	string keyname = textureName;
	CGLImage *texture = 0;

	texture = _textureMap[keyname]; 
	if (texture)
		return texture;
	return texture;
} 



void C3DModel::setTextureWithName(CGLImage *texture, string textureName) 
{ 
	_textureMap[textureName] = texture; 
} 


/*
 	Function:	Interpolate
 	Usage:		valut = Interpolate(interpSet, u);
 	---------------------------------------------------------------------------
	Interpolate ranges from 0.0 to 1.0
		where	0.0 is for at p0 (pn1 for cubic)
		and		1.0 is for at p1 (p0 for cubic)
	
	Method			Result
	INTERP_NONE		p0
	INTERP_LINEAR	straight line between p0 and p1
	INTERP_CUBIC	smooth curve between pn1 and p0
	
	Note that since out of 4 points are needed for cubic and the interpolation
	is done between the middle two, so without knowing 2 frames ahead what frame
	will come, cubic interpolations are lagged behind by a frame.
	
	The uLast value is stored as well as u2 and u3 so that repetative u's
	don't require recalculating u2 and u3.
 */
 
float C3DModel::Interpolate(float p[], float u, InterpMethodT method)
{
	// to make indexing of the points array p[] make more sense
	#define pn2	0
	#define pn1	1
	#define p0	2
	#define p1	3

	static float	uLast = 0;	// u value for last interpolation
	static float	u2 = 0;		// u^2 value (preserved between calls)
	static float	u3 = 0;		// u^3 value (preserved between calls)
//	float			a,b,c,d;	// parameters for cubic interpolation
	float			tn1, t0;	// tangents at 2 points being interpolated
	
	switch(method)
	{
		case INTERP_NONE:
			return p[p0];
			// break;
			
		case INTERP_LINEAR:
			// values are frequently the same, so we will optimize for this
			if(p[p0] == p[p1])
				return p[p0];
			else
				return (p[p0] * (1 - u) + p[p1] * (u));
		
		case INTERP_CUBIC:
			/*	Since we don't know 2 frames ahead, we will interpolate between pn1 and p0
				as in:
				           (interpolate a
				             curve HERE)
					      pn1 --------- p0
					     /                \
					    /                  \
					   /                    \
					 pn2                     \
					                          p1
				
				Points along an interpolated curve are returned according to the parameter u.
				The curve is made using what I believe is called a Cardinal Spline algorithm.
				For more info on interpolating splines, look up Hermite Splines, Cardinal
				Splines, or Cubic Spline Interpolation Methods.
			 */

			// only calculate u2 and u3 if u has changed
			if(u != uLast)
			{
				//u = interpolate;
				u2 = u*u;
				u3 = u2*u;
				uLast = u;
			}
			
			/*
			// Simple, unoptimized method of computing interpolated value
			
			// tangent at pn1
			t0 = (p[p1] - p[pn1]) / 2;
			// tangent at p0
			tn1 = (p[p0] - p[pn2]) / 2;
			
			a = (2 * p[pn1]) + (-2 * p[p0]) + (1 * tn1) + (1 * t0);
			b = (-3 * p[pn1]) + (3 * p[p0]) + (-2 * tn1) + (-1 * t0);
			c = (0 * p[pn1]) + (0 * p[p0]) + (1 * tn1) + (0 * t0);
			d = (1 * p[pn1]) + (0 * p[p0]) + (0 * tn1) + (0 * t0);
			
			return a*u3 + b*u2 + c*u + d;
			*/
			
			// slightly optimized (I think):
			tn1 = ((p[p0] - p[pn2]) / 2);
			t0 = ((p[p1] - p[pn1]) / 2);
			
			return
				(
					+	(	+ (2 * p[pn1]) 
							+ (-2 * p[p0])
							+ (1 * (tn1))
							+ (1 * (t0))
						) * u3
						
					+	(	+ (-3 * p[pn1])
							+ (3 * p[p0])
							+ (-2 * tn1)
							+ (-1 * t0)
						) * u2
						
					+ 	(	+ tn1
						) * u
					
					+	(	+ p[pn1]
						) * 1
				);

			//break;
			
		default:
			dprintf("Invalid interpolation method\n");
			break;
	}
	
	return 0;	// actual return done by the switch cases
}

string C3DModel::frameName(SInt16 frame)
{
#pragma unused (frame)
	return string("unimplemented");
}
