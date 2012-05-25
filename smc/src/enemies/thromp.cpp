/***************************************************************************
 * thromp.cpp  -  falling stone
 *
 * Copyright (C) 2006 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../enemies/thromp.h"
#include "../core/game_core.h"
#include "../video/animation.h"
#include "../level/level_player.h"
#include "../level/level.h"
#include "../gui/hud.h"
#include "../video/gl_surface.h"
#include "../user/savegame.h"
#include "../core/sprite_manager.h"
#include "../input/mouse.h"
#include "../core/i18n.h"
#include "../core/filesystem/filesystem.h"
// CEGUI
#include "CEGUIXMLAttributes.h"
#include "CEGUIWindowManager.h"
#include "elements/CEGUICombobox.h"
#include "elements/CEGUIListboxTextItem.h"
#include "elements/CEGUIEditbox.h"

namespace SMC
{

/* *** *** *** *** *** *** cThromp *** *** *** *** *** *** *** *** *** *** *** */

cThromp :: cThromp( cSprite_Manager *sprite_manager )
: cEnemy( sprite_manager )
{
	cThromp::Init();
}

cThromp :: cThromp( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager )
: cEnemy( sprite_manager )
{
	cThromp::Init();
	cThromp::Load_From_XML( attributes );
}

cThromp :: ~cThromp( void )
{
	//
}

void cThromp :: Init( void  )
{
	m_type = TYPE_THROMP;
	m_pos_z = 0.093f;
	m_camera_range = 1000;
	m_can_be_on_ground = 0;
	m_fire_resistant = 1;

	m_state = STA_STAY;
	m_move_back = 0;
	m_dest_velx = 0;
	m_dest_vely = 0;
	m_img_dir = "enemy/thromp/";
	Set_Direction( DIR_DOWN );
	Set_Speed( 7 );
	Set_Max_Distance( 200 );

	m_kill_sound = "enemy/thromp/die.ogg";
	m_kill_points = 200;
}

cThromp *cThromp :: Copy( void ) const
{
	cThromp *thromp = new cThromp( m_sprite_manager );
	thromp->Set_Pos( m_start_pos_x, m_start_pos_y );
	thromp->Set_Image_Dir( m_img_dir );
	thromp->Set_Direction( m_start_direction );
	thromp->Set_Max_Distance( m_max_distance );
	thromp->Set_Speed( m_speed );
	return thromp;
}

void cThromp :: Load_From_XML( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// image directory
	Set_Image_Dir( attributes.getValueAsString( "image_dir", m_img_dir ).c_str() );
	// direction
	Set_Direction( Get_Direction_Id( attributes.getValueAsString( "direction", Get_Direction_Name( m_start_direction ) ).c_str() ) );
	// max distance
	Set_Max_Distance( static_cast<float>(attributes.getValueAsInteger( "max_distance", static_cast<int>(m_max_distance) )) );
	// speed
	Set_Speed( attributes.getValueAsFloat( "speed", m_speed ) );
}

void cThromp :: Save_To_XML( CEGUI::XMLSerializer &stream )
{
	// begin
	stream.openTag( m_type_name );

	// name
	Write_Property( stream, "type", "thromp" );
	// position
	Write_Property( stream, "posx", static_cast<int>( m_start_pos_x ) );
	Write_Property( stream, "posy", static_cast<int>( m_start_pos_y ) );
	// image directory
	Write_Property( stream, "image_dir", m_img_dir );
	// direction
	Write_Property( stream, "direction", Get_Direction_Name( m_start_direction ) );
	// max distance
	Write_Property( stream, "max_distance", static_cast<int>(m_max_distance) );
	// speed
	Write_Property( stream, "speed", m_speed );

	// end
	stream.closeTag();
}

void cThromp :: Load_From_Savegame( cSave_Level_Object *save_object )
{
	cEnemy::Load_From_Savegame( save_object );

	// Don't activate if dead
	if( m_dead )
	{
		return;
	}

	// move_back
	if( save_object->exists( "move_back" ) )
	{
		m_move_back = string_to_int( save_object->Get_Value( "move_back" ) ) > 0;
	}
}

cSave_Level_Object *cThromp :: Save_To_Savegame( void )
{
	cSave_Level_Object *save_object = cEnemy::Save_To_Savegame();

	// move_back ( only save if needed )
	if( m_move_back )
	{
		save_object->m_properties.push_back( cSave_Level_Object_Property( "move_back", int_to_string( m_move_back ) ) );
	}

	return save_object;
}

void cThromp :: Set_Image_Dir( std::string dir )
{
	if( dir.empty() )
	{
		return;
	}

	// remove pixmaps dir
	if( dir.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == 0 )
	{
		dir.erase( 0, strlen( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) );
	}

	// add trailing slash if missing
	if( *(dir.end() - 1) != '/' )
	{
		dir.insert( dir.length(), "/" );
	}

	// if not image directory
	if( !File_Exists( DATA_DIR "/" GAME_PIXMAPS_DIR "/" + dir + "up.png" ) && !File_Exists( DATA_DIR "/" GAME_PIXMAPS_DIR "/" + dir + "up.settings" ) )
	{
		printf( "Warning : Thromp image dir does not exist %s\n", dir.c_str() );
		return;
	}

	m_img_dir = dir;

	Update_Images();
}

void cThromp :: Set_Direction( const ObjectDirection dir )
{
	// already set
	if( m_start_direction == dir )
	{
		return;
	}

	cEnemy::Set_Direction( dir, 1 );

	Update_Distance_Rect();
	Update_Dest_Vel();
	Update_Images();
}

void cThromp :: Set_Max_Distance( float nmax_distance )
{
	m_max_distance = nmax_distance;

	if( m_max_distance < 0 )
	{
		m_max_distance = 0;
	}

	Update_Distance_Rect();
}

void cThromp :: Set_Speed( float val )
{
	if( val < 0.1f )
	{
		val = 0.1f;
	}

	m_speed = val;

	Update_Dest_Vel();
}

void cThromp :: Activate( void )
{
	if( m_state == STA_FLY )
	{
		return;
	}

	m_state = STA_FLY;

	m_velx = m_dest_velx;
	m_vely = m_dest_vely;
	m_move_back = 0;

	// active image
	Set_Image_Num( 1 );
}

bool cThromp :: Move_Back( void )
{
	if( m_state == STA_STAY || m_move_back )
	{
		return 0;
	}

	m_velx = -m_dest_velx * 0.01f;
	m_vely = -m_dest_vely * 0.01f;

	m_move_back = 1;

	// default image
	Set_Image_Num( 0 );

	return 1;
}

void cThromp :: DownGrade( bool force /* = 0 */ )
{
	Set_Dead( 1 );
	m_massive_type = MASS_PASSIVE;
	m_counter = 0.0f;
	m_velx = 0.0f;
	m_vely = 0.0f;

	if( !force )
	{
		// animation
		cParticle_Emitter *anim = new cParticle_Emitter( m_sprite_manager );
		Generate_Hit_Animation( anim );

		anim->Set_Scale( 0.8f );
		anim->Set_Direction_Range( 0.0f, 360.0f );
		anim->Emit();
		pActive_Animation_Manager->Add( anim );
	}
	else
	{
		Set_Rotation_Z( 180.0f );
	}
}

void cThromp :: Update_Dying( void )
{
	m_counter += pFramerate->m_speed_factor;

	// default death
	if( !Is_Float_Equal( m_rot_z, 180.0f ) )
	{
		Set_Active( 0 );
	}
	// falling death
	else
	{
		// a little bit upwards first
		if( m_counter < 5.0f )
		{
			Move( 0.0f, -5.0f );
		}
		// if not below the ground : fall
		else if( m_col_rect.m_y < pActive_Camera->m_limit_rect.m_y )
		{
			Move( 0.0f, 20.0f );
		}
		// if below disable
		else
		{
			m_rot_z = 0.0f;
			Set_Active( 0 );
		}
	}
}

void cThromp :: Update( void )
{
	cEnemy::Update();

	if( !m_valid_update || !Is_In_Range() )
	{
		return;
	}

	// standing ( waiting )
	if( m_state == STA_STAY )
	{
		GL_rect final_distance = Get_Final_Distance_Rect();

		// if player is in front then activate
		if( pLevel_Player->m_maryo_type != MARYO_GHOST && pLevel_Player->m_col_rect.Intersects( final_distance ) )
		{
			Activate();
		}
	}
	// flying ( moving into the destination direction )
	else
	{
		// distance to final position
		float dist_to_final_pos = 0;
		// multiplier for the minimal velocity
		float vel_mod_min = 1;

		/* slow down
		 * only if the velocity is not too small for the given distance to the final position
		 * final velocity should not get smaller on the last 10% to the final position
		*/
		if( m_direction == DIR_LEFT )
		{
			dist_to_final_pos = m_max_distance - ( m_start_pos_x - m_pos_x );

			// move back
			if( m_move_back )
			{
				vel_mod_min = ( dist_to_final_pos + ( m_max_distance * 0.1f ) ) / m_max_distance;
				if( -m_velx > m_dest_velx * vel_mod_min )
				{
					m_velx *= 1 + ( 0.2f * pFramerate->m_speed_factor );
				}
			}

		}
		else if( m_direction == DIR_RIGHT )
		{
			dist_to_final_pos = m_max_distance + ( m_start_pos_x - m_pos_x );

			// move back
			if( m_move_back )
			{
				vel_mod_min = ( dist_to_final_pos + ( m_max_distance * 0.1f ) ) / m_max_distance;
				if( -m_velx < m_dest_velx * vel_mod_min )
				{
					m_velx *= 1 + ( 0.2f * pFramerate->m_speed_factor );
				}
			}
		}
		else if( m_direction == DIR_UP )
		{
			dist_to_final_pos = m_max_distance - ( m_start_pos_y - m_pos_y );

			// move back
			if( m_move_back )
			{
				vel_mod_min = ( dist_to_final_pos + ( m_max_distance * 0.1f ) ) / m_max_distance;
				if( -m_vely > m_dest_vely * vel_mod_min )
				{
					m_vely *= 1 + ( 0.2f * pFramerate->m_speed_factor );
				}
			}
		}
		else if( m_direction == DIR_DOWN )
		{
			dist_to_final_pos = m_max_distance + ( m_start_pos_y - m_pos_y );

			// move back
			if( m_move_back )
			{
				vel_mod_min = ( dist_to_final_pos + ( m_max_distance * 0.1f ) ) / m_max_distance;
				if( -m_vely < m_dest_vely * vel_mod_min )
				{
					m_vely *= 1 + ( 0.2f * pFramerate->m_speed_factor );
				}
			}
		}

		// reached final position move back
		if( !m_move_back && dist_to_final_pos < 0.0f )
		{
			Move_Back();
		}
		// reached original position
		else if( m_move_back && dist_to_final_pos > m_max_distance )
		{
			m_state = STA_STAY;
			Set_Pos( m_start_pos_x, m_start_pos_y );
			// unset velocity
			Set_Velocity( 0, 0 );

			m_move_back = 0;
		}
	}
}

void cThromp :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( !m_valid_draw )
	{
		return;
	}

	// draw distance rect
	if( editor_level_enabled )
	{
		GL_rect final_distance = Get_Final_Distance_Rect();
		final_distance.m_x -= pActive_Camera->m_x;
		final_distance.m_y -= pActive_Camera->m_y;

		pVideo->Draw_Rect( &final_distance, m_pos_z - 0.000001f, &whitealpha128 );
	}

	cEnemy::Draw( request );
}

void cThromp :: Update_Images( void )
{
	// clear images
	Clear_Images();
	// set images
	Add_Image( pVideo->Get_Surface( m_img_dir + Get_Direction_Name( m_start_direction ) + ".png" ) );
	Add_Image( pVideo->Get_Surface( m_img_dir + Get_Direction_Name( m_start_direction ) + "_active.png" ) );
	// set start image
	Set_Image_Num( 0, 1 );

	// set active image
	if( m_state == STA_FLY )
	{
		Set_Image_Num( 1 );
	}

	Create_Name();
}

void cThromp :: Update_Dest_Vel( void )
{
	if( m_start_direction == DIR_UP )
	{
		m_dest_velx = 0;
		m_dest_vely = -m_speed;
	}
	else if( m_start_direction == DIR_DOWN )
	{
		m_dest_velx = 0;
		m_dest_vely = m_speed;
	}
	else if( m_start_direction == DIR_LEFT )
	{
		m_dest_velx = -m_speed;
		m_dest_vely = 0;
	}
	else if( m_start_direction == DIR_RIGHT )
	{
		m_dest_velx = m_speed;
		m_dest_vely = 0;
	}
	else
	{
		m_dest_velx = 0;
		m_dest_vely = 0;
	}
}

void cThromp :: Update_Distance_Rect( void )
{
	if( m_start_direction == DIR_UP )
	{
		m_distance_rect.m_x = m_col_pos.m_x;
		m_distance_rect.m_y = -m_max_distance;
		m_distance_rect.m_w = m_col_rect.m_w;
		m_distance_rect.m_h = m_max_distance;
	}
	else if( m_start_direction == DIR_DOWN )
	{
		m_distance_rect.m_x = m_col_pos.m_x;
		m_distance_rect.m_y = 0;
		m_distance_rect.m_w = m_col_rect.m_w;
		m_distance_rect.m_h = m_max_distance;
	}
	else if( m_start_direction == DIR_LEFT )
	{
		m_distance_rect.m_x = -m_max_distance;
		m_distance_rect.m_y = m_col_pos.m_y;
		m_distance_rect.m_w = m_max_distance;
		m_distance_rect.m_h = m_col_rect.m_h;
	}
	else if( m_start_direction == DIR_RIGHT )
	{
		m_distance_rect.m_x = 0;
		m_distance_rect.m_y = m_col_pos.m_y;
		m_distance_rect.m_w = m_max_distance;
		m_distance_rect.m_h = m_col_rect.m_h;
	}
}

GL_rect cThromp :: Get_Final_Distance_Rect( void ) const
{
	GL_rect final_distance = m_distance_rect;

	final_distance.m_x += m_rect.m_x;
	final_distance.m_y += m_rect.m_y;

	if( m_start_direction == DIR_LEFT || m_start_direction == DIR_RIGHT )
	{
		final_distance.m_x += m_rect.m_w;
		final_distance.m_w -= m_rect.m_w;
	}
	else if( m_start_direction == DIR_UP || m_start_direction == DIR_DOWN )
	{
		final_distance.m_y += m_rect.m_h;
		final_distance.m_h -= m_rect.m_h;
	}

	return final_distance;
}

bool cThromp :: Is_Update_Valid( void )
{
	if( m_dead || m_freeze_counter )
	{
		return 0;
	}

	return 1;
}

bool cThromp :: Is_Draw_Valid( void )
{
	bool valid = cEnemy::Is_Draw_Valid();

	// if editor enabled
	if( editor_enabled )
	{
		// if active mouse object
		if( pMouseCursor->m_active_object == this )
		{
			return 1;
		}
	}

	return valid;
}

void cThromp :: Generate_Smoke( unsigned int amount /* = 20 */ ) const
{
	// smoke on the destination direction
	float smoke_x;
	float smoke_y;
	float smoke_width;
	float smoke_height;

	if( m_direction == DIR_DOWN )
	{
		smoke_x = m_pos_x;
		smoke_y = m_pos_y + m_rect.m_h - 5;
		smoke_width = m_col_rect.m_w;
		smoke_height = 1;
	}
	else if( m_direction == DIR_UP )
	{
		smoke_x = m_pos_x;
		smoke_y = m_pos_y + 5;
		smoke_width = m_col_rect.m_w;
		smoke_height = 1;
	}
	else if( m_direction == DIR_LEFT )
	{
		smoke_x = m_pos_x + 5;
		smoke_y = m_pos_y;
		smoke_width = 1;
		smoke_height = m_col_rect.m_h;
	}
	else if( m_direction == DIR_RIGHT )
	{
		smoke_x = m_pos_x + m_rect.m_w - 5;
		smoke_y = m_pos_y;
		smoke_width = 1;
		smoke_height = m_col_rect.m_h;
	}
	else
	{
		return;
	}

	// animation
	cParticle_Emitter *anim = new cParticle_Emitter( m_sprite_manager );
	anim->Set_Emitter_Rect( smoke_x, smoke_y, smoke_width, smoke_height );
	anim->Set_Image( pVideo->Get_Surface( "animation/particles/smoke.png" ) );
	anim->Set_Quota( amount );
	anim->Set_Pos_Z( m_pos_z + 0.000001f );
	anim->Set_Time_to_Live( 1, 1 );
	anim->Set_Direction_Range( 180, 180 );
	anim->Set_Speed( 0.05f, 0.4f );
	anim->Set_Fading_Alpha( 1 );
	anim->Set_Const_Rotation_Z( -2, 4 );
	anim->Emit();
	pActive_Animation_Manager->Add( anim );
}

Col_Valid_Type cThromp :: Validate_Collision( cSprite *obj )
{
	// basic validation checking
	Col_Valid_Type basic_valid = Validate_Collision_Ghost( obj );

	// found valid collision
	if( basic_valid != COL_VTYPE_NOT_POSSIBLE )
	{
		return basic_valid;
	}

	if( obj->m_type == TYPE_PLAYER || obj->m_sprite_array == ARRAY_ENEMY )
	{
		cMovingSprite *moving_sprite = static_cast<cMovingSprite *>(obj);

		Col_Valid_Type validation = Validate_Collision_Object_On_Top( moving_sprite );

		if( validation != COL_VTYPE_NOT_POSSIBLE )
		{
			return validation;
		}

		// massive
		if( m_massive_type == MASS_MASSIVE )
		{
			return COL_VTYPE_INTERNAL;
		}

		// if moving back collide with nothing
		if( m_move_back )
		{
			return COL_VTYPE_INTERNAL;
		}
		else
		{
			return COL_VTYPE_BLOCKING;
		}
	}
	// massive
	if( obj->m_massive_type == MASS_MASSIVE )
	{
		if( obj->m_type == TYPE_STATIC_ENEMY )
		{
			return COL_VTYPE_NOT_VALID;
		}

		return COL_VTYPE_INTERNAL;
	}
	if( obj->m_massive_type == MASS_HALFMASSIVE )
	{
		// if moving downwards and the object is on bottom
		if( m_vely >= 0 && Is_On_Top( obj ) )
		{
			// if moving back collide with nothing
			if( !m_move_back )
			{
				return COL_VTYPE_BLOCKING;
			}
		}
	}

	return COL_VTYPE_NOT_VALID;
}

void cThromp :: Handle_Collision_Player( cObjectCollision *collision )
{
	// front
	if( collision->m_direction == m_direction )
	{
		pLevel_Player->DownGrade_Player();

		if( Move_Back() )
		{
			pAudio->Play_Sound( "enemy/thromp/hit.ogg" );
			Generate_Smoke();
		}
	}
	else
	{
		Handle_Move_Object_Collision( collision );
	}
	// left/right of front direction doesn't harm
}

void cThromp :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	// destination direction collision
	if( collision->m_direction == m_direction )
	{
		// if active
		if( m_state == STA_FLY )
		{
			cEnemy *enemy = static_cast<cEnemy *>(m_sprite_manager->Get_Pointer( collision->m_number ));

			// enemies that only hit us
			if( enemy->m_type == TYPE_SPIKEBALL )
			{
				// hits us
				DownGrade( 1 );
			}
			// kill enemy
			else
			{
				pAudio->Play_Sound( enemy->m_kill_sound );
				pHud_Points->Add_Points( enemy->m_kill_points, m_pos_x + m_image->m_w / 3, m_pos_y - 5, "", static_cast<Uint8>(255), 1 );
				enemy->DownGrade( 1 );

				if( !m_move_back )
				{
					Generate_Smoke();
				}
			}

		}
	}
	else
	{
		Handle_Move_Object_Collision( collision );
	}
}

void cThromp :: Handle_Collision_Massive( cObjectCollision *collision )
{
	Send_Collision( collision );

	cSprite *col_obj = m_sprite_manager->Get_Pointer( collision->m_number );

	// ignore ball
	if( col_obj->m_type == TYPE_BALL )
	{
		return;
	}

	if( Move_Back() )
	{
		pAudio->Play_Sound( "enemy/thromp/hit.ogg" );
		Generate_Smoke();
	}
}

void cThromp :: Handle_out_of_Level( ObjectDirection dir )
{
	if( Move_Back() )
	{
		pAudio->Play_Sound( "enemy/thromp/hit.ogg" );
		Generate_Smoke();
	}
}

void cThromp :: Editor_Activate( void )
{
	// get window manager
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// direction
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_thromp_direction" ));
	Editor_Add( UTF8_("Direction"), UTF8_("Direction it moves into."), combobox, 100, 110 );

	combobox->addItem( new CEGUI::ListboxTextItem( "up" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "down" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "left" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "right" ) );

	combobox->setText( Get_Direction_Name( m_start_direction ) );
	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cThromp::Editor_Direction_Select, this ) );

	// image dir
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_thromp_image_dir" ));
	Editor_Add( UTF8_("Image directory"), UTF8_("Directory containing the images"), editbox, 200 );

	editbox->setText( m_img_dir.c_str() );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cThromp::Editor_Image_Dir_Text_Changed, this ) );

	// max distance
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_thromp_max_distance" ));
	Editor_Add( UTF8_("Distance"), UTF8_("Detection distance into its direction"), editbox, 90 );

	editbox->setValidationString( "^[+]?\\d*$" );
	editbox->setText( int_to_string( static_cast<int>(m_max_distance) ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cThromp::Editor_Max_Distance_Text_Changed, this ) );

	// speed
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_thromp_speed" ));
	Editor_Add( UTF8_("Speed"), UTF8_("Speed when activated"), editbox, 120 );

	editbox->setValidationString( "[+]?[0-9]*\\.?[0-9]*" );
	editbox->setText( float_to_string( m_speed, 6, 0 ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cThromp::Editor_Speed_Text_Changed, this ) );

	// init
	Editor_Init();
}

bool cThromp :: Editor_Direction_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Direction( Get_Direction_Id( item->getText().c_str() ) );

	return 1;
}

bool cThromp :: Editor_Image_Dir_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Image_Dir( str_text );

	return 1;
}

bool cThromp :: Editor_Max_Distance_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Max_Distance( static_cast<float>(string_to_int( str_text )) );

	return 1;
}

bool cThromp :: Editor_Speed_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Speed( string_to_float( str_text ) );

	return 1;
}

void cThromp :: Create_Name( void )
{
	m_name = "Thromp ";
	m_name += _(Get_Direction_Name( m_start_direction ).c_str());

	if( m_start_image && !m_start_image->m_name.empty() )
	{
		m_name += " " + m_start_image->m_name;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
