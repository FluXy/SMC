/***************************************************************************
 * animated_sprite.cpp  - multi image object sprite class
 *
 * Copyright (C) 2005 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../objects/animated_sprite.h"
#include "../core/game_core.h"
#include "../core/framerate.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cAnimation_Surface *** *** *** *** *** *** *** *** *** *** */

cAnimation_Surface :: cAnimation_Surface( void )
{
	m_image = NULL;
	m_time = 0;
}

cAnimation_Surface :: ~cAnimation_Surface( void )
{
	//
}

/* *** *** *** *** *** *** *** cAnimated_Sprite *** *** *** *** *** *** *** *** *** *** */

cAnimated_Sprite :: cAnimated_Sprite( cSprite_Manager *sprite_manager, std::string type_name /* = "sprite" */ )
: cMovingSprite( sprite_manager, type_name )
{
	m_curr_img = -1;

	m_anim_enabled = 0;
	m_anim_img_start = 0;
	m_anim_img_end = 0;
	m_anim_time_default = 1000;
	m_anim_counter = 0;
	m_anim_mod = 1.0f;
}

cAnimated_Sprite :: ~cAnimated_Sprite( void )
{
	//
}

void cAnimated_Sprite :: Add_Image( cGL_Surface *image, Uint32 time /* = 0 */ )
{
	// set to default time
	if( time == 0 )
	{
		time = m_anim_time_default;
	}

	cAnimation_Surface obj;
	obj.m_image = image;
	obj.m_time = time;

	m_images.push_back( obj );
}

void cAnimated_Sprite :: Set_Image_Num( const int num, const bool new_startimage /* = 0 */, const bool del_img /* = 0 */ )
{
	if( m_curr_img == num )
	{
		return;
	}

	m_curr_img = num;

	if( m_curr_img < 0 )
	{
		cMovingSprite::Set_Image( NULL, new_startimage, del_img );
	}
	else if( m_curr_img < static_cast<int>(m_images.size()) )
	{
		cMovingSprite::Set_Image( m_images[m_curr_img].m_image, new_startimage, del_img );
	}
	else
	{
		debug_print( "Warning : Object image number %d bigger as the array size %u, sprite type %d, name %s\n", m_curr_img, static_cast<unsigned int>(m_images.size()), m_type, m_name.c_str() );
	}
}

cGL_Surface *cAnimated_Sprite :: Get_Image( const unsigned int num ) const
{
	if( num >= m_images.size() )
	{
		return NULL;
	}

	return m_images[num].m_image;
}

void cAnimated_Sprite :: Clear_Images( void )
{
	m_curr_img = -1;
	m_images.clear();
}

void cAnimated_Sprite :: Update_Animation( void )
{
	// if not valid
	if( !m_anim_enabled || m_anim_img_end == 0 )
	{
		return;
	}

	m_anim_counter += pFramerate->m_elapsed_ticks;

	// out of range
	if( m_curr_img < 0 || m_curr_img >= static_cast<int>(m_images.size()) )
	{
		Set_Image_Num( m_anim_img_start );
		return;
	}

	cAnimation_Surface image = m_images[m_curr_img];

	if( static_cast<Uint32>(m_anim_counter * m_anim_mod) >= image.m_time )
	{
		if( m_curr_img >= m_anim_img_end )
		{
			Set_Image_Num( m_anim_img_start );
		}
		else
		{
			Set_Image_Num( m_curr_img + 1 );
		}
		
		m_anim_counter = static_cast<Uint32>(m_anim_counter * m_anim_mod) - image.m_time;
	}
}

void cAnimated_Sprite :: Set_Time_All( const Uint32 time, const bool default_time /* = 0 */ )
{
	for( cAnimation_Surface_List::iterator itr = m_images.begin(); itr != m_images.end(); ++itr )
	{
		cAnimation_Surface &obj = (*itr);
		obj.m_time = time;
	}

	if( default_time )
	{
		Set_Default_Time( time );
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
