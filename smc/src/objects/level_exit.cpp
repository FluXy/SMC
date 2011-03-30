/***************************************************************************
 * level_exit.cpp  -  area to exit the current level
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

#include "../objects/level_exit.h"
#include "../level/level_player.h"
#include "../core/game_core.h"
#include "../user/preferences.h"
#include "../audio/audio.h"
#include "../core/framerate.h"
#include "../core/main.h"
#include "../video/gl_surface.h"
#include "../video/font.h"
#include "../video/renderer.h"
#include "../level/level.h"
#include "../core/i18n.h"
#include "../core/filesystem/filesystem.h"
// CEGUI
#include "CEGUIXMLAttributes.h"
#include "CEGUIWindowManager.h"
#include "elements/CEGUIEditbox.h"
#include "elements/CEGUICombobox.h"
#include "elements/CEGUIListboxTextItem.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cLevel_Exit :: cLevel_Exit( cSprite_Manager *sprite_manager )
: cAnimated_Sprite( sprite_manager, "levelexit" )
{
	cLevel_Exit::Init();
}

cLevel_Exit :: cLevel_Exit( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager )
: cAnimated_Sprite( sprite_manager, "levelexit" )
{
	cLevel_Exit::Init();
	cLevel_Exit::Load_From_XML( attributes );
}

cLevel_Exit :: ~cLevel_Exit( void )
{
	if( m_editor_entry_name )
	{
		delete m_editor_entry_name;
		m_editor_entry_name = NULL;
	}
}

void cLevel_Exit :: Init( void )
{
	m_sprite_array = ARRAY_ACTIVE;
	m_type = TYPE_LEVEL_EXIT;
	m_massive_type = MASS_PASSIVE;
	m_editor_pos_z = 0.111f;
	m_camera_range = 1000;

	// size
	m_rect.m_w = 10;
	m_rect.m_h = 20;
	m_col_rect.m_w = m_rect.m_w;
	m_col_rect.m_h = m_rect.m_h;
	m_start_rect.m_w = m_rect.m_w;
	m_start_rect.m_h = m_rect.m_h;

	m_exit_type = LEVEL_EXIT_BEAM;
	m_exit_motion = CAMERA_MOVE_FLY;

	Set_Direction( DIR_DOWN );

	m_editor_color = red;
	m_editor_color.alpha = 128;

	m_editor_entry_name = NULL;
}

cLevel_Exit *cLevel_Exit :: Copy( void ) const
{
	cLevel_Exit *level_exit = new cLevel_Exit( m_sprite_manager );
	level_exit->Set_Pos( m_start_pos_x, m_start_pos_y, 1 );
	level_exit->Set_Type( m_exit_type );
	level_exit->Set_Camera_Motion( m_exit_motion );
	level_exit->Set_Path_Identifier( m_path_identifier );
	level_exit->Set_Direction( m_start_direction );
	level_exit->Set_Level( m_dest_level );
	level_exit->Set_Entry( m_dest_entry );
	return level_exit;
}

void cLevel_Exit :: Load_From_XML( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// type
	Set_Type( static_cast<Level_Exit_type>(attributes.getValueAsInteger( "type", m_exit_type )) );
	// motion
	Set_Camera_Motion( static_cast<Camera_movement>(attributes.getValueAsInteger( "camera_motion", m_exit_motion )) );
	// destination level
	Set_Level( attributes.getValueAsString( "level_name" ).c_str() );
	// destination entry
	Set_Entry( attributes.getValueAsString( "entry" ).c_str() );
	// path identifier
	if( m_exit_motion == CAMERA_MOVE_ALONG_PATH || m_exit_motion == CAMERA_MOVE_ALONG_PATH_BACKWARDS )
	{
		Set_Path_Identifier( attributes.getValueAsString( "path_identifier" ).c_str() );
	}
	// direction
	if( m_exit_type == LEVEL_EXIT_WARP )
	{
		Set_Direction( Get_Direction_Id( attributes.getValueAsString( "direction", Get_Direction_Name( m_start_direction ) ).c_str() ) );
	}
}

void cLevel_Exit :: Save_To_XML( CEGUI::XMLSerializer &stream )
{
	// begin
	stream.openTag( m_type_name );

	// position
	Write_Property( stream, "posx", static_cast<int>( m_start_pos_x ) );
	Write_Property( stream, "posy", static_cast<int>( m_start_pos_y ) );
	// type
	Write_Property( stream, "type", m_exit_type );
	// camera motion
	Write_Property( stream, "camera_motion", m_exit_motion );

	// destination level name
	std::string str_level = Get_Level( 0, 0 );
	if( !str_level.empty() )
	{
		Write_Property( stream, "level_name", str_level );
	}

	// destination entry name
	if( !m_dest_entry.empty() )
	{
		Write_Property( stream, "entry", m_dest_entry );
	}

	// path identifier
	if( m_exit_motion == CAMERA_MOVE_ALONG_PATH || m_exit_motion == CAMERA_MOVE_ALONG_PATH_BACKWARDS )
	{
		if( !m_path_identifier.empty() )
		{
			Write_Property( stream, "path_identifier", m_path_identifier );
		}
	}

	if( m_exit_type == LEVEL_EXIT_WARP )
	{
		// direction
		Write_Property( stream, "direction", Get_Direction_Name( m_start_direction ) );
	}

	// end
	stream.closeTag();
}

void cLevel_Exit :: Set_Direction( const ObjectDirection dir )
{
	// already set
	if( m_direction == dir )
	{
		return;
	}

	cAnimated_Sprite::Set_Direction( dir, 1 );

	Create_Name();
}

void cLevel_Exit :: Create_Name( void )
{
	m_name = _("Level Exit");

	if( m_exit_type == LEVEL_EXIT_BEAM )
	{
		m_name += _(" Beam");
	}
	else if( m_exit_type == LEVEL_EXIT_WARP )
	{
		m_name += _(" Warp");

		if( m_direction == DIR_UP )
		{
			m_name += " U";
		}
		else if( m_direction == DIR_LEFT )
		{
			m_name += " L";
		}
		else if( m_direction == DIR_DOWN )
		{
			m_name += " D";
		}
		else if( m_direction == DIR_RIGHT )
		{
			m_name += " R";
		}
	}
}

void cLevel_Exit :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( !m_valid_draw )
	{
		return;
	}

	// draw color rect
	pVideo->Draw_Rect( m_col_rect.m_x - pActive_Camera->m_x, m_col_rect.m_y - pActive_Camera->m_y, m_col_rect.m_w, m_col_rect.m_h, m_editor_pos_z, &m_editor_color );

	// draw destination entry name
	if( m_editor_entry_name )
	{
		// create request
		cSurface_Request *surface_request = new cSurface_Request();
		// blit
		m_editor_entry_name->Blit( m_col_rect.m_x + m_col_rect.m_w + 5 - pActive_Camera->m_x, m_col_rect.m_y - pActive_Camera->m_y, m_editor_pos_z, surface_request );
		surface_request->m_shadow_pos = 2;
		surface_request->m_shadow_color = lightgreyalpha64;
		// add request
		pRenderer->Add( surface_request );
	}
}

void cLevel_Exit :: Activate( void )
{
	// warp player out
	if( m_exit_type == LEVEL_EXIT_WARP )
	{
		pAudio->Play_Sound( "enter_pipe.ogg" );

		pLevel_Player->Set_Moving_State( STA_FALL );
		pLevel_Player->Set_Image_Num( pLevel_Player->Get_Image() + pLevel_Player->m_direction );
		pLevel_Player->Stop_Ducking();
		pLevel_Player->Reset_On_Ground();

		// set position and image
		if( m_direction == DIR_UP || m_direction == DIR_DOWN )
		{
			pLevel_Player->Set_Pos_X( m_col_rect.m_x - pLevel_Player->m_col_pos.m_x + ( m_col_rect.m_w * 0.5f ) - ( pLevel_Player->m_col_rect.m_w * 0.5f ) );
		}
		else if( m_direction == DIR_LEFT || m_direction == DIR_RIGHT )
		{
			pLevel_Player->Set_Pos_Y( m_col_rect.m_y - pLevel_Player->m_col_pos.m_y + ( m_col_rect.m_h * 0.5f ) - ( pLevel_Player->m_col_rect.m_h * 0.5f ) );

			// set rotation
			if( m_direction == DIR_RIGHT )
			{
				pLevel_Player->Set_Rotation_Z( 90.0f );
			}
			else if( m_direction == DIR_LEFT )
			{
				pLevel_Player->Set_Rotation_Z( 270.0f );
			}
		}

		float player_posz = pLevel_Player->m_pos_z;
		// change position z to be behind massive for the animation
		pLevel_Player->m_pos_z = 0.0799f;

		// set the speed
		float speedx = 0.0f;
		float speedy = 0.0f;

		if( m_direction == DIR_DOWN )
		{
			speedy = 2.7f;
		}
		else if( m_direction == DIR_UP )
		{
			speedy = -2.7f;
		}
		else if( m_direction == DIR_RIGHT )
		{
			speedx = 2.7f;
		}
		else if( m_direction == DIR_LEFT )
		{
			speedx = -2.7f;
		}

		// size moved is the height
		float maryo_size = pLevel_Player->m_col_rect.m_h;

		// move slowly in
		while( maryo_size > 0.0f )
		{
			pLevel_Player->Move( speedx, speedy );

			// reduce size
			if( speedx > 0.0f )
			{
				maryo_size -= speedx * pFramerate->m_speed_factor;
			}
			else if( speedx < 0.0f )
			{
				maryo_size += speedx * pFramerate->m_speed_factor;
			}
			else if( speedy > 0.0f )
			{
				maryo_size -= speedy * pFramerate->m_speed_factor;
			}
			else if( speedy < 0.0f )
			{
				maryo_size += speedy * pFramerate->m_speed_factor;
			}
			else
			{
				break;
			}

			// update audio
			pAudio->Update();
			// draw
			Draw_Game();

			pVideo->Render();
			pFramerate->Update();
		}

		// set position z back
		pLevel_Player->m_pos_z = player_posz;
		// set invisible
		pLevel_Player->Set_Active( 0 );

		if( m_direction == DIR_RIGHT || m_direction == DIR_LEFT )
		{
			pLevel_Player->Set_Rotation_Z( 0 );
		}
	}

	pLevel_Player->Clear_Collisions();

	// exit level
	if( m_dest_level.empty() && m_dest_entry.empty() )
	{
		pLevel_Manager->Finish_Level( 1 );
	}
	// enter entry
	else
	{
		pLevel_Manager->Goto_Sub_Level( m_dest_level, m_dest_entry, m_exit_motion, m_path_identifier );
	}
}

void cLevel_Exit :: Set_Type( Level_Exit_type exit_type )
{
	m_exit_type = exit_type;

	Create_Name();
}

void cLevel_Exit :: Set_Camera_Motion( Camera_movement camera_motion )
{
	m_exit_motion = camera_motion;
}

void cLevel_Exit :: Set_Level( std::string filename )
{
	if( filename.empty() && m_dest_entry.empty() )
	{
		m_dest_level.clear();
		// red for no destination level
		m_editor_color = red;
		m_editor_color.alpha = 128;
		return;
	}

	// lila for set destination level
	m_editor_color = lila;
	m_editor_color.alpha = 128;

	// erase file type and directory if set
	m_dest_level = Trim_Filename( filename, 0, 0 );
}

std::string cLevel_Exit :: Get_Level( bool with_dir /* = 1 */, bool with_end /* = 1 */ ) const
{
	std::string name = m_dest_level;

	// Get level path
	pLevel_Manager->Get_Path( name );

	// return
	return Trim_Filename( name, with_dir, with_end );
}

void cLevel_Exit :: Set_Entry( const std::string &entry_name )
{
	if( m_editor_entry_name )
	{
		delete m_editor_entry_name;
		m_editor_entry_name = NULL;
	}

	// Set new name
	m_dest_entry = entry_name;

	// if empty don't create the editor image
	if( m_dest_entry.empty() )
	{
		return;
	}

	m_editor_entry_name = pFont->Render_Text( pFont->m_font_small, m_dest_entry, white );
}

void cLevel_Exit :: Set_Path_Identifier( const std::string &identifier )
{
	m_path_identifier = identifier;
}

bool cLevel_Exit :: Is_Draw_Valid( void )
{
	// if editor not enabled
	if( !editor_enabled )
	{
		return 0;
	}

	// if not visible on the screen
	if( !m_active || !Is_Visible_On_Screen() )
	{
		return 0;
	}

	return 1;
}

void cLevel_Exit :: Editor_Activate( void )
{
	// get window manager
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// warp
	if( m_exit_type == LEVEL_EXIT_WARP )
	{
		// direction
		CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "level_exit_direction" ));
		Editor_Add( UTF8_("Direction"), UTF8_("Direction to move in"), combobox, 100, 105 );

		combobox->addItem( new CEGUI::ListboxTextItem( "up" ) );
		combobox->addItem( new CEGUI::ListboxTextItem( "down" ) );
		combobox->addItem( new CEGUI::ListboxTextItem( "right" ) );
		combobox->addItem( new CEGUI::ListboxTextItem( "left" ) );
		combobox->setText( Get_Direction_Name( m_start_direction ) );

		combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cLevel_Exit::Editor_Direction_Select, this ) );
	}

	// motion
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "CAMERA_MOVEotion" ));
	Editor_Add( UTF8_("Motion"), UTF8_("Camera Motion"), combobox, 100, 105 );

	combobox->addItem( new CEGUI::ListboxTextItem( "fly" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "blink" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "path" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "path backwards" ) );

	if( m_exit_motion == CAMERA_MOVE_FLY )
	{
		combobox->setText( "fly" );
	}
	else if( m_exit_motion == CAMERA_MOVE_BLINK )
	{
		combobox->setText( "blink" );
	}
	else if( m_exit_motion == CAMERA_MOVE_ALONG_PATH_BACKWARDS )
	{
		combobox->setText( "path backwards" );
	}
	else
	{
		combobox->setText( "path" );
	}

	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cLevel_Exit::Editor_Motion_Select, this ) );

	// destination level
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "level_exit_destination_level" ));
	Editor_Add( UTF8_("Destination Level"), UTF8_("Name of the level that should be entered. If empty uses the current level."), editbox, 150 );

	editbox->setText( Get_Level( 0, 0 ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cLevel_Exit::Editor_Destination_Level_Text_Changed, this ) );

	// destination entry
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "level_exit_destination_entry" ));
	Editor_Add( UTF8_("Destination Entry"), UTF8_("Name of the Entry in the destination level. If empty the entry point is the player start position."), editbox, 150 );

	editbox->setText( m_dest_entry.c_str() );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cLevel_Exit::Editor_Destination_Entry_Text_Changed, this ) );

	// path identifier
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "level_exit_path_identifier" ));
	Editor_Add( UTF8_("Path Identifier"), UTF8_("Name of the Path to use for the camera movement."), editbox, 150 );

	editbox->setText( m_path_identifier.c_str() );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cLevel_Exit::Editor_Path_Identifier_Text_Changed, this ) );


	// init
	Editor_Init();
}

void cLevel_Exit :: Editor_State_Update( void )
{
	// get window manager
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// path identifier
	CEGUI::Editbox *editbox_path_identifier = static_cast<CEGUI::Editbox *>(wmgr.getWindow( "level_exit_path_identifier" ));
	// destination level
	CEGUI::Editbox *editbox_destination_level = static_cast<CEGUI::Editbox *>(wmgr.getWindow( "level_exit_destination_level" ));
	// direction
	//CEGUI::Combobox *combobox_direction = static_cast<CEGUI::Combobox *>(wmgr.getWindow( "level_exit_direction" ));


	if( m_exit_motion == CAMERA_MOVE_ALONG_PATH || m_exit_motion == CAMERA_MOVE_ALONG_PATH_BACKWARDS )
	{
		editbox_path_identifier->setEnabled( 1 );
		editbox_destination_level->setEnabled( 0 );
	}
	else if( m_exit_motion == CAMERA_MOVE_FLY )
	{
		editbox_path_identifier->setEnabled( 0 );
		editbox_destination_level->setEnabled( 0 );
	}
	else
	{
		editbox_path_identifier->setEnabled( 0 );
		editbox_destination_level->setEnabled( 1 );
	}

	/*if( m_exit_type == LEVEL_EXIT_WARP )
	{
		combobox_direction->setEnabled( 1 );
	}
	else
	{
		combobox_direction->setEnabled( 0 );
	}*/
}

bool cLevel_Exit :: Editor_Direction_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Direction( Get_Direction_Id( item->getText().c_str() ) );

	return 1;
}

bool cLevel_Exit :: Editor_Motion_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();
	std::string str_text = item->getText().c_str();

	if( str_text.compare( "fly" ) == 0 )
	{
		Set_Camera_Motion( CAMERA_MOVE_FLY );
	}
	else if( str_text.compare( "blink" ) == 0 )
	{
		Set_Camera_Motion( CAMERA_MOVE_BLINK );
	}
	else if( str_text.compare( "path backwards" ) == 0 )
	{
		Set_Camera_Motion( CAMERA_MOVE_ALONG_PATH_BACKWARDS );
	}
	else
	{
		Set_Camera_Motion( CAMERA_MOVE_ALONG_PATH );
	}

	Editor_State_Update();

	return 1;
}

bool cLevel_Exit :: Editor_Destination_Level_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Level( str_text );

	return 1;
}

bool cLevel_Exit :: Editor_Destination_Entry_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Entry( str_text );

	return 1;
}

bool cLevel_Exit :: Editor_Path_Identifier_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Path_Identifier( str_text );

	return 1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
