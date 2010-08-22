/***************************************************************************
 * renderer.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2006 - 2010 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_RENDERER_H
#define SMC_RENDERER_H

#include "../video/video.h"
#include "../core/math/line.h"
#include "../core/math/rect.h"

namespace SMC
{

/* *** *** *** *** *** *** *** Render Types *** *** *** *** *** *** *** *** *** *** */

enum RenderType
{
	REND_NOTHING = 0,
	REND_RECT = 1,
	REND_GRADIENT = 2,
	REND_SURFACE = 3,
	REND_TEXT = 4, // todo
	REND_LINE = 5,
	REND_CIRCLE = 6
};

/* *** *** *** *** *** *** cRenderRequest *** *** *** *** *** *** *** *** *** *** *** */

class cRenderRequest
{
public:
	cRenderRequest( void );
	virtual ~cRenderRequest( void );

	// Draw the Render Request
	virtual void Draw( void );

	// render basic state
	void Render_Basic( void );
	// clear basic render state
	void Render_Basic_Clear( void ) const;

	// render advanced state
	void Render_Advanced( void );
	// clear advanced render state
	void Render_Advanced_Clear( void ) const;

	// render type
	RenderType type;

	// globalscale
	bool globalscale;
	// if not set camera position is subtracted
	bool no_camera;

	// Z position
	float pos_z;
	// rotation
	float rotx, roty, rotz;
	// blending
	GLenum blend_sfactor;
	GLenum blend_dfactor;
	// shadow position
	float shadow_pos;
	// shadow color
	Color shadow_color;

	// combine type
	GLint combine_type;
	// combine color
	float combine_col[3];

	// times to render until deletion
	int render_count;
};

typedef vector<cRenderRequest *> RenderList;

/* *** *** *** *** *** *** cLineRequest *** *** *** *** *** *** *** *** *** *** *** */

class cLine_Request : public cRenderRequest
{
public:
	cLine_Request( void );
	virtual ~cLine_Request( void );

	// draw
	virtual void Draw( void );

	// color
	Color color;
	// position
	GL_line line;
	// width
	float line_width;
	// stipple pattern
	GLushort stipple_pattern;
};

/* *** *** *** *** *** *** cRectRequest *** *** *** *** *** *** *** *** *** *** *** */

class cRect_Request : public cRenderRequest
{
public:
	cRect_Request( void );
	virtual ~cRect_Request( void );

	// draw
	virtual void Draw( void );
	// color
	Color color;
	// rect
	GL_rect rect;
	// rect is filled
	bool filled;
	// scale
	float scale_x;
	float scale_y;
	float scale_z;
	// line width (only used if not filled)
	float line_width;
	// stipple pattern (only used if not filled)
	GLushort stipple_pattern;
};

/* *** *** *** *** *** *** cGradientRequest *** *** *** *** *** *** *** *** *** *** *** */

class cGradient_Request : public cRenderRequest
{
public:
	cGradient_Request( void );
	virtual ~cGradient_Request( void );

	// draw
	virtual void Draw( void );

	// rect
	GL_rect rect;
	// direction
	ObjectDirection dir;
	// colors
	Color color_1;
	Color color_2;
};

/* *** *** *** *** *** *** cCircleRequest *** *** *** *** *** *** *** *** *** *** *** */

class cCircle_Request : public cRenderRequest
{
public:
	cCircle_Request( void );
	virtual ~cCircle_Request( void );

	// draw
	virtual void Draw( void );
	// color
	Color color;
	// position
	GL_point pos;
	// radius
	float radius;
	// if set circle is not filled
	float line_width;
};

/* *** *** *** *** *** *** cSurfaceRequest *** *** *** *** *** *** *** *** *** *** *** */

class cSurface_Request : public cRenderRequest
{
public:
	cSurface_Request( void );
	virtual ~cSurface_Request( void );

	// Draw
	virtual void Draw( void );

	// texture id
	GLuint texture_id;
	// position
	float pos_x, pos_y;
	// scale
	float scale_x;
	float scale_y;
	float scale_z;
	// size
	float w, h;

	// color
	Color color;

	// delete texture after request finished
	bool delete_texture;
};

/* *** *** *** *** *** *** cRenderQueue *** *** *** *** *** *** *** *** *** *** *** */

class cRenderQueue
{
public:
	cRenderQueue( unsigned int reserve_items );
	~cRenderQueue( void );

	/* Add a Render Request
	*/
	void Add( cRenderRequest *obj );

	/* Render current data
	 * clear: if set clear the finished data after rendering
	*/
	void Render( bool clear = 1 );

	/* Reduce the render count
	 * amount : render count decrease amount
	 * clear: if set clear the finished data after rendering
	*/
	void Fake_Render( unsigned int amount = 1, bool clear = 1 );

	/* clear the render data
	 * if force is given all objects will be removed
	*/
	void Clear( bool force = 1 );

	// renderdata array
	RenderList renderdata;

	// Z position sort
	struct zpos_sort
	{
		bool operator()( const cRenderRequest *a, const cRenderRequest *b ) const
		{
			return a->pos_z < b->pos_z;
		}
	};
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Renderer class
extern cRenderQueue *pRenderer;
// Renderer after GUI was drawn
extern cRenderQueue *pRenderer_GUI;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
