

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <agl.h>
#include <glut.h>
#include <aglRenderers.h>

#include "GLPane.h"


#define AGL_RENDERER_VOODOO3_ID AGL_RENDERER_GENERIC_ID|AGL_RENDERER_ATI_ID

#define	VooDooWarning "You are trying to run a single buffered application using VooDoo drivers.\nThis will not work.\nInside your application set \"doubleBuffer\" to true and add the following instruction to your display method: \"glj.gljSwap()\"\n"

#define PIXEL_FROMAT_LIST_SIZE	20

typedef struct
{
	GLboolean	valid;
	
	GLint		id;
	
	GLboolean	accelerated;
	
	GLint		depthModes;
	
	GLint		colorModes;
	
	GLint		stencilModes;
	
	GLboolean	singleBuffer;
	GLboolean	doubleBuffer;
	GLboolean	mono;
	GLboolean	stereo;
}
RendererRect, *RendererRectPtr, **RendererRectHnd;

typedef struct
{
	UInt32			count;
	
	RendererRectPtr	info;
	
	GLint			attributeList[PIXEL_FROMAT_LIST_SIZE];
}
RenderersInfoRect, *RenderersInfoRectPtr, **RenderersInfoRectHnd;

//
// Private Function Prottypes
//


static AGLContext	get_GC( AGLDrawable glDrawable, Boolean verbose, Boolean *doubleBuffer, Boolean *stereo);

static void			chooseDefaultPixelFormat( GLint (*pixelFormatList)[PIXEL_FROMAT_LIST_SIZE]);
static void			printPixelFormat( GLint pixelFormatList[PIXEL_FROMAT_LIST_SIZE]);
static GLboolean	isColorARGB( GLint colorModes);
static GLint		chooseBestColor( GLint colorModes);
static GLint		chooseBestDepth( GLint depthModes);
static GLint		mapDepth( GLint depthModes);
static void			getRenderersInfo( RenderersInfoRectPtr info);
static void			printRendererInfo( RendererRect info);
static void			chooseBestRenderer( RenderersInfoRectPtr info, GLboolean *doubleBuffer, GLboolean *stereo);

static void			printAllRenderer();



// ---------------------------------------------------------------------------
//	¥ GLPane						Parameterized Constructor [public]
// ---------------------------------------------------------------------------
// desc is a string of pixel format attributes

GLPane::GLPane( const SPaneInfo&	inPaneInfo, LStr255& desc) : LPane(inPaneInfo)
{
	mCtx = nil;
	SetupAGL(desc);
	//aglSetCurrentContext(mCtx);
}


// ---------------------------------------------------------------------------
//	¥ GLPane						Stream Constructor		  [public]
// ---------------------------------------------------------------------------

GLPane::GLPane( LStream*	inStream) : LPane(inStream)
{
	mCtx = nil;
	unsigned char desc[255];
	inStream->ReadData( desc, 255 );
	SetupAGL(desc);
	//aglSetCurrentContext(mCtx);
}


// ---------------------------------------------------------------------------
//	¥ ~GLPane						Destructor				  [public]
// ---------------------------------------------------------------------------

GLPane::~GLPane()
{
	//aglSetCurrentContext(NULL);
	aglSetDrawable(mCtx, NULL);
	aglDestroyContext(mCtx);
}
#pragma mark -


// ---------------------------------------------------------------------------
//	¥ Draw															  [public]
// ---------------------------------------------------------------------------

void GLPane::DrawSelf()
{
	//aglSetCurrentContext(NULL);
	ForeColor(blackColor);
	BackColor(whiteColor);
	//aglSetCurrentContext(mCtx);
	DrawGL();
	SwapBuffers();
}


void GLPane::SwapBuffers()
{
	FocusDraw();
	glFlush();
	aglSwapBuffers(mCtx);
}

Boolean
GLPane::FocusDraw(
	LPane*	 inSubPane )
{
	LPane::FocusDraw(inSubPane);
	if (mCtx) {
		aglSetCurrentContext(mCtx);
		
		// need to update context whenever the window is moved
		// to avoid a problem with GeForce 2
		aglUpdateContext(mCtx);
	}
	return true;
}


void GLPane::SetupAGL(unsigned char *desc)
{
#pragma unused (desc)
	StGrafPortSaver savePort;
	//AGLPixelFormat 	fmt;
	GLboolean      	ok = true;
	//fmt = aglChoosePixelFormat(NULL, 0, attrib);
	//ThrowIfNil_(fmt);
	
	/*
	mCtx = aglCreateContext(fmt, NULL);
	ThrowIfNil_(mCtx);
	
	//ok = aglSetDrawable(mCtx,(CGrafPort*) mGrafPort);
	FocusDraw();
	GrafPtr		thePort;		// Verify Port setting
	::GetPort(&thePort);
	ok = aglSetDrawable(mCtx,(CGrafPort*)thePort);
	*/
	
	FocusDraw();
	GrafPtr		thePort;		// Verify Port setting
	::GetPort(&thePort);
	Boolean verbose = true, jdoubleBuffer = true, jstereoView = false;
#if PP_Target_Carbon
    mCtx = get_GC(thePort, verbose, &jdoubleBuffer, &jstereoView);
#else
    mCtx = get_GC((CGrafPort*)thePort, verbose, &jdoubleBuffer, &jstereoView);
#endif

	ThrowIfNil_(ok);
	
//	ok = aglSetCurrentContext(mCtx);
//	ThrowIfNil_(ok);
	
	//aglDestroyPixelFormat(fmt);
// FIXME - debugging complained about this delete
//	delete attrib;
	
	InitGL();
	SDimension16	l_PortDim;
	GetFrameSize(l_PortDim);
	ReshapeGL(l_PortDim.width, l_PortDim.height);
}



void GLPane::ClickSelf(const SMouseDownEvent&	inMouseDown )
{
	Point    theOldMouseLoc;
	Point    theNewMouseLoc;

	FocusDraw();

	theOldMouseLoc = inMouseDown.whereLocal;
	MouseDown(inMouseDown, theOldMouseLoc);
	theNewMouseLoc = theOldMouseLoc;

//	while (::StillDown()) {
	while (::Button()) {
		::GetMouse(&theNewMouseLoc);
		if (1 || ::EqualPt(theNewMouseLoc, theOldMouseLoc) == false) {
			MouseDrag(inMouseDown, theNewMouseLoc);
			theOldMouseLoc = theNewMouseLoc;
		}
	}
}


void GLPane::ResizeFrameBy(SInt16		inWidthDelta,
						   SInt16		inHeightDelta,
						   Boolean		inRefresh)
{
	// Resize Pane
	//printf("ResizeFrameBy(inWidthDelta %d, inHeightDelta %d, inRefresh %d)\n", inWidthDelta, inHeightDelta, inRefresh);
	//aglSetCurrentContext(NULL);
	LPane::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);
	SDimension16 portSize;
	GetFrameSize(portSize);
	aglSetCurrentContext(mCtx);
	ReshapeGL(portSize.width, portSize.height);
}


// draw immediatly the panel
void GLPane::Redraw()
{
	FocusDraw();
	ForeColor(blackColor);
	BackColor(whiteColor);
	//aglSetCurrentContext(mCtx);
	DrawGL();
	SwapBuffers();	
}


void GLPane::MoveBy(SInt32 inHorizDelta,SInt32 inVertDelta,Boolean inRefresh)
{
	//printf("MoveBy(inHorizDelta %d, inVertDelta %d, inRefresh %d)\n", inHorizDelta, inVertDelta, inRefresh);
	LPane::MoveBy(inHorizDelta, inVertDelta, inRefresh);
}

void GLPane::ShowSelf()
{
	LPane::ShowSelf();
}

// this function called whenever pane changes size, as well as the first time the pane is
// created
#pragma mark -
void GLPane::ReshapeGL(int width, int height)
{
	FocusDraw();
    glViewport(0, 0, width, height);    
	aglUpdateContext(mCtx);
#pragma unused (width, height)
}

// called once at pane creation	
void GLPane::InitGL()
{
	
	dprintf("GLPane::InitGL\n");
	
//	Other calls might include: glEnable(GL_LIGHTING) or glEnable(GL_DEPTH_TEST);
    
	SDimension16 l_PortDim;
	GetFrameSize(l_PortDim);
	ReshapeGL(l_PortDim.width, l_PortDim.height);

}

// Draw GL content. If pane is double buffer call SwapBuffers() at end
void GLPane::DrawGL()
{

/*
	// draw the lights
	glPointSize(6);
	glBegin(GL_POINTS);
	glColor3f(light0_diffuse[0], light0_diffuse[1], light0_diffuse[2]);
	glVertex3f(light0_position[0], light0_position[1], light0_position[2]);		
	glColor3f(light1_diffuse[0], light1_diffuse[1], light1_diffuse[2]);
	glVertex3f(light1_position[0], light1_position[1], light1_position[2]);		
	glEnd();
	glPointSize(1);
	glFlush();
*/

}


// handle mouse down
void GLPane::MouseDown(const SMouseDownEvent&, const Point&)
{
}

// handle mouse drag
void GLPane::MouseDrag(const SMouseDownEvent&, const Point&)
{
}

#pragma mark -
//
// AGLContext
//

#if 0 // PP_Target_Carbon

static AGLContext get_GC(AGLDrawable glDrawable, Boolean verbose, Boolean *doubleBuffer, Boolean *stereo)
{
	GLint          attrib[] = { AGL_RGBA, AGL_NONE, AGL_DOUBLEBUFFER, AGL_ACCELERATED  };
	AGLPixelFormat fmt;
	AGLContext     ctx;
	GLboolean      ok;

	/* Choose an rgb pixel format */
	fmt = aglChoosePixelFormat(NULL, 0, attrib);
	if(fmt == NULL) return NULL;


	/* Create an AGL context */
	ctx = aglCreateContext(fmt, NULL);
	if(ctx == NULL) return NULL;

	/* Attach the window to the context */
	GrafPtr		thePort;		// Verify Port setting
	::GetPort(&thePort);
	ok = aglSetDrawable(ctx, thePort);
//	ok = aglSetDrawable(ctx, win);
	if(!ok) return NULL;
	
	/* Make the context the current context */
//	ok = aglSetCurrentContext(ctx);
//	if(!ok) return NULL;

	/* Pixel format is no longer needed */
	aglDestroyPixelFormat(fmt);

	return ctx;

}

#else
static AGLContext get_GC(AGLDrawable glDrawable, Boolean verbose, Boolean *doubleBuffer, Boolean *stereo)
{
	GDHandle			device		= GetMainDevice();
	AGLContext			context		= nil;
	AGLPixelFormat		pixelformat	= nil;
	RenderersInfoRect	info;
	
	
#ifdef _DEBUG_OPENGL_MAC_	
	verbose = true;
    dprintf("in get_GC, glDrawable=0x%X, verbose=%d, doubleBuffer=%d, stereo= %d \n", glDrawable, verbose, *doubleBuffer, *stereo);
#endif
	
	if (glDrawable == nil)
	{
		dprintf("get_GC: Error, glDrawable is nil \n");
		
		return nil;
	}
	
	if (verbose == true)
	{
		printAllRenderer();
	}
	
	chooseDefaultPixelFormat(&info.attributeList);
	getRenderersInfo(&info);
	chooseBestRenderer(&info, doubleBuffer, stereo);
	
	// create pixel format
    if (verbose == true)
    {
		printPixelFormat(info.attributeList);
    }
	pixelformat = aglChoosePixelFormat(&device, 1, info.attributeList);
	if (pixelformat == nil)
	{
		dprintf("get_GC: Error, pixelformat is nil \n");
		if (info.info->id == AGL_RENDERER_VOODOO3_ID)
		{
			dprintf(VooDooWarning);
		}
		
		return nil;
	}
	else if (verbose == true)
	{
		dprintf("pixelformat ok \n");
	}
	
	DisposePtr((Ptr)info.info);
	
	// create AGL context
	context = aglCreateContext(pixelformat, nil);
	if (context == nil)
	{
		dprintf("getGC context could NOT be created \n");
		
		return nil;
	}
	else if (verbose == true)
	{
		dprintf("context ok \n");
		
		long parameter = 0;
		if (aglSetInteger(context, AGL_SWAP_INTERVAL, &parameter) == GL_FALSE)
		{
			dprintf("setting AGL_SWAP_INTERVAL failed \n");
		}
	}
	
	
	// associate the context with the Mac drawable (window)
	if (aglSetDrawable(context, glDrawable) == GL_FALSE)
	{
		dprintf("aglSetDrawable NOT successfull \n");
		
		aglDestroyPixelFormat(pixelformat);
		aglDestroyContext(context);
		
		return nil;
	}
	else if (verbose == true)
	{
		dprintf("aglSetDrawable successfull \n");
	}
	
	if (aglSetCurrentContext(context) == GL_FALSE)
	{
		dprintf("aglSetCurrentContext NOT successfull \n");
		
		aglDestroyPixelFormat(pixelformat);
		aglDestroyContext(context);
		
		return nil;
	}
	else if (verbose == true)
	{
		dprintf("aglSetCurrentContext successfull \n");
	}
	
	if (verbose == true)
	{
		dprintf("AGLContext created:  %d \n", (UInt32)context);
	}
		
	aglDestroyPixelFormat(pixelformat);
	
	return context;
}
#endif

//
// chooseDefaultPixelFormat
//
static void chooseDefaultPixelFormat( GLint (*pixelFormatList)[PIXEL_FROMAT_LIST_SIZE])
{
	(*pixelFormatList)[0] = AGL_RENDERER_ID;
	(*pixelFormatList)[1] = AGL_RENDERER_GENERIC_ID;
	//(*pixelFormatList)[1] = AGL_RENDERER_ATI_ID;
	//(*pixelFormatList)[1] = AGL_RENDERER_GENERIC_ID|AGL_RENDERER_ATI_ID;	// VooDoo?
	
	(*pixelFormatList)[2] = AGL_DEPTH_SIZE;
	(*pixelFormatList)[3] = 16;
	
	(*pixelFormatList)[4] = AGL_RGBA;
	
	(*pixelFormatList)[5] = AGL_DOUBLEBUFFER;
	
//	(*pixelFormatList)[6] = AGL_OFFSCREEN;
	(*pixelFormatList)[6] = AGL_NONE;
	
	(*pixelFormatList)[7] = AGL_NONE;
}

//
// printPixelFormat
//
static void printPixelFormat( GLint pixelFormatList[PIXEL_FROMAT_LIST_SIZE])
{
	dprintf("attributes of pixel format lists \n");
	dprintf("	renderer ID:     0x%X \n", pixelFormatList[1]);
	dprintf("	z buffer depth:   %d \n", pixelFormatList[3]);
	dprintf("	double buffer:    %d \n", (pixelFormatList[5]==AGL_DOUBLEBUFFER));
	dprintf("	stereo:           %d \n", (pixelFormatList[6]==AGL_STEREO));
	
	dprintf("attributes of pixel format lists in raw format \n");
	dprintf("	0:	 %d \n", pixelFormatList[0]);
	dprintf("	1:	 %d \n", pixelFormatList[1]);
	dprintf("	2:	 %d \n", pixelFormatList[2]);
	dprintf("	3:	 %d \n", pixelFormatList[3]);
	dprintf("	4:	 %d \n", pixelFormatList[4]);
	dprintf("	5:	 %d \n", pixelFormatList[5]);
	dprintf("	6:	 %d \n", pixelFormatList[6]);
	dprintf("	7:	 %d \n", pixelFormatList[7]);
	dprintf("	8:	 %d \n", pixelFormatList[8]);
	dprintf("	9:	 %d \n", pixelFormatList[9]);
	dprintf("	10:	 %d \n", pixelFormatList[10]);
}

//
// isColorARGB
//
static GLboolean isColorARGB( GLint colorModes)
{
	GLboolean isARGB = GL_FALSE;
	
	
	if ((colorModes&AGL_ARGB16161616_BIT) != 0)
	{
		isARGB = GL_TRUE;
	}
	else if ((colorModes&AGL_ARGB12121212_BIT) != 0)
	{
		isARGB = GL_TRUE;
	}
	else if ((colorModes&AGL_RGB101010_A8_BIT) != 0)
	{
		isARGB = GL_TRUE;
	}
	else if ((colorModes&AGL_ARGB2101010_BIT) != 0)
	{
		isARGB = GL_TRUE;
	}
	else if ((colorModes&AGL_RGB888_A8_BIT) != 0)
	{
		isARGB = GL_TRUE;
	}
	else if ((colorModes&AGL_ARGB8888_BIT) != 0)
	{
		isARGB = GL_TRUE;
	}
	else if ((colorModes&AGL_RGB565_A8_BIT) != 0)
	{
		isARGB = GL_TRUE;
	}
	else if ((colorModes&AGL_RGB555_A8_BIT) != 0)
	{
		isARGB = GL_TRUE;
	}
	else if ((colorModes&AGL_ARGB1555_BIT) != 0)
	{
		isARGB = GL_TRUE;
	}
	else if ((colorModes&AGL_RGB444_A8_BIT) != 0)
	{
		isARGB = GL_TRUE;
	}
	else if ((colorModes&AGL_ARGB4444_BIT) != 0)
	{
		isARGB = GL_TRUE;
	}
	else if ((colorModes&AGL_RGB332_A8_BIT) != 0)
	{
		isARGB = GL_TRUE;
	}
	else if ((colorModes&AGL_BGR233_A8_BIT) != 0)
	{
		isARGB = GL_TRUE;
	}
	else if ((colorModes&AGL_RGB8_A8_BIT) != 0)
	{
		isARGB = GL_TRUE;
	}
	
	return isARGB;
}
	
//
// chooseBestColor
//
static GLint chooseBestColor( GLint colorModes)
{
	GLint best = 0;
	
	
	if ((colorModes&AGL_ARGB16161616_BIT) != 0)
	{
		best = AGL_ARGB16161616_BIT;
	}
	else if ((colorModes&AGL_RGB161616_BIT) != 0)
	{
		best = AGL_RGB161616_BIT;
	}
	else if ((colorModes&AGL_ARGB12121212_BIT) != 0)
	{
		best = AGL_ARGB12121212_BIT;
	}
	else if ((colorModes&AGL_RGB121212_BIT) != 0)
	{
		best = AGL_RGB121212_BIT;
	}
	else if ((colorModes&AGL_RGB101010_A8_BIT) != 0)
	{
		best = AGL_RGB101010_A8_BIT;
	}
	else if ((colorModes&AGL_ARGB2101010_BIT) != 0)
	{
		best = AGL_ARGB2101010_BIT;
	}
	else if ((colorModes&AGL_RGB101010_BIT) != 0)
	{
		best = AGL_RGB101010_BIT;
	}
	else if ((colorModes&AGL_RGB888_A8_BIT) != 0)
	{
		best = AGL_RGB888_A8_BIT;
	}
	else if ((colorModes&AGL_ARGB8888_BIT) != 0)
	{
		best = AGL_ARGB8888_BIT;
	}
	else if ((colorModes&AGL_RGB888_BIT) != 0)
	{
		best = AGL_RGB888_BIT;
	}
	else if ((colorModes&AGL_RGB565_A8_BIT) != 0)
	{
		best = AGL_RGB565_A8_BIT;
	}
	else if ((colorModes&AGL_RGB565_BIT) != 0)
	{
		best = AGL_RGB565_BIT;
	}
	else if ((colorModes&AGL_RGB555_A8_BIT) != 0)
	{
		best = AGL_RGB555_A8_BIT;
	}
	else if ((colorModes&AGL_ARGB1555_BIT) != 0)
	{
		best = AGL_ARGB1555_BIT;
	}
	else if ((colorModes&AGL_RGB555_BIT) != 0)
	{
		best = AGL_RGB555_BIT;
	}
	else if ((colorModes&AGL_RGB444_A8_BIT) != 0)
	{
		best = AGL_RGB444_A8_BIT;
	}
	else if ((colorModes&AGL_ARGB4444_BIT) != 0)
	{
		best = AGL_ARGB4444_BIT;
	}
	else if ((colorModes&AGL_RGB444_BIT) != 0)
	{
		best = AGL_RGB444_BIT;
	}
	else if ((colorModes&AGL_RGB332_A8_BIT) != 0)
	{
		best = AGL_RGB332_A8_BIT;
	}
	else if ((colorModes&AGL_RGB332_BIT) != 0)
	{
		best = AGL_RGB332_BIT;
	}
	else if ((colorModes&AGL_BGR233_A8_BIT) != 0)
	{
		best = AGL_BGR233_A8_BIT;
	}
	else if ((colorModes&AGL_BGR233_BIT) != 0)
	{
		best = AGL_BGR233_BIT;
	}
	else if ((colorModes&AGL_RGB8_A8_BIT) != 0)
	{
		best = AGL_RGB8_A8_BIT;
	}
	else if ((colorModes&AGL_RGB8_BIT) != 0)
	{
		best = AGL_RGB8_BIT;
	}
/*
	else if ((colorModes&AGL_INDEX16_BIT) != 0)
	{
		best = AGL_INDEX16_BIT;
	}
	else if ((colorModes&AGL_INDEX8_BIT) != 0)
	{
		best = AGL_INDEX8_BIT;
	}
	
*/
	return best;
}

//
// chooseBestDepth
//
static GLint chooseBestDepth( GLint depthModes)
{
	GLint best = 0;
	
	
	if ((depthModes&AGL_128_BIT) != 0)
	{
		best = AGL_128_BIT;
	}
	else if ((depthModes&AGL_96_BIT) != 0)
	{
		best = AGL_96_BIT;
	}
	else if ((depthModes&AGL_64_BIT) != 0)
	{
		best = AGL_64_BIT;
	}
	else if ((depthModes&AGL_48_BIT) != 0)
	{
		best = AGL_48_BIT;
	}
	else if ((depthModes&AGL_32_BIT) != 0)
	{
		best = AGL_32_BIT;
	}
	else if ((depthModes&AGL_24_BIT) != 0)
	{
		best = AGL_24_BIT;
	}
	else if ((depthModes&AGL_16_BIT) != 0)
	{
		best = AGL_16_BIT;
	}
	else if ((depthModes&AGL_12_BIT) != 0)
	{
		best = AGL_12_BIT;
	}
	else if ((depthModes&AGL_10_BIT) != 0)
	{
		best = AGL_10_BIT;
	}
	else if ((depthModes&AGL_8_BIT) != 0)
	{
		best = AGL_8_BIT;
	}
	else if ((depthModes&AGL_6_BIT) != 0)
	{
		best = AGL_6_BIT;
	}
	else if ((depthModes&AGL_5_BIT) != 0)
	{
		best = AGL_5_BIT;
	}
	else if ((depthModes&AGL_4_BIT) != 0)
	{
		best = AGL_4_BIT;
	}
	else if ((depthModes&AGL_3_BIT) != 0)
	{
		best = AGL_3_BIT;
	}
	else if ((depthModes&AGL_2_BIT) != 0)
	{
		best = AGL_2_BIT;
	}
	else if ((depthModes&AGL_1_BIT) != 0)
	{
		best = AGL_1_BIT;
	}
	else if ((depthModes&AGL_0_BIT) != 0)
	{
		best = AGL_0_BIT;
	}
		
	return best;
}

//
// mapDepth
//
static GLint mapDepth( GLint depthMode)
{
	GLint depth = 0;
	
	
	if (depthMode == AGL_128_BIT)
	{
		depth = 128;
	}
	else if (depthMode == AGL_96_BIT)
	{
		depth = 96;
	}
	else if (depthMode == AGL_64_BIT)
	{
		depth = 64;
	}
	else if (depthMode == AGL_48_BIT)
	{
		depth = 48;
	}
	else if (depthMode == AGL_32_BIT)
	{
		depth = 32;
	}
	else if (depthMode == AGL_24_BIT)
	{
		depth = 24;
	}
	else if (depthMode == AGL_16_BIT)
	{
		depth = 16;
	}
	else if (depthMode == AGL_12_BIT)
	{
		depth = 12;
	}
	else if (depthMode == AGL_10_BIT)
	{
		depth = 10;
	}
	else if (depthMode == AGL_8_BIT)
	{
		depth = 8;
	}
	else if (depthMode == AGL_6_BIT)
	{
		depth = 6;
	}
	else if (depthMode == AGL_5_BIT)
	{
		depth = 5;
	}
	else if (depthMode == AGL_4_BIT)
	{
		depth = 4;
	}
	else if (depthMode == AGL_3_BIT)
	{
		depth = 3;
	}
	else if (depthMode == AGL_2_BIT)
	{
		depth = 2;
	}
	else if (depthMode == AGL_1_BIT)
	{
		depth = 1;
	}
	else if (depthMode == AGL_0_BIT)
	{
		depth = 0;
	}
	
	return depth;
}

//
// getRenderersInfo
//
static void getRenderersInfo( RenderersInfoRectPtr info)
{
	UInt32			count			= 0;
	AGLRendererInfo renderInfo		= nil,
					renderInfoHead	= nil;
	GDHandle		device			= GetMainDevice();
	GLint			returnedInfo	= 0;
	//THz				saveZone		= GetZone();
	
	
	count = 0;
	renderInfoHead = aglQueryRendererInfo(&device, 1);
	renderInfo = renderInfoHead;
 	while (renderInfo != nil)
 	{
	    renderInfo = aglNextRendererInfo(renderInfo);	    
 		count++;
 	}
 	aglDestroyRendererInfo(renderInfoHead);
 	
 	info->count = count;
	//SetZone(SystemZone());
 	info->info = (RendererRectPtr)NewPtrClear(info->count * sizeof(RendererRect));
	//SetZone(saveZone);
 	
	count = 0;
	renderInfoHead = aglQueryRendererInfo(&device, 1);
	renderInfo = renderInfoHead;
 	while (renderInfo != nil)
 	{
 		info->info[count].valid = GL_TRUE;
 		
		aglDescribeRenderer(renderInfo, AGL_RENDERER_ID, &returnedInfo);
		info->info[count].id = returnedInfo;
				
		aglDescribeRenderer(renderInfo, AGL_ACCELERATED, &returnedInfo);
		info->info[count].accelerated = (GLboolean)returnedInfo;
		
		aglDescribeRenderer(renderInfo, AGL_BUFFER_MODES, &returnedInfo);
		info->info[count].singleBuffer	= (GLboolean)((returnedInfo&AGL_SINGLEBUFFER_BIT)!=0);
		info->info[count].doubleBuffer	= (GLboolean)((returnedInfo&AGL_DOUBLEBUFFER_BIT)!=0);
		info->info[count].mono			= (GLboolean)((returnedInfo&AGL_MONOSCOPIC_BIT)!=0);
		info->info[count].stereo		= (GLboolean)((returnedInfo&AGL_STEREOSCOPIC_BIT)!=0);
		
		aglDescribeRenderer(renderInfo, AGL_COLOR_MODES, &returnedInfo);
		info->info[count].colorModes = returnedInfo;
		
		aglDescribeRenderer(renderInfo, AGL_DEPTH_MODES, &returnedInfo);
		info->info[count].depthModes = returnedInfo;
		
		aglDescribeRenderer(renderInfo, AGL_STENCIL_MODES, &returnedInfo);
		info->info[count].stencilModes = returnedInfo;		
		
		renderInfo = aglNextRendererInfo(renderInfo);
		count++;
 	}
 	aglDestroyRendererInfo(renderInfoHead);
}

//
// printRendererInfo
//
static void printRendererInfo( RendererRect info)
{
	dprintf("renderer info \n");
	dprintf("	valid:         %d \n", info.valid);
	dprintf("	ID:           0x%X \n", info.id);
	dprintf("	accelerated:   %d \n", info.accelerated);
	dprintf("	depthModes:    %d \n", info.depthModes);
	dprintf("	colorModes:    %d \n", info.colorModes);
	dprintf("	stencilModes:  %d \n", info.stencilModes);
	dprintf("	singleBuffer:  %d \n", info.singleBuffer);
	dprintf("	doubleBuffer:  %d \n", info.doubleBuffer);
	dprintf("	mono:          %d \n", info.mono);
	dprintf("	stereo:        %d \n", info.stereo);
}

//
// chooseBestRenderer
//
static void chooseBestRenderer( RenderersInfoRectPtr info, GLboolean *doubleBuffer, GLboolean *stereo)
{
	UInt32 		count,
				listIndex					= 0;
	GLboolean	haveAcceleratedRenderer		= GL_FALSE,
				haveValidRenderer			= GL_FALSE,
				chooseNewPixelFormat		= GL_FALSE;
	
	
	for (count=0; count<(info->count); count++)
	{
		info->info[count].valid = info->info[count].valid && (isColorARGB( info->info[count].colorModes) == GL_TRUE);
		
		if (*doubleBuffer == GL_TRUE)
		{
			info->info[count].valid = info->info[count].valid && (info->info[count].doubleBuffer == GL_TRUE);
		}
		
		if (*stereo == GL_TRUE)
		{
			//info->info[count].valid = info->info[count].valid && (info->info[count].stereo == GL_TRUE);
			// there are no stereoscopic OpenGL renderers for mac avaialable yet
			// no reason to reject a (possibly hardware accelerated) renderer just because of that
		}
		
		haveAcceleratedRenderer = haveAcceleratedRenderer || (info->info[count].accelerated == GL_TRUE);
		haveValidRenderer = haveValidRenderer || (info->info[count].valid == GL_TRUE);
	}
	
	// first look for an accelerated renderer
	if (chooseNewPixelFormat == GL_FALSE)
	{
		count = 0;
		while (count < info->count)
		{
			if ((info->info[count].valid == GL_TRUE) && (info->info[count].accelerated == GL_TRUE))
			{
				chooseNewPixelFormat = GL_TRUE;
								
				break;
			}
			
			count++;
		}
	}
	
	// now look for any renderer
	if (chooseNewPixelFormat == GL_FALSE)
	{
		count = 0;
		while (count < info->count)
		{
			if (info->info[count].valid == GL_TRUE)
			{
				chooseNewPixelFormat = GL_TRUE;
				
				break;
			}
			
			count++;
		}
	}
	
	if (chooseNewPixelFormat == GL_TRUE)
	{
		info->attributeList[0] = AGL_RENDERER_ID;
		info->attributeList[1] = info->info[count].id;
		
		info->attributeList[2] = AGL_DEPTH_SIZE;
		info->attributeList[3] = mapDepth( chooseBestDepth( info->info[count].depthModes));
		
		info->attributeList[4] = AGL_RGBA;
		
		listIndex = 5;
		
		if ((*doubleBuffer == GL_TRUE) && (info->info[count].doubleBuffer == GL_TRUE))
		{
			info->attributeList[listIndex++] = AGL_DOUBLEBUFFER;
		}
		else
		{
			*doubleBuffer = GL_FALSE;
		}
		
		if ((*stereo == GL_TRUE) && (info->info[count].stereo == GL_TRUE))
		{
			info->attributeList[listIndex++] = AGL_STEREO;
		}
		else
		{
			*stereo = GL_FALSE;
		}
		
//		info->attributeList[listIndex++] = AGL_OFFSCREEN;
		info->attributeList[listIndex++] = AGL_NONE;
	}
	
	DisposePtr((Ptr)info->info);
	info->info = nil;
}

#pragma mark -

//
// printBufferModes
//
static void printBufferModes( GLint v)
{
	if(v & AGL_MONOSCOPIC_BIT)   dprintf("            AGL_MONOSCOPIC_BIT \n");
	if(v & AGL_STEREOSCOPIC_BIT) dprintf("            AGL_STEREOSCOPIC_BIT \n");
	if(v & AGL_SINGLEBUFFER_BIT) dprintf("            AGL_SINGLEBUFFER_BIT \n");
	if(v & AGL_DOUBLEBUFFER_BIT) dprintf("            AGL_DOUBLEBUFFER_BIT \n");
}

//
// printColorModes
//
static void printColorModes( GLint v)
{
	if(v & AGL_RGB8_BIT)         dprintf("            AGL_RGB8_BIT \n");
	if(v & AGL_RGB8_A8_BIT)      dprintf("            AGL_RGB8_A8_BIT \n");
	if(v & AGL_BGR233_BIT)       dprintf("            AGL_BGR233_BIT \n");
	if(v & AGL_BGR233_A8_BIT)    dprintf("            AGL_BGR233_A8_BIT \n");
	if(v & AGL_RGB332_BIT)       dprintf("            AGL_RGB332_BIT \n");
	if(v & AGL_RGB332_A8_BIT)    dprintf("            AGL_RGB332_A8_BIT \n");
	if(v & AGL_RGB444_BIT)       dprintf("            AGL_RGB444_BIT \n");
	if(v & AGL_ARGB4444_BIT)     dprintf("            AGL_ARGB4444_BIT \n");
	if(v & AGL_RGB444_A8_BIT)    dprintf("            AGL_RGB444_A8_BIT \n");
	if(v & AGL_RGB555_BIT)       dprintf("            AGL_RGB555_BIT \n");
	if(v & AGL_ARGB1555_BIT)     dprintf("            AGL_ARGB1555_BIT \n");
	if(v & AGL_RGB555_A8_BIT)    dprintf("            AGL_RGB555_A8_BIT \n");
	if(v & AGL_RGB565_BIT)       dprintf("            AGL_RGB565_BIT \n");
	if(v & AGL_RGB565_A8_BIT)    dprintf("            AGL_RGB565_A8_BIT \n");
	if(v & AGL_RGB888_BIT)       dprintf("            AGL_RGB888_BIT \n");
	if(v & AGL_ARGB8888_BIT)     dprintf("            AGL_ARGB8888_BIT \n");
	if(v & AGL_RGB888_A8_BIT)    dprintf("            AGL_RGB888_A8_BIT \n");
	if(v & AGL_RGB101010_BIT)    dprintf("            AGL_RGB101010_BIT \n");
	if(v & AGL_ARGB2101010_BIT)  dprintf("            AGL_ARGB2101010_BIT \n");
	if(v & AGL_RGB101010_A8_BIT) dprintf("            AGL_RGB101010_A8_BIT \n");
	if(v & AGL_RGB121212_BIT)    dprintf("            AGL_RGB121212_BIT \n");
	if(v & AGL_ARGB12121212_BIT) dprintf("            AGL_ARGB12121212_BIT \n");
	if(v & AGL_RGB161616_BIT)    dprintf("            AGL_RGB161616_BIT \n");
	if(v & AGL_ARGB16161616_BIT) dprintf("            AGL_ARGB16161616_BIT \n");
	if(v & AGL_INDEX8_BIT)       dprintf("            AGL_INDEX8_BIT \n");
	if(v & AGL_INDEX16_BIT)      dprintf("            AGL_INDEX16_BIT \n");
}

//
// printBitModes
//
static void printBitModes( GLint v)
{
	if(v & AGL_0_BIT)            dprintf("            AGL_0_BIT \n");
	if(v & AGL_1_BIT)            dprintf("            AGL_1_BIT \n");
	if(v & AGL_2_BIT)            dprintf("            AGL_2_BIT \n");
	if(v & AGL_3_BIT)            dprintf("            AGL_3_BIT \n");
	if(v & AGL_4_BIT)            dprintf("            AGL_4_BIT \n");
	if(v & AGL_5_BIT)            dprintf("            AGL_5_BIT \n");
	if(v & AGL_6_BIT)            dprintf("            AGL_6_BIT \n");
	if(v & AGL_8_BIT)            dprintf("            AGL_8_BIT \n");
	if(v & AGL_10_BIT)           dprintf("            AGL_10_BIT \n");
	if(v & AGL_12_BIT)           dprintf("            AGL_12_BIT \n");
	if(v & AGL_16_BIT)           dprintf("            AGL_16_BIT \n");
	if(v & AGL_24_BIT)           dprintf("            AGL_24_BIT \n");
	if(v & AGL_32_BIT)           dprintf("            AGL_32_BIT \n");
	if(v & AGL_48_BIT)           dprintf("            AGL_48_BIT \n");
	if(v & AGL_64_BIT)           dprintf("            AGL_64_BIT \n");
	if(v & AGL_96_BIT)           dprintf("            AGL_96_BIT \n");
	if(v & AGL_128_BIT)          dprintf("            AGL_128_BIT \n");
}

//
// printInfoStats
//
static void printInfoStats( AGLRendererInfo info)
{
	GLint rv;
	
	
	aglDescribeRenderer(info, AGL_RENDERER_ID, &rv);
	dprintf("        AGL_RENDERER_ID     : 0x%X \n", rv);
	
	aglDescribeRenderer(info, AGL_OFFSCREEN, &rv);
	dprintf("        AGL_OFFSCREEN       : %s \n", (rv ? "GL_TRUE" : "GL_FALSE"));
	
	aglDescribeRenderer(info, AGL_FULLSCREEN, &rv);
	dprintf("        AGL_FULLSCREEN      : %s \n", (rv ? "GL_TRUE" : "GL_FALSE"));
	
	aglDescribeRenderer(info, AGL_WINDOW, &rv);
	dprintf("        AGL_WINDOW          : %s \n", (rv ? "GL_TRUE" : "GL_FALSE"));
	
	aglDescribeRenderer(info, AGL_ACCELERATED, &rv);
	dprintf("        AGL_ACCELERATED     : %s \n", (rv ? "GL_TRUE" : "GL_FALSE"));
	
	aglDescribeRenderer(info, AGL_ROBUST, &rv);
	dprintf("        AGL_ROBUST          : %s \n", (rv ? "GL_TRUE" : "GL_FALSE"));
	
	aglDescribeRenderer(info, AGL_BACKING_STORE, &rv);
	dprintf("        AGL_BACKING_STORE   : %s \n", (rv ? "GL_TRUE" : "GL_FALSE"));
	
	aglDescribeRenderer(info, AGL_MP_SAFE, &rv);
	dprintf("        AGL_MP_SAFE         : %s \n", (rv ? "GL_TRUE" : "GL_FALSE"));
	
	aglDescribeRenderer(info, AGL_COMPLIANT, &rv);
	dprintf("        AGL_COMPLIANT       : %s \n", (rv ? "GL_TRUE" : "GL_FALSE"));
	
	aglDescribeRenderer(info, AGL_MULTISCREEN, &rv);
	dprintf("        AGL_MULTISCREEN     : %s \n", (rv ? "GL_TRUE" : "GL_FALSE"));
	
	aglDescribeRenderer(info, AGL_BUFFER_MODES, &rv);
	dprintf("        AGL_BUFFER_MODES    : 0x%X \n", rv);
	printBufferModes( rv);
	
	aglDescribeRenderer(info, AGL_MIN_LEVEL, &rv);
	dprintf("        AGL_MIN_LEVEL       :  %d \n", rv);
	
	aglDescribeRenderer(info, AGL_MAX_LEVEL, &rv);
	dprintf("        AGL_MAX_LEVEL       :  %d \n", rv);
	
	aglDescribeRenderer(info, AGL_COLOR_MODES, &rv);
	dprintf("        AGL_COLOR_MODES     : 0x%X \n", rv);
	printColorModes( rv);
	
	aglDescribeRenderer(info, AGL_ACCUM_MODES, &rv);
	dprintf("        AGL_ACCUM_MODES     : 0x%X \n", rv);
	printColorModes( rv);
	
	aglDescribeRenderer(info, AGL_DEPTH_MODES, &rv);
	dprintf("        AGL_DEPTH_MODES     : 0x%X \n", rv);
	printBitModes( rv);
	
	aglDescribeRenderer(info, AGL_STENCIL_MODES, &rv);
	dprintf("        AGL_STENCIL_MODES   : 0x%X \n", rv);
	printBitModes( rv);
	
	aglDescribeRenderer(info, AGL_MAX_AUX_BUFFERS, &rv);
	dprintf("        AGL_MAX_AUX_BUFFERS :  %d \n", rv);
	
	aglDescribeRenderer(info, AGL_VIDEO_MEMORY, &rv);
	dprintf("        AGL_VIDEO_MEMORY    :  %d \n", rv);
	
	aglDescribeRenderer(info, AGL_TEXTURE_MEMORY, &rv);
	dprintf("        AGL_TEXTURE_MEMORY  :  %d \n", rv);
}

//
// checkGetRendererInfo
//
static void checkGetRendererInfo( GDHandle device)
{
	AGLRendererInfo	info,
					infoSaved;
	GLint			inum;
	
	
	info =  aglQueryRendererInfo(&device, 1);
	if (info == nil)
	{
		dprintf("aglQueryRendererInfo : Info Error \n");
		return;
	}
	
	inum = 0;
	while (info != nil)
	{
		dprintf("    Renderer :  %d \n", inum);
		printInfoStats( info);
		infoSaved = info;
		info = aglNextRendererInfo(infoSaved);
		aglDestroyRendererInfo(infoSaved);
		inum++;
	}
}

//
// printAllRenderer
//
static void printAllRenderer()
{
	GLenum   err;
	GDHandle device;
	GLuint   dnum = 0;
	
	
	dprintf("the renderers that are available on this system: \n");
	
	err = aglGetError();
	if (err != AGL_NO_ERROR)
	{
		dprintf("aglGetError 1 - %s \n", aglErrorString(err));
	}
	
	device = GetDeviceList();
	while (device != nil)
	{
		dprintf("Device :  %d \n", dnum);
		checkGetRendererInfo( device);
		device = GetNextDevice(device);
		dnum++;
	}
		
	err = aglGetError();
	if (err != AGL_NO_ERROR)
	{
		dprintf("aglGetError 2 - %s \n", aglErrorString(err));
	}
}