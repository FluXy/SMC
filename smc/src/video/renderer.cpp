/***************************************************************************
 * renderer.cpp  -  Render Queueing
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

#include "../video/renderer.h"
#include "../core/game_core.h"
#include <algorithm>
// SDL
#include "SDL.h"
#include "SDL_opengl.h"

namespace SMC
{

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const float doubled_pi = static_cast<float>(M_PI * 2.0f);
static GLuint last_bind_texture = 0;

/* *** *** *** *** *** *** cRenderRequest *** *** *** *** *** *** *** *** *** *** *** */

cRenderRequest :: cRenderRequest( void )
{
	type = REND_NOTHING;
	globalscale = 1;
	no_camera = 1;

	pos_z = 0.0f;

	rotx = 0.0f;
	roty = 0.0f;
	rotz = 0.0f;

	blend_sfactor = GL_SRC_ALPHA;
	blend_dfactor = GL_ONE_MINUS_SRC_ALPHA;

	shadow_pos = 0.0f;
	shadow_color = static_cast<Uint8>(0);

	combine_type = 0;
	combine_col[0] = 0.0f;
	combine_col[1] = 0.0f;
	combine_col[2] = 0.0f;

	render_count = 1;
}

cRenderRequest :: ~cRenderRequest( void )
{

}

void cRenderRequest :: Draw( void )
{
	// virtual
}

void cRenderRequest :: Render_Basic( void )
{
	// tried to replace this with gl push and pop but that was a lot slower on a Radeon X850 Pro
	// clear the matrix (default position and orientation)
	glLoadIdentity();

	// global scale
	if( globalscale && ( global_upscalex != 1.0f || global_upscaley != 1.0f ) )
	{
		glScalef( global_upscalex, global_upscaley, 1.0f );
	}

	// blend factor
	if( blend_sfactor != GL_SRC_ALPHA || blend_dfactor != GL_ONE_MINUS_SRC_ALPHA )
	{
		glBlendFunc( blend_sfactor, blend_dfactor );
	}
}

void cRenderRequest :: Render_Basic_Clear( void ) const
{
	// clear blend factor
	if( blend_sfactor != GL_SRC_ALPHA || blend_dfactor != GL_ONE_MINUS_SRC_ALPHA )
	{
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}

	// if debug build check for errors
#ifdef _DEBUG
	while( 1 )
	{
		GLenum error = glGetError();

		if( error != GL_NO_ERROR )
		{
			printf( "RenderRequest : GL Error found : %s\n", gluErrorString( error ) );
		}
		else
		{
			break;
		}
	}

#endif
}

void cRenderRequest :: Render_Advanced( void )
{
	// rotation
	if( rotx != 0.0f )
	{
		glRotatef( rotx, 1.0f, 0.0f, 0.0f );
	}
	if( roty != 0.0f )
	{
		glRotatef( roty, 0.0f, 1.0f, 0.0f );
	}
	if( rotz != 0.0f )
	{
		glRotatef( rotz, 0.0f, 0.0f, 1.0f );
	}

	// Color Combine
	if( combine_type != 0 )
	{
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
		glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, combine_type );
		glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_CONSTANT );
		glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, combine_col );
		glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE );
	}
}

void cRenderRequest :: Render_Advanced_Clear( void ) const
{
	// clear color modifications
	if( combine_type != 0 )
	{
		float col[3] = { 0.0f, 0.0f, 0.0f };
		glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, col );
		glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	}
}

/* *** *** *** *** *** *** cLineRequest *** *** *** *** *** *** *** *** *** *** *** */

cLine_Request :: cLine_Request( void )
: cRenderRequest()
{
	type = REND_LINE;
	color = static_cast<Uint8>(0);
	line = GL_line( 0.0f, 0.0f, 0.0f, 0.0f );
	line_width = 1.0f;
	stipple_pattern = 0;
}

cLine_Request :: ~cLine_Request( void )
{

}

void cLine_Request :: Draw( void )
{
	Render_Basic();

	// set camera position
	if( !no_camera )
	{
		glTranslatef( -pActive_Camera->m_x, -pActive_Camera->m_y, pos_z );
	}
	else
	{
		// only z position
		glTranslatef( 0.0f, 0.0f, pos_z );
	}

	Render_Advanced();

	// color
	if( color.red != 255 || color.green != 255 || color.blue != 255 || color.alpha != 255 )
	{
		glColor4ub( color.red, color.green, color.blue, color.alpha );
	}

	if( glIsEnabled( GL_TEXTURE_2D ) )
	{
		glDisable( GL_TEXTURE_2D );
	}

	// change width
	if( line_width != 1.0f )
	{
		glLineWidth( line_width );
	}
	// enable stipple pattern
	if( stipple_pattern != 0 )
	{
		glEnable( GL_LINE_STIPPLE );
		glLineStipple( 2, stipple_pattern );
	}

	glBegin( GL_LINES );
		glVertex2f( line.m_x1, line.m_y1 );
		glVertex2f( line.m_x2, line.m_y2 );
	glEnd();

	// clear stipple pattern
	if( stipple_pattern != 0 )
	{
		glDisable( GL_LINE_STIPPLE );
	}
	// clear line width
	if( line_width != 1.0f )
	{
		glLineWidth( 1.0f );
	}

	// clear color
	if( color.red != 255 || color.green != 255 || color.blue != 255 || color.alpha != 255 )
	{
		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	}

	Render_Advanced_Clear();
	Render_Basic_Clear();
}

/* *** *** *** *** *** *** cRectRequest *** *** *** *** *** *** *** *** *** *** *** */

cRect_Request :: cRect_Request( void )
: cRenderRequest()
{
	type = REND_RECT;
	color = static_cast<Uint8>(0);

	rect = GL_rect( 0.0f, 0.0f, 0.0f, 0.0f );
	filled = 1;
	scale_x = 1.0f;
	scale_y = 1.0f;
	scale_z = 1.0f;

	line_width = 1.0f;
	stipple_pattern = 0;
}

cRect_Request :: ~cRect_Request( void )
{

}

void cRect_Request :: Draw( void )
{
	Render_Basic();

	// get half the size
	float half_w = rect.m_w / 2;
	float half_h = rect.m_h / 2;
	// position
	float final_pos_x = rect.m_x + ( half_w * scale_x );
	float final_pos_y = rect.m_y + ( half_h * scale_y );

	// set camera position
	if( !no_camera )
	{
		final_pos_x -= pActive_Camera->m_x;
		final_pos_y -= pActive_Camera->m_y;
	}

	glTranslatef( final_pos_x, final_pos_y, pos_z );

	// scale
	if( scale_x != 1.0f || scale_y != 1.0f || scale_z != 1.0f )
	{
		glScalef( scale_x, scale_y, scale_z );
	}

	Render_Advanced();

	// color
	if( color.red != 255 || color.green != 255 || color.blue != 255 || color.alpha != 255 )
	{
		glColor4ub( color.red, color.green, color.blue, color.alpha );
	}

	if( glIsEnabled( GL_TEXTURE_2D ) )
	{
		glDisable( GL_TEXTURE_2D );
	}

	if( filled )
	{
		glBegin( GL_POLYGON );
	}
	else
	{
		// change width
		if( line_width != 1.0f )
		{
			glLineWidth( line_width );
		}
		// enable stipple pattern
		if( stipple_pattern != 0 )
		{
			glEnable( GL_LINE_STIPPLE );
			glLineStipple( 2, stipple_pattern );
		}

		glBegin( GL_LINE_LOOP );
	}
		// top left
		glVertex2f( -half_w, -half_h );
		// top right
		glVertex2f( half_w, -half_h );
		// bottom right
		glVertex2f( half_w, half_h );
		// bottom left
		glVertex2f( -half_w, half_h );
	glEnd();

	// clear stipple pattern
	if( stipple_pattern != 0 )
	{
		glDisable( GL_LINE_STIPPLE );
	}
	// clear line width
	if( line_width != 1.0f )
	{
		glLineWidth( 1.0f );
	}

	// clear color
	if( color.red != 255 || color.green != 255 || color.blue != 255 || color.alpha != 255 )
	{
		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	}

	Render_Advanced_Clear();
	Render_Basic_Clear();
}

/* *** *** *** *** *** *** cGradientRequest *** *** *** *** *** *** *** *** *** *** *** */

cGradient_Request :: cGradient_Request( void )
: cRenderRequest()
{
	type = REND_GRADIENT;
	rect = GL_rect( 0.0f, 0.0f, 0.0f, 0.0f );
	dir = DIR_UNDEFINED;
	color_1 = static_cast<Uint8>(0);
	color_2 = static_cast<Uint8>(0);
}

cGradient_Request :: ~cGradient_Request( void )
{

}

void cGradient_Request :: Draw( void )
{
	Render_Basic();

	// set camera position
	if( !no_camera )
	{
		glTranslatef( rect.m_x - pActive_Camera->m_x, rect.m_y - pActive_Camera->m_y, pos_z );
	}
	// ignore camera position
	else
	{
		glTranslatef( rect.m_x, rect.m_y, pos_z );
	}

	if( glIsEnabled( GL_TEXTURE_2D ) )
	{
		glDisable( GL_TEXTURE_2D );
	}

	Render_Advanced();

	if( dir == DIR_VERTICAL )
	{
		glBegin( GL_POLYGON );
			glColor4ub( color_1.red, color_1.green, color_1.blue, color_1.alpha );
			glVertex2f( 0.0f, 0.0f );
			glVertex2f( rect.m_w, 0.0f );
			glColor4ub( color_2.red, color_2.green, color_2.blue, color_2.alpha );
			glVertex2f( rect.m_w, rect.m_h );
			glVertex2f( 0.0f, rect.m_h );
		glEnd();

	}
	else if( dir == DIR_HORIZONTAL )
	{
		glBegin( GL_POLYGON );
			glColor4ub( color_1.red, color_1.green, color_1.blue, color_1.alpha );
			glVertex2f( 0.0f, rect.m_h );
			glVertex2f( 0.0f, 0.0f );
			glColor4ub( color_2.red, color_2.green, color_2.blue, color_2.alpha );
			glVertex2f( rect.m_w, 0.0f );
			glVertex2f( rect.m_w, rect.m_h );
		glEnd();
	}

	// clear color
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

	Render_Advanced_Clear();
	Render_Basic_Clear();
}

/* *** *** *** *** *** *** cCircleRequest *** *** *** *** *** *** *** *** *** *** *** */

cCircle_Request :: cCircle_Request( void )
: cRenderRequest()
{
	type = REND_CIRCLE;
	color = static_cast<Uint8>(0);
	pos = GL_point( 0.0f, 0.0f );
	radius = 0.1f;
	// default is filled
	line_width = 0;
}

cCircle_Request :: ~cCircle_Request( void )
{

}

void cCircle_Request :: Draw( void )
{
	Render_Basic();

	// set camera position
	if( !no_camera )
	{
		glTranslatef( pos.m_x - pActive_Camera->m_x, pos.m_y - pActive_Camera->m_y, pos_z );
	}
	// ignore camera position
	else
	{
		glTranslatef( pos.m_x, pos.m_y, pos_z );
	}

	Render_Advanced();

	// color
	if( color.red != 255 || color.green != 255 || color.blue != 255 || color.alpha != 255 )
	{
		glColor4ub( color.red, color.green, color.blue, color.alpha );
	}

	if( glIsEnabled( GL_TEXTURE_2D ) )
	{
		glDisable( GL_TEXTURE_2D );
	}

	// not filled
	if( line_width )
	{
		// set line width
		glLineWidth( line_width );

		glBegin( GL_LINE_STRIP );
	}
	// filled
	else
	{
		glBegin( GL_TRIANGLE_FAN );

		// start with center
		glVertex2f( 0.0f, 0.0f );
	}

	// set step size based on radius
	float step_size = 1.0f / ( radius * 0.05f );

	// minimum step size
	if( step_size > 0.2f )
	{
		step_size = 0.2f;
	}

	float angle = 0.0f;

	while( angle < doubled_pi )
	{
		glVertex2f( radius * sin( angle ), radius * cos( angle ) );
		angle += step_size;
	}

	// draw to end
	angle = doubled_pi;
	glVertex2f( radius * sin( angle ), radius * cos( angle ) );

	glEnd();

	// clear line width
	if( line_width != 1 )
	{
		glLineWidth( 1.0f );
	}

	// clear color
	if( color.red != 255 || color.green != 255 || color.blue != 255 || color.alpha != 255 )
	{
		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	}

	Render_Advanced_Clear();
	Render_Basic_Clear();
}

/* *** *** *** *** *** *** cSurfaceRequest *** *** *** *** *** *** *** *** *** *** *** */

cSurface_Request :: cSurface_Request( void )
: cRenderRequest()
{
	type = REND_SURFACE;
	texture_id = 0;

	pos_x = 0.0f;
	pos_y = 0.0f;

	w = 0.0f;
	h = 0.0f;

	scale_x = 1.0f;
	scale_y = 1.0f;
	scale_z = 1.0f;

	color = static_cast<Uint8>(255);

	delete_texture = 0;
}

cSurface_Request :: ~cSurface_Request( void )
{
	if( delete_texture && glIsTexture( texture_id ) )
	{
		glDeleteTextures( 1, &texture_id );
	}
}

void cSurface_Request :: Draw( void )
{
	// draw shadow
	if( shadow_pos )
	{
		// shadow position
		pos_x += shadow_pos;
		pos_y += shadow_pos;
		pos_z -= 0.000001f;

		// save data
		Color temp_color = color;
		float temp_shadow_pos = shadow_pos;
		GLint temp_combine_type = combine_type;
		float temp_combine_col[3];
		temp_combine_col[0] = combine_col[0];
		temp_combine_col[1] = combine_col[1];
		temp_combine_col[2] = combine_col[2];

		// temporarily unset to prevent endless loop
		shadow_pos = 0;
		// shadow as a white texture
		color = black;
		// keep shadow_color alpha
		color.alpha = shadow_color.alpha;
		// combine color
		combine_type = GL_REPLACE;
		combine_col[0] = static_cast<float>(shadow_color.red) / 260;
		combine_col[1] = static_cast<float>(shadow_color.green) / 260;
		combine_col[2] = static_cast<float>(shadow_color.blue) / 260;

		// draw shadow
		Draw();

		// set back data
		shadow_pos = temp_shadow_pos;
		color = temp_color;
		combine_type = temp_combine_type;
		combine_col[0] = temp_combine_col[0];
		combine_col[1] = temp_combine_col[1];
		combine_col[2] = temp_combine_col[2];
		pos_z += 0.000001f;

		// move back to original position
		pos_x -= shadow_pos;
		pos_y -= shadow_pos;
	}

	Render_Basic();

	// get half the size
	float half_w = w / 2;
	float half_h = h / 2;
	// position
	float final_pos_x = pos_x + ( half_w * scale_x );
	float final_pos_y = pos_y + ( half_h * scale_y );

	// set camera position
	if( !no_camera )
	{
		final_pos_x -= pActive_Camera->m_x;
		final_pos_y -= pActive_Camera->m_y;
	}

	glTranslatef( final_pos_x, final_pos_y, pos_z );

	// scale
	if( scale_x != 1.0f || scale_y != 1.0f || scale_z != 1.0f )
	{
		glScalef( scale_x, scale_y, scale_z );
	}

	Render_Advanced();

	// color
	if( color.red != 255 || color.green != 255 || color.blue != 255 || color.alpha != 255 )
	{
		glColor4ub( color.red, color.green, color.blue, color.alpha );
	}

	if( !glIsEnabled( GL_TEXTURE_2D ) )
	{
		glEnable( GL_TEXTURE_2D );
	}

	// only bind if not the same texture
	if( last_bind_texture != texture_id )
	{
		glBindTexture( GL_TEXTURE_2D, texture_id );
		last_bind_texture = texture_id;
	}

	/* vertex arrays should not be used to draw simple primitives as it
	 * does have no positive performance gain
	*/
	// rectangle
	glBegin( GL_QUADS );
		// top left
		glTexCoord2f( 0.0f, 0.0f );
		glVertex2f( -half_w, -half_h );
		// top right
		glTexCoord2f( 1.0f, 0.0f );
		glVertex2f( half_w, -half_h );
		// bottom right
		glTexCoord2f( 1.0f, 1.0f );
		glVertex2f( half_w, half_h );
		// bottom left
		glTexCoord2f( 0.0f, 1.0f );
		glVertex2f( -half_w, half_h );
	glEnd();

	// clear color
	if( color.red != 255 || color.green != 255 || color.blue != 255 || color.alpha != 255 )
	{
		/* alpha is automatically 1 for glColor3f
		 * update : not on the shitty intel drivers :(
		*/
		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	}

	Render_Advanced_Clear();
	Render_Basic_Clear();
}

/* *** *** *** *** *** *** cRenderQueue *** *** *** *** *** *** *** *** *** *** *** */

cRenderQueue :: cRenderQueue( unsigned int reserve_items )
{
	renderdata.reserve( reserve_items );
}

cRenderQueue :: ~cRenderQueue( void )
{
	Clear();
}

void cRenderQueue :: Add( cRenderRequest *obj )
{
	if( !obj )
	{
		return;
	}

	// if no type
	if( obj->type == REND_NOTHING )
	{
		delete obj;
		return;
	}

	renderdata.push_back( obj );
}

void cRenderQueue :: Render( bool clear /* = 1 */ )
{
	// z position sort
	std::sort( renderdata.begin(), renderdata.end(), zpos_sort() );
	// reset last texture
	last_bind_texture = 0;

	for( RenderList::iterator itr = renderdata.begin(); itr != renderdata.end(); ++itr )
	{
		cRenderRequest *obj = (*itr);

		obj->Draw();
		obj->render_count--;
	}

	if( clear )
	{
		Clear( 0 );
	}
}

void cRenderQueue :: Fake_Render( unsigned int amount /* = 1 */, bool clear /* = 1 */ )
{
	for( RenderList::iterator itr = renderdata.begin(); itr != renderdata.end(); ++itr )
	{
		cRenderRequest *obj = (*itr);
		obj->render_count -= amount;
	}

	if( clear )
	{
		Clear( 0 );
	}
}

void cRenderQueue :: Clear( bool force /* = 1 */ )
{
	for( RenderList::iterator itr = renderdata.begin(); itr != renderdata.end(); )
	{
		cRenderRequest *obj = (*itr);

		// if forced or finished rendering
		if( force || obj->render_count <= 0 )
		{
			itr = renderdata.erase( itr );
			delete obj;
		}
		// increment
		else
		{
			++itr;
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cRenderQueue *pRenderer = NULL;
cRenderQueue *pRenderer_GUI = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
