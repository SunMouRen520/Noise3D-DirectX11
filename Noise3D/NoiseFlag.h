
/***********************************************************************

							h：标志

***********************************************************************/

#pragma once

enum NOISE_FILLMODE
{
	NOISE_FILLMODE_SOLID				= D3D11_FILL_SOLID,
	NOISE_FILLMODE_WIREFRAME		= D3D11_FILL_WIREFRAME,
	NOISE_FILLMODE_POINT				= 0,
};

enum NOISE_CULLMODE
{
	NOISE_CULLMODE_NONE			= D3D11_CULL_NONE,
	NOISE_CULLMODE_BACK				= D3D11_CULL_BACK,
	NOISE_CULLMODE_FRONT			= D3D11_CULL_FRONT,
};

enum NOISE_BLENDMODE
{
	NOISE_BLENDMODE_OPAQUE	= 0,
	NOISE_BLENDMODE_ALPHA	= 1,
	NOISE_BLENDMODE_ADDITIVE	= 2,
	NOISE_BLENDMODE_COLORFILTER = 3,
};

enum NOISE_VERTEX_TYPE
{
	NOISE_VERTEX_TYPE_SIMPLE =0,
	NOISE_VERTEX_TYPE_DEFAULT =1,
};

//灯光类型
enum NOISE_LIGHT_TYPE
{
	NOISE_LIGHT_TYPE_DIRECTIONAL = 0,
	NOISE_LIGHT_TYPE_POINT = 1,
	NOISE_LIGHT_TYPE_SPOT = 2,
};

//NoiseTimer的时间单位
enum NOISE_TIMER_TIMEUNIT
{
	NOISE_TIMER_TIMEUNIT_MILLISECOND = 0,
	NOISE_TIMER_TIMEUNIT_SECOND = 1,
};

enum NOISE_MAINLOOP_STATUS
{
	NOISE_MAINLOOP_STATUS_BUSY = 0,
	NOISE_MAINLOOP_STATUS_QUIT_LOOP = 1,
};

enum NOISE_GRAPHIC_OBJECT_TYPE
{
	NOISE_GRAPHIC_OBJECT_TYPE_POINT_3D		= 0,
	NOISE_GRAPHIC_OBJECT_TYPE_LINE_3D			= 1,
	NOISE_GRAPHIC_OBJECT_TYPE_POINT_2D		= 2,
	NOISE_GRAPHIC_OBJECT_TYPE_LINE_2D			= 3,
	NOISE_GRAPHIC_OBJECT_TYPE_TRIANGLE_2D	= 4,
};

enum NOISE_GRAPHIC_OBJECT_SHAPE_2D
{
	NOISE_GRAPHIC_OBJECT_SHAPE_2D_WIREFRAME_RECT = 0,
	NOISE_GRAPHIC_OBJECT_SHAPE_2D_WIREFRAME_ELLIPSE = 1,
	NOISE_GRAPHIC_OBJECT_SHAPE_2D_SOLID_RECT = 2,
	NOISE_GRAPHIC_OBJECT_SHAPE_2D_SOLID_ELLIPSE = 3,
	NOISE_GRAPHIC_OBJECT_SHAPE_2D_TEXTURED_RECT = 4,
	NOISE_GRAPHIC_OBJECT_SHAPE_2D_TEXTURED_ELLIPSE = 5,
};