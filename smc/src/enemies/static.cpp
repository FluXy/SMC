/***************************************************************************
 * static.cpp  -  static enemy
 *
 * Copyright (C) 2007 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../enemies/static.h"
#include "../core/game_core.h"
#include "../level/level_player.h"
#include "../level/level.h"
#include "../gui/hud.h"
#include "../video/gl_surface.h"
#include "../core/sprite_manager.h"
#include "../core/i18n.h"
#include "../objects/path.h"
#include "../core/filesystem/filesystem.h"
// CEGUI
#include "CEGUIXMLAttributes.h"
#include "CEGUIWindowManager.h"
#include "elements/CEGUIEditbox.h"
#include "elements/CEGUICombobox.h"
#include "elements/CEGUIListboxTextItem.h"

namespace SMC
{

/* *** *** *** *** *** *** cStaticEnemy *** *** *** *** *** *** *** *** *** *** *** */

cStaticEnemy :: cStaticEnemy( cSprite_Manager *sprite_manager )
: cEnemy( sprite_manager ), m_path_state( sprite_manager )
{
	cStaticEnemy::Init();
}

cStaticEnemy :: cStaticEnemy( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager )
: cEnemy( sprite_manager ), m_path_state( sprite_manager )
{
	cStaticEnemy::Init();
	cStaticEnemy::Load_From_XML( attributes );
}

cStaticEnemy :: ~cStaticEnemy( void )
{
	//
}

void cStaticEnemy :: Init( void )
{
	m_type = TYPE_STATIC_ENEMY;
	m_pos_z = 0.094f;
	m_can_be_on_ground = 0;
	m_can_be_hit_from_shell = 0;

	Set_Rotation_Speed( 0.0f );
    Set_Speed( 0.0f );
	Set_Static_Image( "enemy/static/blocks/spike_1/2_grey.png" );
	Create_Name();
}

void cStaticEnemy :: Init_Links( void )
{
	// link to parent path
	m_path_state.Set_Path_Identifier( m_path_state.m_path_identifier );
}

cStaticEnemy *cStaticEnemy :: Copy( void ) const
{
	cStaticEnemy *static_enemy = new cStaticEnemy( m_sprite_manager );
	static_enemy->Set_Pos( m_start_pos_x, m_start_pos_y, 1 );
	static_enemy->Set_Static_Image( m_img_filename );
	static_enemy->Set_Rotation_Speed( m_rotation_speed );
    static_enemy->Set_Path_Identifier( m_path_state.m_path_identifier );
    static_enemy->Set_Speed( m_speed );
	static_enemy->m_fire_resistant = m_fire_resistant;
	static_enemy->m_ice_resistance = m_ice_resistance;
	return static_enemy;
}

void cStaticEnemy :: Load_From_XML( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// rotation speed
	Set_Rotation_Speed( static_cast<float>( attributes.getValueAsFloat( "rotation_speed", -7.5f ) ) );
	// image
	Set_Static_Image( attributes.getValueAsString( "image", "enemy/static/saw/default.png" ).c_str() );
    // path
    Set_Path_Identifier( attributes.getValueAsString( "path", "" ).c_str() );
    // movement speed
    Set_Speed( static_cast<float>( attributes.getValueAsFloat( "speed", m_speed ) ) );
	// fire resistant
	m_fire_resistant = attributes.getValueAsBool( "fire_resistant", m_fire_resistant );
	// ice resistance
	m_ice_resistance = static_cast<float>( attributes.getValueAsFloat( "ice_resistance", m_ice_resistance ) );
}

void cStaticEnemy :: Save_To_XML( CEGUI::XMLSerializer &stream )
{
	// begin
	stream.openTag( m_type_name );

	// name
	Write_Property( stream, "type", "static" );
	// position
	Write_Property( stream, "posx", static_cast<int>( m_start_pos_x ) );
	Write_Property( stream, "posy", static_cast<int>( m_start_pos_y ) );
	// rotation speed
	Write_Property( stream, "rotation_speed", m_rotation_speed );
	// image
	Write_Property( stream, "image", m_img_filename );
    // path
	Write_Property( stream, "path", m_path_state.m_path_identifier );
    // speed
    Write_Property( stream, "speed", m_speed );
	// fire resistant
	Write_Property( stream, "fire_resistant", m_fire_resistant );
	// ice resistance
	Write_Property( stream, "ice_resistance", m_ice_resistance );

	// end
	stream.closeTag();
}

void cStaticEnemy :: Set_Sprite_Manager( cSprite_Manager *sprite_manager )
{
	cSprite::Set_Sprite_Manager( sprite_manager );
	m_path_state.Set_Sprite_Manager( sprite_manager );
}

void cStaticEnemy :: Load_From_Savegame( cSave_Level_Object *save_object )
{
	cEnemy::Load_From_Savegame( save_object );
	m_path_state.Load_From_Savegame( save_object );
}

cSave_Level_Object *cStaticEnemy :: Save_To_Savegame( void )
{
	cSave_Level_Object *save_object = cEnemy::Save_To_Savegame();
	m_path_state.Save_To_Savegame( save_object );

	return save_object;
}

void cStaticEnemy :: Set_Rotation_Speed( float speed )
{
	m_rotation_speed = speed;
}

void cStaticEnemy :: Set_Static_Image( const std::string &filename )
{
	if( filename.empty() || ( !File_Exists( filename ) && !File_Exists( DATA_DIR "/" GAME_PIXMAPS_DIR "/" + filename ) ) )
	{
		return;
	}

	Clear_Images();

	m_img_filename = filename;

	// remove pixmaps dir
	if( m_img_filename.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == 0 )
	{
		m_img_filename.erase( 0, strlen( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) );
	}

	Add_Image( pVideo->Get_Surface( filename ) );
	Set_Image_Num( 0, 1 );
	Create_Name();
}

void cStaticEnemy :: Set_Path_Identifier( const std::string &path )
{
    m_path_state.Set_Path_Identifier( path );
	Set_Velocity( 0.0f, 0.0f );
}

void cStaticEnemy :: Set_Speed( float speed )
{
    m_speed = speed;
}

void cStaticEnemy :: DownGrade( bool force /* = 0 */ )
{
	Set_Dead( 1 );
	m_massive_type = MASS_PASSIVE;
	m_counter = 0.0f;
	m_velx = 0.0f;
	m_vely = 0.0f;
	Set_Scale_Directions( 1, 1, 1, 1 );

	// falling death
	Set_Rotation_Z( 180.0f );
}

void cStaticEnemy :: Update_Dying( void )
{
	m_counter += pFramerate->m_speed_factor * 0.1f;

	// falling death

	// a little bit upwards first
	if( m_counter < 0.3f )
	{
		Move( 0.0f, -5.0f );
	}
	// if not below the ground : fall
	else if( m_col_rect.m_y < pActive_Camera->m_limit_rect.m_y )
	{
		Move( 0.0f, 20.0f );
		Add_Scale( -pFramerate->m_speed_factor * 0.01f );
	}
	// if below disable
	else
	{
		m_rot_z = 0.0f;
		Set_Scale( 1.0f );
		Set_Active( 0 );
	}
}

void cStaticEnemy :: Update( void )
{
	cEnemy::Update();

	if( !m_valid_update || !Is_In_Range() )
	{
		return;
	}

	if( m_rotation_speed )
	{
		// update rotation
		Add_Rotation_Z( m_rotation_speed * pFramerate->m_speed_factor );
	}

    if( m_path_state.m_path )
    {
		// move along path
        if( m_path_state.Path_Move( m_speed * pFramerate->m_speed_factor ) == 0 )
        {
			if( !m_path_state.m_path->m_rewind )
			{
				// if we can not move further along the path, reverse the direction
				m_path_state.Move_Toggle();
			}
        }

    	// get difference
		float diff_x = ( m_path_state.m_path->m_start_pos_x + m_path_state.m_pos_x ) - m_pos_x;
		float diff_y = ( m_path_state.m_path->m_start_pos_y + m_path_state.m_pos_y ) - m_pos_y;

		// move to position
        Set_Velocity( diff_x, diff_y );
    }
}

void cStaticEnemy :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( !m_valid_draw )
	{
		return;
	}

	// current path state position
	if( game_debug )
	{
		m_path_state.Draw();
	}

	// enemy
	cEnemy::Draw( request );
}

bool cStaticEnemy :: Is_Update_Valid( void )
{
	if( m_dead || m_freeze_counter )
	{
		return 0;
	}

	return 1;
}

Col_Valid_Type cStaticEnemy :: Validate_Collision( cSprite *obj )
{
	if( obj->m_massive_type == MASS_MASSIVE )
	{
		switch( obj->m_type )
		{
			case TYPE_ROKKO:
			{
				return COL_VTYPE_NOT_VALID;
			}
			case TYPE_GEE:
			{
				return COL_VTYPE_NOT_VALID;
			}
			case TYPE_TURTLE_BOSS:
			{
				return COL_VTYPE_NOT_VALID;
			}
			case TYPE_FURBALL_BOSS:
			{
				return COL_VTYPE_NOT_VALID;
			}
			case TYPE_SPIKEBALL:
			{
				return COL_VTYPE_NOT_VALID;
			}
			case TYPE_PLAYER:
			{
				return COL_VTYPE_INTERNAL;
			}
			default:
			{
				break;
			}
		}

		if( obj->m_sprite_array == ARRAY_ENEMY )
		{
			return COL_VTYPE_INTERNAL;
		}

		return COL_VTYPE_NOT_VALID;
	}

	return COL_VTYPE_NOT_VALID;
}

void cStaticEnemy :: Handle_Collision_Player( cObjectCollision *collision )
{
	pLevel_Player->DownGrade_Player();
}

void cStaticEnemy :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	// invalid
	if( collision->m_number < 0 )
	{
		return;
	}

	cEnemy *enemy = static_cast<cEnemy *>(m_sprite_manager->Get_Pointer( collision->m_number ));

	// already dead
	if( enemy->m_dead )
	{
		return;
	}

	// kill enemy
	pAudio->Play_Sound( enemy->m_kill_sound );
	pHud_Points->Add_Points( enemy->m_kill_points, m_pos_x, m_pos_y - 5.0f, "", static_cast<Uint8>(255), 1 );
	enemy->DownGrade( 1 );
}

void cStaticEnemy :: Editor_Activate( void )
{
	// get window manager
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// image
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_static_enemy_image" ));
	Editor_Add( UTF8_("Image"), UTF8_("Image filename"), editbox, 200 );

	editbox->setText( m_img_filename.c_str() );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cStaticEnemy::Editor_Image_Text_Changed, this ) );

	// rotation speed
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_static_enemy_rotation_speed" ));
	Editor_Add( UTF8_("Rotation Speed"), UTF8_("Rotation Speed"), editbox, 120 );

	editbox->setValidationString( "[-+]?[0-9]*\\.?[0-9]*" );
	editbox->setText( float_to_string( m_rotation_speed, 6, 0 ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cStaticEnemy::Editor_Rotation_Speed_Text_Changed, this ) );

	// path
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_static_enemy_path_identifier" ));
	Editor_Add( UTF8_("Path Identifier"), UTF8_("Name of the Path to move along."), editbox, 120 );

	editbox->setText( m_path_state.m_path_identifier.c_str() );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cStaticEnemy::Editor_Path_Identifier_Text_Changed, this ) );

	// speed
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_static_enemy_speed" ));
	Editor_Add( UTF8_("Speed"), UTF8_("Maximum speed"), editbox, 120 );

	editbox->setValidationString( "[-+]?[0-9]*\\.?[0-9]*" );
	editbox->setText( float_to_string( m_speed, 6, 0 ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cStaticEnemy::Editor_Speed_Text_Changed, this ) );

	// fire resistant
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_static_enemy_fire_resistant" ));
	Editor_Add( UTF8_("Fire Resistant"), UTF8_("If it is resistant against fire"), combobox, 120, 80 );

	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Enabled") ) );
	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Disabled") ) );

	if( m_fire_resistant )
	{
		combobox->setText( UTF8_("Enabled") );
	}
	else
	{
		combobox->setText( UTF8_("Disabled") );
	}

	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cStaticEnemy::Editor_Fire_Resistant_Select, this ) );

	// ice resistance
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_static_enemy_ice_resistance" ));
	Editor_Add( UTF8_("Ice Resistance"), UTF8_("Resistance against Ice (0.0-1.0)"), editbox, 120 );

	editbox->setValidationString( "[+]?[0-9]*\\.?[0-9]*" );
	editbox->setText( float_to_string( m_ice_resistance, 6, 0 ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cStaticEnemy::Editor_Ice_Resistance_Text_Changed, this ) );
    
	// init
	Editor_Init();
}

bool cStaticEnemy :: Editor_Image_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Static_Image( str_text );

	return 1;
}

bool cStaticEnemy :: Editor_Rotation_Speed_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Rotation_Speed( string_to_float( str_text ) );

	return 1;
}

bool cStaticEnemy :: Editor_Path_Identifier_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Path_Identifier( str_text );

	return 1;
}

bool cStaticEnemy :: Editor_Speed_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Speed( string_to_float( str_text ) );

	return 1;
}

bool cStaticEnemy :: Editor_Fire_Resistant_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	if( item->getText().compare( UTF8_("Enabled") ) == 0 )
	{
		m_fire_resistant = 1;
	}
	else
	{
		m_fire_resistant = 0;
	}

	return 1;
}

bool cStaticEnemy :: Editor_Ice_Resistance_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	m_ice_resistance = string_to_float( str_text );

	if( m_ice_resistance > 1.0f )
	{
		m_ice_resistance = 1.0f;
	}
	
	return 1;
}

void cStaticEnemy :: Create_Name( void )
{
	m_name = "Static Enemy";

	if( m_start_image && !m_start_image->m_name.empty() )
	{
		m_name += " " + m_start_image->m_name;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
