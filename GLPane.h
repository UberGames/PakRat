#pragma once

#include <LPane.h>
#include <agl.h>

class	GLPane : public LPane {
  public:
	enum { class_ID = 'OpGL' };

	GLPane(const SPaneInfo&	inPaneInfo,  LStr255& desc);
	GLPane(LStream*		inStream);
	virtual			~GLPane();

	virtual void	ClickSelf(const SMouseDownEvent &);
	virtual void	ResizeFrameBy(SInt16		inWidthDelta,
								  SInt16		inHeightDelta,
								  Boolean		inRefresh);
	virtual void	MoveBy(SInt32		inHorizDelta,
						   SInt32		inVertDelta,
						   Boolean		inRefresh);
	virtual void	ShowSelf(void);
	void			SwapBuffers();	// swap buffers in double buffer mode
	void			Redraw();		// draw immediatly the panel
	virtual void	ReshapeGL(int,int);		// change size

	virtual Boolean		FocusDraw(LPane* inSubPane = nil);


 protected:
 
	GLPane();

	// Overridable OpenGL methods
	virtual void			DrawGL();				// GL Rendering
	virtual void			MouseDown(const SMouseDownEvent&	inMouseDown, const Point&);// Mouse down event
	virtual void			MouseDrag(const SMouseDownEvent&	inMouseDown, const Point&);// mouse is being dragged

	virtual	void			SetupAGL(unsigned char* attributes);
	virtual	void			DrawSelf(); 			//  Don't override this function

	AGLContext				mCtx;
	int						_width, _height;

	void			InitGL(void);			// Init the pane for the first time (called during constructor)

};


