/***************************************************************************
 * global_effect.cpp  -  class for handling level global effects
 *
 * Copyright (C) 2006 - 2009 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
 
#include "../level/global_effect.h"
#include "../core/framerate.h"
#include "../level/level_editor.h"
#include "../core/camera.h"
#include "../core/game_core.h"
#include "../video/gl_surface.h"
#include "../core/filesystem/filesystem.h"
// CEGUI
#include "CEGUIXMLAttributes.h"

namespace SMC
{

/* *** *** *** *** *** cGlobal_effect *** *** *** *** *** *** *** *** *** *** *** *** */

cGlobal_effect :: cGlobal_effect( cSprite_Manager *sprite_manager )
: cParticle_Emitter( sprite_manager )
{
	cGlobal_effect::Clear();
}

cGlobal_effect :: cGlobal_effect( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager )
: cParticle_Emitter( sprite_manager )
{
	cGlobal_effect::Clear();
	cGlobal_effect::Create_From_Stream( attributes );
}

cGlobal_effect :: ~cGlobal_effect( void )
{

}

void cGlobal_effect :: Init_Anim( void )
{
	if( m_global_effect_type == GL_EFF_NONE )
	{
		return;
	}

	if( m_global_effect_type == GL_EFF_FALLING )
	{
		// image doesn't exist
		if( !pVideo->Get_Surface( m_image_filename ) )
		{
			m_valid = 0;
		}
		// image exists
		else
		{
			m_image = pVideo->Get_Surface( m_image_filename );
			// valid effect
			m_valid = 1;
		}
	}

	// update ahead
	if( m_valid )
	{
		float old_speedfactor = pFramerate->m_speed_factor;
		pFramerate->m_speed_factor = 1.0f;
		// use time to live as seconds
		for( float i = 0.0f; i < speedfactor_fps * time_to_live; i++ )
		{
			Update();
		}

		pFramerate->m_speed_factor = old_speedfactor;
	}
}

void cGlobal_effect :: Clear( void )
{
	cParticle_Emitter::Clear();

	m_global_effect_type = GL_EFF_NONE;

	m_image_filename.clear();
	m_image = NULL;
	Set_Emitter_Rect( 0, 0, static_cast<float>(game_res_w), 0 );
	Set_Emitter_Time_to_Live( -1 );
	Set_Emitter_Iteration_Interval( 0.3f );
	Set_Pos_Z( 0.12f, 0 );
	Set_Time_to_Live( 7, 0 );
	Set_Scale( 0.2f, 0.2f );
	Set_Speed( 2, 8 );
	Set_Direction_Range( 0, 90 );
	Set_Rotation( 0, 0, 0, 1 );
	Set_Const_Rotation_Z( -5, 10 );

	m_valid = 0;
}

void cGlobal_effect :: Create_From_Stream( CEGUI::XMLAttributes &attributes )
{
	// Type
	Set_Type( static_cast<GlobalEffectType>(attributes.getValueAsInteger( "type", m_global_effect_type )) );
	// Image
	Set_Image( attributes.getValueAsString( "image", m_image_filename ).c_str() );
	// Creation Rect
	Set_Emitter_Rect( static_cast<float>(attributes.getValueAsInteger( "rect_x", static_cast<int>(m_start_pos_x) )), static_cast<float>(attributes.getValueAsInteger( "rect_y", static_cast<int>(m_start_pos_y) )), static_cast<float>(attributes.getValueAsInteger( "rect_w", static_cast<int>(m_rect.m_w) )), static_cast<float>(attributes.getValueAsInteger( "rect_h", static_cast<int>(m_rect.m_h) )) );
	// Z Position
	Set_Pos_Z( attributes.getValueAsFloat( "z", m_pos_z ), attributes.getValueAsFloat( "z_rand", posz_rand ) );
	// Time to Live
	if( attributes.exists( "time_to_live" ) )
	{
		Set_Time_to_Live( attributes.getValueAsFloat( "time_to_live", time_to_live ), attributes.getValueAsFloat( "time_to_live_rand", time_to_live_rand ) );
	}
	// if not set uses old Lifetime mod ( 0.99.4 and below )
	else
	{
		Set_Time_to_Live( attributes.getValueAsFloat( "lifetime_mod", 20 ) * 0.3f );
	}
	// Emitter Iteration Interval
	if( attributes.exists( "emitter_iteration_interval" ) )
	{
		Set_Emitter_Iteration_Interval( attributes.getValueAsFloat( "emitter_iteration_interval", emitter_iteration_interval ) );
	}
	// if not set uses old Creation speed ( 0.99.7 and below )
	else
	{
		Set_Emitter_Iteration_Interval( ( 1 / attributes.getValueAsFloat( "creation_speed", 0.3f ) ) * 0.032f );
	}
	// Scale
	Set_Scale( attributes.getValueAsFloat( "scale", size_scale ), attributes.getValueAsFloat( "scale_rand", size_scale_rand ) );
	// Speed
	Set_Speed( attributes.getValueAsFloat( "speed", vel ), attributes.getValueAsFloat( "speed_rand", vel_rand ) );
	// Direction
	Set_Direction_Range( attributes.getValueAsFloat( "dir_range_start", angle_start ), attributes.getValueAsFloat( "dir_range_size", angle_range ) );
	// start rotation
	Set_Rotation( attributes.getValueAsFloat( "rot_x", m_start_rot_x ), attributes.getValueAsFloat( "rot_y", m_start_rot_y ), attributes.getValueAsFloat( "rot_z", m_start_rot_z ), 1 );
	// Constant Rotation Z
	Set_Const_Rotation_Z( attributes.getValueAsFloat( "const_rotz", const_rotz ), attributes.getValueAsFloat( "const_rotz_rand", const_rotz_rand ) );
}

void cGlobal_effect :: Save_To_Stream( ofstream &file )
{
	if( m_global_effect_type == GL_EFF_NONE )
	{
		return;
	}

	// begin global effect
	file << "\t<global_effect>" << std::endl;

	// type
	file << "\t\t<Property name=\"type\" value=\"" << m_global_effect_type << "\" />" << std::endl;
	// image
	std::string img_filename = m_image_filename;

	if( img_filename.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == 0 )
	{
		img_filename.erase( 0, strlen( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) );
	}

	file << "\t\t<Property name=\"image\" value=\"" << img_filename << "\" />" << std::endl;
	// rect
	file << "\t\t<Property name=\"rect_x\" value=\"" << m_start_pos_x << "\" />" << std::endl;
	file << "\t\t<Property name=\"rect_y\" value=\"" << m_start_pos_y << "\" />" << std::endl;
	file << "\t\t<Property name=\"rect_w\" value=\"" << m_rect.m_w << "\" />" << std::endl;
	file << "\t\t<Property name=\"rect_h\" value=\"" << m_rect.m_h << "\" />" << std::endl;
	// Z Position
	file << "\t\t<Property name=\"z\" value=\"" << m_pos_z << "\" />" << std::endl;
	file << "\t\t<Property name=\"z_rand\" value=\"" << posz_rand << "\" />" << std::endl;
	// Time to Live
	file << "\t\t<Property name=\"time_to_live\" value=\"" << time_to_live << "\" />" << std::endl;
	file << "\t\t<Property name=\"time_to_live_rand\" value=\"" << time_to_live_rand << "\" />" << std::endl;
	// Emitter Iteration Interval
	file << "\t\t<Property name=\"emitter_iteration_interval\" value=\"" << emitter_iteration_interval << "\" />" << std::endl;
	// scale
	file << "\t\t<Property name=\"scale\" value=\"" << size_scale << "\" />" << std::endl;
	file << "\t\t<Property name=\"scale_rand\" value=\"" << size_scale_rand << "\" />" << std::endl;
	// speed
	file << "\t\t<Property name=\"speed\" value=\"" << vel << "\" />" << std::endl;
	file << "\t\t<Property name=\"speed_rand\" value=\"" << vel_rand << "\" />" << std::endl;
	// direction range
	file << "\t\t<Property name=\"dir_range_start\" value=\"" << angle_start << "\" />" << std::endl;
	file << "\t\t<Property name=\"dir_range_size\" value=\"" << angle_range << "\" />" << std::endl;
	// start rotation
	file << "\t\t<Property name=\"rot_x\" value=\"" << m_start_rot_x << "\" />" << std::endl;
	file << "\t\t<Property name=\"rot_y\" value=\"" << m_start_rot_y << "\" />" << std::endl;
	file << "\t\t<Property name=\"rot_z\" value=\"" << m_start_rot_z << "\" />" << std::endl;
	// constant rotation Z
	file << "\t\t<Property name=\"const_rotz\" value=\"" << const_rotz << "\" />" << std::endl;
	file << "\t\t<Property name=\"const_rotz_rand\" value=\"" << const_rotz_rand << "\" />" << std::endl;

	// end global effect
	file << "\t</global_effect>" << std::endl;
}

void cGlobal_effect :: Update( void )
{
	if( editor_level_enabled || !m_valid )
	{
		return;
	}

	// if disabled
	if( m_global_effect_type == GL_EFF_NONE )
	{
		return;
	}
	
	Set_Pos( m_start_pos_x + pActive_Camera->m_x, m_start_pos_y + pActive_Camera->m_y );
	// update particles
	Update_Particles();
	// update particle animation
	cParticle_Emitter::Update();
}

void cGlobal_effect :: Update_Particles( void )
{
	GL_rect camera_rect = pActive_Camera->Get_Rect();
	// temporary animation rect
	GL_rect anim_rect;

	for( ParticleList::iterator itr = objects.begin(); itr != objects.end(); ++itr )
	{
		// get animation particle pointer
		cParticle *obj = static_cast<cParticle *>(*itr);
		
		// set rectangle
		anim_rect = GL_rect( obj->m_pos_x, obj->m_pos_y, obj->m_rect.m_w, obj->m_rect.m_h );
		
		// if on screen
		if( camera_rect.Intersects( anim_rect ) )
		{
			continue;
		}

		// out in left
		if( anim_rect.m_x + anim_rect.m_w < camera_rect.m_x )
		{
			// move to right
			obj->Set_Pos_X( camera_rect.m_x + camera_rect.m_w - 1 );
		}
		// out in right
		else if( anim_rect.m_x > camera_rect.m_x + camera_rect.m_w )
		{
			// move to left
			obj->Set_Pos_X( camera_rect.m_x + 1 );
		}
		// out on top
		else if( anim_rect.m_y + anim_rect.m_h < camera_rect.m_y )
		{
			// move to bottom
			obj->Set_Pos_Y( camera_rect.m_y + camera_rect.m_h - 1 );
		}
		// out on bottom
		else if( anim_rect.m_y > camera_rect.m_y + camera_rect.m_h )
		{
			// move to top
			obj->Set_Pos_Y( camera_rect.m_y + 1 );
		}
	}
}

void cGlobal_effect :: Draw( void )
{
	cParticle_Emitter::Draw();
}

void cGlobal_effect :: Set_Type( const GlobalEffectType type )
{
	if( m_global_effect_type == type )
	{
		return;
	}

	m_global_effect_type = type;

	if( m_global_effect_type == GL_EFF_NONE )
	{
		cParticle_Emitter::Clear();
	}
}

void cGlobal_effect :: Set_Type( const std::string &str_type )
{
	if( str_type.compare( "Disabled" ) == 0 )
	{
		Set_Type( GL_EFF_NONE );
	}
	else if( str_type.compare( "Falling" ) == 0 || str_type.compare( "Default" ) == 0 )
	{
		Set_Type( GL_EFF_FALLING );
	}
	else
	{
		printf( "Warning : Unknown Global Effect type %s\n", str_type.c_str() );
	}
}

void cGlobal_effect :: Set_Image( const std::string &img_file )
{
	m_image_filename = img_file;
	Convert_Path_Separators( m_image_filename );
}

std::string cGlobal_effect :: Get_Type_Name( void ) const
{
	if( m_global_effect_type == GL_EFF_NONE )
	{
		return "Disabled";
	}
	else if( m_global_effect_type == GL_EFF_FALLING )
	{
		return "Falling";
	}

	return "Unknown";
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
