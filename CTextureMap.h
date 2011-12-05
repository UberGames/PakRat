/* 
	CTextureMap.h

	Author:			Tom Naughton
	Description:	<describe the CTextureMap class here>
*/

#ifndef CTextureMap_H
#define CTextureMap_H


class CTextureMap
{
public:
	CTextureMap();
	virtual ~CTextureMap();

	void	BeginMap();
	void	BeginTriangle();
	void	EndTriangle();
	Boolean	NextSize(UInt32 width, UInt32 height);
	void	AddVertex(float x, float y);
	void	EndMap();
	
	PicHandle	picture()  { return _pic; };
	Rect		bounds() { return _bounds; };

protected:

	void 		StartClip();
	void 		EndClip();

	PicHandle 	_pic;
	Rect 		_bounds;
	UInt32 		_vertind;
	UInt32		_size;
	float 		_firstX, _firstY;
	RgnHandle 	_theCurrentClipRgn;
};

#endif	// CTextureMap_H
