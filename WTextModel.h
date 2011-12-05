//=================================================================
// ¥ WTextModel
// © Timothy Paustian All Rights Reserved 1997
//	 Apple Event Text Model
// <mailto:paustian@bact.wisc.edu>
// <http://www.bact.wisc.edu/WText/overview.html>
//=================================================================
//
// Purpose:	 An implementation of the text model for apple events
// this is the base class that represents the document. Paragraphs
// lines, words and characters are subclassses of this class. Each of 
// these objects is created on the fly during the apple event request.
// It does not make sense to keep model objects around for them during
// non use.
//
// All of the utility routines for finding words, chars, lines etc.
// are kept here since many model objects in a text hierchy might want
// to use them.
//
// subclasses of WTextModel just add modifying routines to filter the
// call and adapt them to paragraphs, lines, words and chars.
//
// Revision: 3/9/98 initial creation
//
//=================================================================

#ifndef	_H_WTextModel
#define	_H_WTextModel

#pragma once

#if PP_Uses_Pragma_Import
	#pragma import on
#endif

#include <LModelObject.h>

enum {
	kAEDefaultJustified			= FOUR_CHAR_CODE('DEFT')
};

//we need this for the TestStyle struct
#ifndef __TEXTEDIT__
#include <TextEdit.h>
#endif

//Needed for the WERuler struct def
#ifndef __WASTE__
#include "waste.h"
#endif

class WSelectionText;
class WCharCounter;
class WWordCounter;
class WParaCounter;
class WTextView;

class WTextModel : public LModelObject { 
//don't do any of these you weenie.
private:
							WTextModel();

							WTextModel(
								const WTextModel & inOriginal);													

		WTextModel& 	operator=(
								const WTextModel & inRhs);
public:
							 

								
							WTextModel(
									LModelObject	*inSuperModel,
									SInt32			inPosition);
							
							WTextModel(
								LModelObject *inSuperModel,
								WTextView		*	inClient);
	

	
	virtual 				~WTextModel();
	
																		
	virtual void		HandleAppleEvent(
										const AppleEvent	&inAppleEvent,
										AppleEvent			&outAEReply,
										AEDesc				&outResult,
										long				inAENumber);
	
	virtual void		HandleGetData(	
										const AppleEvent	&inAppleEvent,
										AEDesc				&outResult,
										long				inAENumber);
										
	virtual void		HandleSetData(	
										const AppleEvent	&inAppleEvent,
										AppleEvent			&outAEReply);
										
	virtual void		HandleSelect();
	
	virtual void		HandleDelete(
								AppleEvent&	/* outAEReply */,
								AEDesc&		/* outResult */);
	
	virtual SInt32		CountSubModels(
								DescType	inModelID) const;
	virtual SInt32		CountChars() const;
	
	virtual SInt32		CountWords() const;
	
	virtual SInt32		CountLines() const;
	
	virtual SInt32		CountParagraphs() const;
	
	virtual SInt32		GetPositionOfSubModel(	
												DescType			inModelID,
												const LModelObject	*inSubModel) const;
	
	virtual LModelObject*	GetModelProperty(
										DescType inProperty) const;
										
	virtual void		GetModelToken(	
								DescType		inModelID,
								DescType		inKeyForm,
								const AEDesc	&inKeyData,
								AEDesc			&outToken) const;
							
	virtual void		GetSubModelByPosition(
											DescType		inModelID,
											SInt32			inPosition,
											AEDesc			&outToken) const;
											
	
	virtual void		GetSubModelByComplexKey(
											DescType inModelID, DescType inKeyForm,
											const AEDesc&	 inKeyData, AEDesc&	outToken) const;
											
	virtual void		GetRangeInfo(	
										const AEDesc inKeyData, SInt32 *outStart, 
										DescType *outStopType, SInt32 *outStop) const;
	
	virtual void		GetAllSubModels(
										DescType inModelID, 
										AEDesc &outToken) const;										
	
	virtual SInt32		GetPosition() const {return mModelPosition;} 
	
	virtual void		SetPosition(SInt32 inPosition) {mModelPosition = inPosition;}
	
	virtual WTextView * GetTextView() {return mTE;}
	
	virtual SInt8		FindNthChar(
								Handle outChar, 
								SInt32 & ioPosition) const;

	virtual void		FindNthWord(
								Handle outWordH, 
								SInt32 inNumber, 
								SInt32 & outStart, 
								SInt32  & outEnd) const;
							
	virtual void		FindNthLine(
								Handle outLineH, 
								SInt32 inNumber, 
								SInt32 & outStart, 
								SInt32 & outEnd) const;

	virtual	void		FindNthParagraph(
								Handle outParaH, 
								SInt32 inNumber, 
								SInt32 & outStart, 
								SInt32 & outEnd) const;
									
	virtual void		SetSearchPointers(
									SInt32	start, 
									SInt32 	end);
									
	virtual void		GetSearchPointers
									(SInt32	& start, 
									SInt32	& end) const;
	
	virtual void		DoSetStyle(
									AEDesc inValue);
	
	virtual void		CreateStyle(
								short 				inMode, 
								TextStyle	  &	inStyle,
								AppleEvent	&		outStyleEvent);
								
#if WASTE_VERSION > 0x02000000
	virtual void		CreateParaStyle(
								WERulerMode 				inMode, 
								WERuler  &		inStyle,
								AppleEvent	&		outStyleEvent);
								
	virtual void		SetParaStyle(
										AEDesc	inValue);
	
	
#endif
	
	virtual void		MakeRangeSpecifier(
								AEDesc	&inSuperSpecifier,
								AEDesc	&outSelfSpecifier) const;
	
	virtual bool		CanUseParagraphs(
									SInt32 & ioStart, 
									SInt32  & ioEnd) const;
							
	virtual bool		CanUseWords(
									SInt32 & ioStart, 
									SInt32 & ioEnd) const;
	
	virtual void		MakeInsertSpecifier(
									AEDesc	&inSuperSpecifier,
									AEDesc	&outSelfSpecifier) const;
	
	virtual void		SendEventToSelf(
								SInt32			suite, 
								SInt32			event,
								AppleEvent	&	outAppleEvent,
								bool				specifyThis = true);
	
	virtual void		SelectAll();
	
	virtual void		MakeSelfSpecifier(	
								AEDesc	&inSuperSpecifier,
								AEDesc	&outSelfSpecifier) const;
											
	virtual void		GetAEProperty(	
								DescType		inProperty,
								const AEDesc	&inRequestedType,
								AEDesc			&outPropertyDesc) const;

	virtual void		SetAEProperty(
								DescType inProperty,
							 	const AEDesc &inValue,
							 	AEDesc& outAEReply);
	
	virtual bool	AEPropertyExists(
							DescType	inProperty) const;
	
	virtual void		DoContinuousStyles(
								SInt32 selStart, 
								SInt32 selEnd, 
								AEDesc *outDesc) const;
	
	virtual void		BuildStyleAEDesc(
								AEDesc *ioStyleDesc, 
								Style inStyle) const;
							
	virtual void		SetStyle(
								WEStyleMode		inMode,
								TextStyle	inStyle);
								
	virtual void		FindTheTextForModel(
								Handle		theTextHandle, 
								SInt32	&	outStart, 
								SInt32 	&	outEnd) const;
	

	
	virtual WCharCounter*	GetCharLoop() {return mCharLooper;}
	
	virtual WWordCounter*	GetWordLoop() {return mWordLooper;}
	
	virtual WParaCounter*	GetParaLoop() {return mParaLooper;}
	
	virtual WSelectionText*	GetSelectionObj(){return mSelection;}
	
private:
	virtual void		AddInsertionLocation(
								AppleEvent		& outAppleEvent);
protected:
	
	WTextView			*	mTE;			//a pointer to the text engine
	WSelectionText		*	mSelection; //a pointer to the selection object
	SInt32				mStart;			//the start of the current text model object
	SInt32				mEnd;				//the end of the current text model object
	//SInt32					mInsertStart; //when inserting text, the start of selection before insert
	//SInt32					mInsertEnd;	  //when inserting text, the end of selection before insert
	WCharCounter		*	mCharLooper; //a class for counting and finding chars
	WWordCounter		*	mWordLooper; //a class for counting and finding words
	WParaCounter		*	mParaLooper; //a class for counting and finding paragraphs
	SInt32					mModelPosition;   //the current requested position
	SInt32					mStartPosition;
	SInt32					mEndPosition;
};



#if PP_Uses_Pragma_Import
	#pragma import reset
#endif
#endif
