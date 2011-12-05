/* 
	CAnimationQueue.h

	Author:			Tom Naughton
	Description:	<describe the CAnimationQueue class here>
*/

#ifndef CAnimationQueue_H
#define CAnimationQueue_H

#define ANIMATION_QUEUE_SIZE 1024
#define ANIMATION_MAX_LOST_FRAMES 25

typedef struct MDLFRAMEINFO
{
} mdl_frame_info;


class CAnimationQueue
{
public:
	CAnimationQueue();
	virtual ~CAnimationQueue();
	
	void		PushSequence(int start, int count);
	void		PopFrames(int count);
	SInt16		PeekFrame(int index);

	SInt16 currentFrame, nextFrame;
	float interpolation;
	
	float startTime;
	SInt16 queueLength;
	SInt16 queueCursor;
	SInt16 queue[ANIMATION_QUEUE_SIZE];
};

#endif	// CAnimationQueue_H
