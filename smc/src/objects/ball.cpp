/***************************************************************************
 * ball.cpp  -  ball class
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

#include "../objects/ball.h"
#include "../core/game_core.h"
#include "../enemies/enemy.h"
#include "../objects/goldpiece.h"
#include "../enemies/gee.h"
#include "../enemies/spika.h"
#include "../level/level_player.h"
#include "../video/animation.h"
#include "../level/level.h"
#include "../gui/hud.h"
#include "../core/sprite_manager.h"

namespace SMC
{

/* *** *** *** *** *** *** cBall *** *** *** *** *** *** *** *** *** *** *** */

cBall :: cBall( cSprite_Manager *sprite_manager, float x, float y, const cSprite *origin_object /* = NULL */, ball_effect btype /* = FIREBALL_DEFAULT */  )
: cAnimated_Sprite( sprite_manager )
{
	m_sprite_array = ARRAY_ACTIVE;
	m_type = TYPE_BALL;
	m_pos_z = 0.095f;
	m_gravity_max = 20.0f;

	Set_Spawned( 1 );
	m_camera_range = 2000;

	m_massive_type = MASS_MASSIVE;

	m_glim_mod = 1;
	m_glim_counter = 0.0f;
	m_fire_counter = 0.0f;

	Set_Pos( x, y, 1 );

	if( btype == FIREBALL_DEFAULT || btype == FIREBALL_EXPLOSION )
	{
		Add_Image( pVideo->Get_Surface( "animation/fireball/1.png" ) );
		Add_Image( pVideo->Get_Surface( "animation/fireball/2.png" ) );
		Add_Image( pVideo->Get_Surface( "animation/fireball/3.png" ) );
		Set_Image_Num( 0, 1 );
		Set_Animation( 1 );
		Set_Animation_Image_Range( 0, 2 );
		Set_Time_All( 120, 1 );

		m_ball_type = FIREBALL_DEFAULT;
	}
	else if( btype == ICEBALL_DEFAULT || btype == ICEBALL_EXPLOSION )
	{
		Set_Image( pVideo->Get_Surface( "animation/iceball/1.png" ) );
		m_ball_type = ICEBALL_DEFAULT;
	}
	else
	{
		printf( "Warning : Ball unknown type %d\n", btype );
		cAnimated_Sprite::Destroy();
		return;
	}

	if( origin_object )
	{
		m_origin_array = origin_object->m_sprite_array;
		m_origin_type = origin_object->m_type;

		if( m_origin_type == TYPE_PLAYER )
		{
			if( m_ball_type == FIREBALL_DEFAULT || m_ball_type == ICEBALL_DEFAULT )
			{
				pLevel_Player->m_shoot_counter = speedfactor_fps;
			}
		}
	}
	// if origin not set
	else
	{
		printf( "Warning : Ball origin not set\n" );
		m_origin_array = ARRAY_UNDEFINED;
		m_origin_type = TYPE_UNDEFINED;
	}
}

cBall :: ~cBall( void )
{
	// always destroy
	if( !m_auto_destroy )
	{
		cBall::Destroy();
	}
}

void cBall :: Destroy_Ball( bool with_sound /* = 0 */ )
{
	if( with_sound )
	{
		if( m_ball_type == FIREBALL_DEFAULT )
		{
			pAudio->Play_Sound( "item/fireball_explode.wav" );
		}
	}

	Destroy();
}

void cBall :: Destroy( void )
{
	if( m_auto_destroy )
	{
		return;
	}

	if( m_ball_type == FIREBALL_DEFAULT )
	{
		pActive_Animation_Manager->Add( new cAnimation_Fireball( m_sprite_manager, m_pos_x + m_col_rect.m_w / 2, m_pos_y + m_col_rect.m_h / 2 ) );
	}
	else if( m_ball_type == ICEBALL_DEFAULT )
	{
		// create animation
		cParticle_Emitter *anim = new cParticle_Emitter( m_sprite_manager );
		Generate_Particles( anim );
		anim->Set_Quota( 15 );
		anim->Emit();
		pActive_Animation_Manager->Add( anim );
	}

	cAnimated_Sprite::Destroy();
}

void cBall :: Update( void )
{
	if( !m_valid_update )
	{
		return;
	}

	// if this is out of range
	if( !Is_In_Range() )
	{
		Destroy();
	}

	Update_Animation();

	// right
	if( m_velx > 0.0f )
	{
		m_rot_z += pFramerate->m_speed_factor * 40;
	}
	// left
	else
	{
		m_rot_z -= pFramerate->m_speed_factor * 40;
	}

	if( m_vely < m_gravity_max )
	{
		Add_Velocity_Y_Max( 1.0f, m_gravity_max );
	}

	// glim animation
	if( m_glim_mod )
	{
		m_glim_counter += pFramerate->m_speed_factor * 0.1f;

		if( m_glim_counter > 1.0f )
		{
			m_glim_counter = 1.0f;
			m_glim_mod = 0;
		}
	}
	else
	{
		m_glim_counter -= pFramerate->m_speed_factor * 0.1f;

		if( m_glim_counter < 0.0f )
		{
			m_glim_counter = 0.0f;
			m_glim_mod = 1;
		}
	}

	// generate fire particle animation
	m_fire_counter += pFramerate->m_speed_factor;
	while( m_fire_counter > 1 )
	{
		Generate_Particles();
		m_fire_counter -= 1;
	}
}

void cBall :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( !m_valid_draw )
	{
		return;
	}

	// don't draw if leveleditor mode
	if( editor_level_enabled )
	{
		return;
	}

	if( m_ball_type == FIREBALL_DEFAULT )
	{
		Set_Color_Combine( m_glim_counter, m_glim_counter / 1.2f, m_glim_counter / 2, GL_ADD );
	}
	else if( m_ball_type == ICEBALL_DEFAULT )
	{
		Set_Color_Combine( m_glim_counter / 6.0f, m_glim_counter / 6.0f, m_glim_counter / 6.0f, GL_ADD );
	}

	cAnimated_Sprite::Draw( request );
}

void cBall :: Generate_Particles( cParticle_Emitter *anim /* = NULL */ ) const
{
	bool create_anim = 0;

	if( !anim )
	{
		create_anim = 1;
		// create animation
		anim = new cParticle_Emitter( m_sprite_manager );
	}

	anim->Set_Emitter_Rect( m_col_rect );
	anim->Set_Pos_Z( m_pos_z + 0.0001f );
	if( m_ball_type == FIREBALL_DEFAULT )
	{
		anim->Set_Image( pVideo->Get_Surface( "animation/particles/fire_2.png" ) );
		anim->Set_Time_to_Live( 0.2f );
	}
	// ice
	else
	{
		anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
		anim->Set_Time_to_Live( 0.5f );
		anim->Set_Color( Color( static_cast<Uint8>(90), 90, 255 ) );
	}
	anim->Set_Blending( BLEND_ADD );
	anim->Set_Speed( 0.35f, 0.3f );
	anim->Set_Scale( 0.4f, 0.2f );
	
	if( create_anim )
	{
		anim->Emit();
		pActive_Animation_Manager->Add( anim );
	}
}

Col_Valid_Type cBall :: Validate_Collision( cSprite *obj )
{
	// basic validation checking
	Col_Valid_Type basic_valid = Validate_Collision_Ghost( obj );

	// found valid collision
	if( basic_valid != COL_VTYPE_NOT_POSSIBLE )
	{
		return basic_valid;
	}

	switch( obj->m_type )
	{
		case TYPE_PLAYER:
		{
			if( m_origin_type != TYPE_PLAYER )
			{
				return COL_VTYPE_INTERNAL;
			}

			return COL_VTYPE_NOT_VALID;
		}
		case TYPE_BALL:
		{
			return COL_VTYPE_NOT_VALID;
		}
		default:
		{
			break;
		}
	}

	if( obj->m_massive_type == MASS_MASSIVE )
	{
		if( obj->m_sprite_array == ARRAY_ENEMY && m_origin_array == ARRAY_ENEMY )
		{
			return COL_VTYPE_NOT_VALID;
		}

		return COL_VTYPE_BLOCKING;
	}
	else if( obj->m_massive_type == MASS_HALFMASSIVE )
	{
		// if moving downwards and the object is on bottom
		if( m_vely >= 0.0f && Is_On_Top( obj ) )
		{
			return COL_VTYPE_BLOCKING;
		}
	}

	return COL_VTYPE_NOT_VALID;
}

void cBall :: Handle_Collision( cObjectCollision *collision )
{
	// already destroyed
	if( m_auto_destroy )
	{
		return;
	}

	cAnimated_Sprite::Handle_Collision( collision );
}

void cBall :: Handle_Collision_Player( cObjectCollision *collision )
{
	// velocity hit
	if( collision->m_direction == DIR_LEFT )
	{
		if( pLevel_Player->m_velx > 0.0f )
		{
			pLevel_Player->m_velx *= 0.3f;
		}
	}
	else if( collision->m_direction == DIR_RIGHT )
	{
		if( pLevel_Player->m_velx < 0.0f )
		{
			pLevel_Player->m_velx *= 0.3f;
		}
	}
	else if( collision->m_direction == DIR_UP )
	{
		if( pLevel_Player->m_vely > 0.0f )
		{
			pLevel_Player->m_vely *= 0.2f;
		}
	}
	else if( collision->m_direction == DIR_DOWN )
	{
		if( pLevel_Player->m_vely < 0.0f )
		{
			pLevel_Player->m_vely *= 0.4f;
		}
	}

	pAudio->Play_Sound( "item/fireball_repelled.wav" );
	Destroy();
}

void cBall :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	cEnemy *enemy = static_cast<cEnemy *>(m_sprite_manager->Get_Pointer( collision->m_number ));

	// if enemy is not destroyable
	if( ( m_ball_type == FIREBALL_DEFAULT && enemy->m_fire_resistant ) || ( m_ball_type == ICEBALL_DEFAULT && enemy->m_ice_resistance >= 1 ) )
	{
		pAudio->Play_Sound( "item/fireball_repelled.wav" );
	}
	// destroy enemy
	else
	{
		// animation
		cParticle_Emitter *anim = new cParticle_Emitter( m_sprite_manager );
		anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
		anim->Set_Time_to_Live( 0.2f, 0.4f );
		anim->Set_Fading_Alpha( 1 );
		anim->Set_Fading_Size( 1 );
		anim->Set_Speed( 0.5f, 2.2f );
		anim->Set_Blending( BLEND_DRIVE );

		// enemy rect particle animation
		for( unsigned int w = 0; w < enemy->m_col_rect.m_w; w += 15 )
		{
			for( unsigned int h = 0; h < enemy->m_col_rect.m_h; h += 15 )
			{
				anim->Set_Pos( enemy->m_pos_x + w, enemy->m_pos_y + h );

				Color anim_color, anim_color_rand;
				if( m_ball_type == FIREBALL_DEFAULT )
				{
					anim_color = Color( static_cast<Uint8>(250), 170, 150 );
					anim_color_rand = Color( static_cast<Uint8>( rand() % 5 ), rand() % 85, rand() % 25, 0 );
				}
				else
				{
					anim_color = Color( static_cast<Uint8>(150), 150, 240 );
					anim_color_rand = Color( static_cast<Uint8>( rand() % 80 ), rand() % 80, rand() % 10, 0 );
				}
				anim->Set_Color( anim_color, anim_color_rand );
				anim->Emit();
			}
		}
		
		pActive_Animation_Manager->Add( anim );

		// play enemy kill sound
		pAudio->Play_Sound( enemy->m_kill_sound );

		if( m_ball_type == FIREBALL_DEFAULT )
		{
			// get points
			pHud_Points->Add_Points( enemy->m_kill_points, m_pos_x, m_pos_y, "", static_cast<Uint8>(255), 1 );

			// create goldpiece
			cMovingSprite *goldpiece = new cFGoldpiece( m_sprite_manager, enemy->m_col_rect.m_x, enemy->m_col_rect.m_y + enemy->m_col_rect.m_h, collision->m_direction );
			// set optimal position
			goldpiece->Col_Move( -( ( goldpiece->m_col_rect.m_w - enemy->m_col_rect.m_w ) / 2 ), -( goldpiece->m_col_pos.m_y + goldpiece->m_col_rect.m_h ), 1, 1 );
			// add goldpiece
			m_sprite_manager->Add( goldpiece );

			enemy->Set_Active( 0 );
			enemy->DownGrade( 1 );
			pLevel_Player->Add_Kill_Multiplier();
		}
		else if( m_ball_type == ICEBALL_DEFAULT )
		{
			enemy->Freeze();
		}
	}

	Destroy();
}

void cBall :: Handle_Collision_Massive( cObjectCollision *collision )
{
	if( collision->m_direction == DIR_DOWN )
	{
		// if directly hitting the ground
		if( m_velx < 0.1f && m_velx > -0.1f )
		{
			Destroy_Ball( 1 );
			return;
		}

		if( m_ball_type == FIREBALL_DEFAULT )
		{
			m_vely = -10.0f;	
			
			// create animation
			cAnimation_Fireball *anim = new cAnimation_Fireball( m_sprite_manager, m_pos_x + m_col_rect.m_w / 2, m_pos_y + m_col_rect.m_h / 2 );
			anim->Set_Fading_Speed( 3.0f );
			pActive_Animation_Manager->Add( anim );
		}
		else if( m_ball_type == ICEBALL_DEFAULT )
		{
			m_vely = -5.0f;

			// create animation
			cParticle_Emitter *anim = new cParticle_Emitter( m_sprite_manager );
			anim->Set_Pos( m_pos_x + m_col_rect.m_w / 2, m_pos_y + m_col_rect.m_h / 2, 1 );
			anim->Set_Image( pVideo->Get_Surface( "animation/particles/cloud.png" ) );
			anim->Set_Direction_Range( 0, 180 );
			anim->Set_Quota( 3 );
			anim->Set_Time_to_Live( 0.8f );
			anim->Set_Pos_Z( m_pos_z + 0.0001f );
			anim->Set_Color( Color( static_cast<Uint8>(50), 50, 250 ) );
			anim->Set_Blending( BLEND_ADD );
			anim->Set_Speed( 0.5f, 0.4f );
			anim->Set_Scale( 0.3f, 0.4f );
			anim->Emit();
			pActive_Animation_Manager->Add( anim );
		}
	}
	// other directions
	else
	{
		Destroy_Ball( 1 );
	}
}

void cBall :: Handle_out_of_Level( ObjectDirection dir )
{
	// ignore top
	if( dir == DIR_TOP )
	{
		return;
	}

	Destroy_Ball( 1 );
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
