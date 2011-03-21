/***************************************************************************
 * enemy.cpp  -  base class for all enemies
 *
 * Copyright (C) 2003 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../enemies/enemy.h"
#include "../core/camera.h"
#include "../video/animation.h"
#include "../user/savegame.h"
#include "../core/game_core.h"
#include "../level/level_player.h"
#include "../level/level_manager.h"

namespace SMC
{

/* *** *** *** *** *** *** cEnemy *** *** *** *** *** *** *** *** *** *** *** */

cEnemy :: cEnemy( cSprite_Manager *sprite_manager )
: cAnimated_Sprite( sprite_manager, "enemy" )
{
	m_sprite_array = ARRAY_ENEMY;
	m_type = TYPE_ENEMY;

	m_camera_range = 1500;

	m_massive_type = MASS_MASSIVE;
	m_state = STA_FALL;
	m_can_be_ground = 1;
	m_dead = 0;
	m_counter = 0.0f;

	m_kill_sound = "enemy/furball/die.ogg";
	m_kill_points = 10;

	m_velx_max = 0.0f;
	m_velx_gain = 0.0f;

	m_fire_resistant = 0;
	m_can_be_hit_from_shell = 1;
}

cEnemy :: ~cEnemy( void )
{

}

void cEnemy :: Load_From_Savegame( cSave_Level_Object *save_object )
{
	// state
	if( save_object->exists( "state" ) )
	{
		m_state = static_cast<Moving_state>(string_to_int( save_object->Get_Value( "state" ) ));
	}

	// new position x
	if( save_object->exists( "new_posx" ) )
	{
		Set_Pos_X( string_to_float( save_object->Get_Value( "new_posx" ) ) );
	}

	// new position y
	if( save_object->exists( "new_posy" ) )
	{
		Set_Pos_Y( string_to_float( save_object->Get_Value( "new_posy" ) ) );
	}

	// direction
	if( save_object->exists( "direction" ) )
	{
		m_direction = static_cast<ObjectDirection>(string_to_int( save_object->Get_Value( "direction" ) ));
	}

	// velocity x
	if( save_object->exists( "velx" ) )
	{
		m_velx = string_to_float( save_object->Get_Value( "velx" ) );
	}

	// velocity y
	if( save_object->exists( "vely" ) )
	{
		m_vely = string_to_float( save_object->Get_Value( "vely" ) );
	}

	// active
	if( save_object->exists( "active" ) )
	{
		Set_Active( string_to_int( save_object->Get_Value( "active" ) ) > 0 );
	}

	// dead
	if( save_object->exists( "dead" ) )
	{
		Set_Dead( string_to_int( save_object->Get_Value( "dead" ) ) > 0 );
	}
}

cSave_Level_Object *cEnemy :: Save_To_Savegame( void )
{
	cSave_Level_Object *save_object = new cSave_Level_Object();

	// default values
	save_object->m_type = m_type;
	save_object->m_properties.push_back( cSave_Level_Object_Property( "posx", int_to_string( static_cast<int>(m_start_pos_x) ) ) );
	save_object->m_properties.push_back( cSave_Level_Object_Property( "posy", int_to_string( static_cast<int>(m_start_pos_y) ) ) );



	// state
	save_object->m_properties.push_back( cSave_Level_Object_Property( "state", int_to_string( m_state ) ) );

	// new position ( only save if needed )
	if( !Is_Float_Equal( m_start_pos_x, m_pos_x ) || !Is_Float_Equal( m_start_pos_y, m_pos_y ) )
	{
		save_object->m_properties.push_back( cSave_Level_Object_Property( "new_posx", int_to_string( static_cast<int>(m_pos_x) ) ) );
		save_object->m_properties.push_back( cSave_Level_Object_Property( "new_posy", int_to_string( static_cast<int>(m_pos_y) ) ) );
	}

	// direction
	save_object->m_properties.push_back( cSave_Level_Object_Property( "direction", int_to_string( m_direction ) ) );

	// velocity
	save_object->m_properties.push_back( cSave_Level_Object_Property( "velx", float_to_string( m_velx ) ) );
	save_object->m_properties.push_back( cSave_Level_Object_Property( "vely", float_to_string( m_vely ) ) );

	// active ( only save if needed )
	if( !m_active )
	{
		save_object->m_properties.push_back( cSave_Level_Object_Property( "active", int_to_string( m_active ) ) );
	}

	// dead ( only save if needed )
	if( m_dead )
	{
		save_object->m_properties.push_back( cSave_Level_Object_Property( "dead", int_to_string( m_dead ) ) );
	}

	return save_object;
}

void cEnemy :: Set_Dead( bool enable /* = 1 */ )
{
	m_dead = enable;

	Update_Valid_Update();
}

void cEnemy :: Update( void )
{
	cMovingSprite::Update();

	// dying animation
	if( m_dead && m_active )
	{
		Update_Dying();
	}

	// frozen
	if( m_freeze_counter )
	{
		// update gravity
		if( m_type == TYPE_FURBALL || m_type == TYPE_TURTLE || m_type == TYPE_KRUSH || m_type == TYPE_SPIKA || m_type == TYPE_SPIKEBALL )
		{
			Update_Gravity();

			Col_Move( m_velx, m_vely );

			if( m_velx )
			{
				// slow down
				m_velx -= (m_velx * 0.06f) * pFramerate->m_speed_factor;
			}
		}
	}
}

void cEnemy :: Update_Late( void )
{
	// another object controls me
	if( m_state == STA_OBJ_LINKED )
	{
		// todo: have a parent pointer and use that instead of always the player
		Move( pLevel_Player->m_velx, pLevel_Player->m_vely );

		// handle collisions manually
		m_massive_type = MASS_MASSIVE;
		cObjectCollisionType *col_list = Collision_Check( &m_col_rect );
		Add_Collisions( col_list, 1 );
		delete col_list;
		Handle_Collisions();
		m_massive_type = MASS_PASSIVE;
	}
}

void cEnemy :: Update_Velocity( void )
{
	// note: this is currently only useful for walker enemy types
	if( m_direction == DIR_RIGHT )
	{
		if( m_velx < m_velx_max )
		{
			Add_Velocity_X_Max( m_velx_gain, m_velx_max );
			Set_Animation_Speed( m_velx / m_velx_max );
		}
		else if( m_velx > m_velx_max )
		{
			Add_Velocity_X_Min( -m_velx_gain, m_velx_max );
			Set_Animation_Speed( m_velx / m_velx_max );
		}
	}
	else if( m_direction == DIR_LEFT )
	{
		if( m_velx > -m_velx_max )
		{
			Add_Velocity_X_Min( -m_velx_gain, -m_velx_max );
			Set_Animation_Speed( m_velx / -m_velx_max );
		}
		else if( m_velx < -m_velx_max )
		{
			Add_Velocity_X_Max( m_velx_gain, -m_velx_max );
			Set_Animation_Speed( m_velx / -m_velx_max );
		}
	}
}

void cEnemy :: Update_Gravity( void )
{
	if( !m_ground_object )
	{
		if( m_vely < m_gravity_max )
		{
			Add_Velocity_Y_Max( 1.5f, m_gravity_max );
		}
		// below ground
		if( m_col_rect.m_y > pActive_Camera->m_limit_rect.m_y )
		{
			DownGrade( 1 );
		}
	}
	// has ground object
	else
	{
		// stop falling
		if( m_vely > 0.0f )
		{
			m_vely = 0.0f;
		}
	}
}

void cEnemy :: Generate_Hit_Animation( cParticle_Emitter *anim /* = NULL */ ) const
{
	bool create_anim = 0;

	if( !anim )
	{
		create_anim = 1;
		// create animation
		anim = new cParticle_Emitter( m_sprite_manager );
	}

	anim->Set_Emitter_Rect( m_col_rect.m_x, m_pos_y + ( m_col_rect.m_h / 3 ), m_col_rect.m_w );
	anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
	anim->Set_Quota( 4 );
	anim->Set_Pos_Z( m_pos_z - 0.000001f );
	anim->Set_Time_to_Live( 0.3f );
	Color col_rand = Color( static_cast<Uint8>( rand() % 5 ), rand() % 5, rand() % 100, 0 );
	// not bright enough
	/*if( col_rand.red + col_rand.green + col_rand.blue < 250 )
	{
		// boost a random color
		unsigned int rand_color = ( rand() % 3 );

		// yellow
		if( rand_color == 0 )
		{
			col_rand.red = 175;
			col_rand.green = 175;
		}
		// green
		else if( rand_color == 1 )
		{
			col_rand.green = 175;
		}
		// blue
		else
		{
			col_rand.blue = 175;
		}
	}*/
	anim->Set_Color( Color( static_cast<Uint8>(250), 250, 150, 255 ), col_rand );
	anim->Set_Speed( 0.5f, 2.6f );
	anim->Set_Scale( 0.2f, 0.6f );
	anim->Set_Direction_Range( 220, 100 );
	anim->Set_Fading_Alpha( 1 );
	//anim->Set_Fading_Size( 1 );
	anim->Set_Blending( BLEND_ADD );
	
	if( create_anim )
	{
		anim->Emit();
		pActive_Animation_Manager->Add( anim );
	}
}

void cEnemy :: Handle_Collision( cObjectCollision *collision )
{
	if( m_dead )
	{
		return;
	}

	cAnimated_Sprite::Handle_Collision( collision );
}

void cEnemy :: Handle_out_of_Level( ObjectDirection dir )
{
	if( dir == DIR_LEFT )
	{
		Set_Pos_X( pActive_Camera->m_limit_rect.m_x - m_col_pos.m_x );
	}
	else if( dir == DIR_RIGHT )
	{
		Set_Pos_X( pActive_Camera->m_limit_rect.m_x + pActive_Camera->m_limit_rect.m_w - m_col_pos.m_x - m_col_rect.m_w - 0.01f );
	}

	if( dir == DIR_LEFT || dir == DIR_RIGHT )
	{
		Turn_Around( dir );
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
