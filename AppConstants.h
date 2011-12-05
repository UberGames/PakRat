// =================================================================================
//	AppConstants.h				©1995-1999 Metrowerks Inc. All rights reserved.
// =================================================================================

#ifndef _H_AppConstants
#define _H_AppConstants
#pragma once

#include <PP_Types.h>

const ResIDT		PPob_PictureWindow				= 20025;
const ResIDT		PPob_MemoryWindow				= 130;
const ResIDT		PPob_ResourceWindow				= 128;
const ResIDT		PPob_GLWindow					= 129;
const ResIDT		PPob_ModelWindow				= 131;
const ResIDT		PPob_MapWindow					= 132;
const ResIDT		PPob_SoundWindow				= 133;
const ResIDT		PPob_TextWindow					= 134;
const ResIDT		PPob_CompileWindow				= 135;
const ResIDT		PPob_ShaderWindow				= 136;

const ResIDT		PPob_RenderingView				= 1000;
const ResIDT		PPob_PositionView				= 1001;
const ResIDT		PPob_AnimationView				= 1002;

const PaneIDT		kGLView							= 200;
const PaneIDT		kGWorldView						= 'Gwld';
const PaneIDT		kTextScroller					= 1;
const PaneIDT		kTextEditView					= 200;
const PaneIDT		kMemoryTable					= 2;
const PaneIDT		kResourceTable					= 2;
const PaneIDT		kModelView						= 201;
const PaneIDT		kMapView						= 203;
const PaneIDT		kShaderView						= 204;


// control window panes

const PaneIDT		kRenderModePopup					= 2000;
const PaneIDT		kRotXSlider							= 2001;
const PaneIDT		kRotYSlider							= 2002;
const PaneIDT		kRotZSlider							= 2003;
const PaneIDT		kRotXField							= 204;
const PaneIDT		kRotYField							= 205;
const PaneIDT		kRotZField							= 206;

const PaneIDT		kTransXSlider						= 2010;
const PaneIDT		kTransYSlider						= 2011;
const PaneIDT		kTransZSlider						= 2012;
const PaneIDT		kTransXField						= 213;
const PaneIDT		kTransYField						= 214;
const PaneIDT		kTransZField						= 215;


const PaneIDT		kMemoryIndicator					= 2100;
const PaneIDT		kMemoryField						= 2101;


const MessageT		msg_RenderModeChanged					= 3000;	
const MessageT		msg_ControllerChanged					= 3001;	

const PaneIDT		kRenderingView							= 301;
const PaneIDT		kPositionView							= 302;
const PaneIDT		kAnimationView							= 303;

const PaneIDT		item_ControlMainPanelView		= 'MPV3';
const PaneIDT		item_ControlPageController		= 'PCTL';

const ResIDT		PPob_TextPrintout				= 1100;
const PaneIDT		kTextPlaceholder				= 1;

const ResIDT		STRx_DefaultDocumentTitle		= 1000;
const SInt16		DefaultDocumentTitle_Untitled	= 1;
const SInt16		DefaultDocumentTitle_UntitledX	= 2;

const OSType		kApplicationCreator				= FOUR_CHAR_CODE('PkRt');

const ResIDT		styl_DocumentInfo				= 128;

// compile window
const PaneIDT		kCompileButton							= 301;
const PaneIDT		kShaderPopup							= 302;


// alerts

#define ALRT_NoDragAndDrop	2210
#define ALRT_NoOpenGL		2211

// some constants

const ResIDT	PPob_TableWindow		= 1;
const ResIDT	PPob_SelectVolDialog	= 128;
const PaneIDT	VolDlg_Popup			= 'VOLp';
const ResIDT	MENU_Volume				= 1000;
const ResIDT	menu_window				= 136;
const ResIDT	menu_recentfiles		= 137;	
const ResIDT	menu_shader				= 138;	
const ResIDT	menu_animation			= 139;	

const ResIDT	PPob_EditTable			= 136;

// icons 
const ResIDT	icon_Text			= 133;
const ResIDT	icon_Model			= 142; // 128;
const ResIDT	icon_Shader			= 137; // 132;
const ResIDT	icon_Skin			= 134;
const ResIDT	icon_Sound			= 138; // 135;
const ResIDT	icon_JPEG			= 131;
const ResIDT	icon_TARGA			= 139;
const ResIDT	icon_Map			= 143; 

// test

const ResIDT	TEXT_Preferences	= 1001;
const ResIDT	TEXT_Animation		= 1002;

// about box

#define		TEXT_AboutVersion	607	
#define		TEXT_Credits		1000

#define 	WIND_MetroAbout 	1000
#define 	PICT_Background 	3000


// menu commands

const MessageT	cmd_lighting			= 3001;	
const MessageT	cmd_wireframe			= 3002;	
const MessageT	cmd_texture				= 3003;	
const MessageT	cmd_interpolation		= 3004;	
const MessageT	cmd_shownormals			= 3005;	
const MessageT	cmd_showbox				= 3006;	
const MessageT	cmd_showinfo			= 3007;	
const MessageT	cmd_repeatanimations	= 3008;	
const MessageT	cmd_assemblemodels		= 3009;	
const MessageT	cmd_playsounds			= 3010;	
const MessageT	cmd_previousskin		= 3011;	
const MessageT	cmd_nextskin			= 3012;	

const MessageT	cmd_backgroundgrey			= 3013;	
const MessageT	cmd_backgroundblack			= 3014;	
const MessageT	cmd_backgroundwhite			= 3015;	
const MessageT	cmd_backgroundchoose		= 3016;	

const MessageT	cmd_paletteQuake			= 3017;	
const MessageT	cmd_paletteHexen			= 3018;	

const MessageT	cmd_backgroundcolormenu		= 3019;	
const MessageT	cmd_palettemenu				= 3020;	
const MessageT	cmd_exportmenu				= 3021;	

const MessageT	cmd_exportobj				= 3022;	
const MessageT	cmd_exportdxf				= 3023;	
const MessageT	cmd_openDocumentPath		= 3024;	

const MessageT	cmd_recentmenu				= 3025;	
const MessageT	cmd_reload					= 3026;	
const MessageT	cmd_chooseShader			= 3027;	
const MessageT	cmd_animateShaders			= 3028;	

const int kDirectoryPopMenuID = 139;

// vertex info

const int SAVED_VERTEX_NUM = 1000;

// error codes

const OSErr	kError_outofmemory					= 9000;	
const OSErr	kError_corruptdata					= 9001;	
const OSErr	kError_badversion					= 9002;	
const OSErr	kError_missingpalette				= 9003;	


#endif // _H_AppConstants