/***************************************************************************
 * level.cpp  -  level handling class
 *
 * Copyright (C) 2003 - 2009 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../level/level.h"
#include "../level/level_editor.h"
#include "../core/game_core.h"
#include "../gui/menu.h"
#include "../user/preferences.h"
#include "../audio/audio.h"
#include "../level/level_player.h"
#include "../objects/goldpiece.h"
#include "../objects/level_exit.h"
#include "../video/font.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "../input/joystick.h"
#include "../user/savegame.h"
#include "../overworld/world_manager.h"
#include "../overworld/overworld.h"
#include "../enemies/turtle.h"
#include "../enemies/bosses/turtle_boss.h"
#include "../enemies/rokko.h"
#include "../enemies/krush.h"
#include "../enemies/furball.h"
#include "../enemies/flyon.h"
#include "../enemies/thromp.h"
#include "../enemies/eato.h"
#include "../enemies/gee.h"
#include "../enemies/spika.h"
#include "../enemies/static.h"
#include "../enemies/spikeball.h"
#include "../objects/powerup.h"
#include "../objects/star.h"
#include "../objects/enemystopper.h"
#include "../objects/spinbox.h"
#include "../objects/bonusbox.h"
#include "../objects/text_box.h"
#include "../objects/moving_platform.h"
#include "../video/renderer.h"
#include "../core/math/utilities.h"
#include "../core/i18n.h"
#include "../objects/path.h"
#include "../core/filesystem/filesystem.h"
#include "../core/filesystem/resource_manager.h"
#include "../overworld/world_editor.h"
// CEGUI
#include "CEGUIXMLParser.h"

namespace SMC
{

/* *** *** *** *** *** cLevel *** *** *** *** *** *** *** *** *** *** *** *** */

cLevel :: cLevel( void )
{
	// settings
	Reset_Settings();

	m_delayed_unload = 0;

	m_sprite_manager = new cSprite_Manager();
	m_background_manager = new cBackground_Manager();
	m_global_effect = new cGlobal_effect( m_sprite_manager );
	m_animation_manager = new cAnimation_Manager();

	// add default gradient layer
	cBackground *gradient_background = new cBackground( m_sprite_manager );
	gradient_background->Set_Type( BG_GR_VER );
	gradient_background->m_pos_z = 0.0001f;
	m_background_manager->Add( gradient_background );
}

cLevel :: ~cLevel( void )
{
	Unload();

	// delete
	delete m_background_manager;
	delete m_global_effect;
	delete m_animation_manager;
	delete m_sprite_manager;
}

bool cLevel :: New( std::string filename )
{
	Unload();

	ifstream ifs;

	// if no name is given create name
	if( filename.empty() )
	{
		unsigned int i = 1;

		// search for a not existing file
		while( 1 )
		{
			// set name
			filename = pResource_Manager->user_data_dir + USER_LEVEL_DIR + "/new_" + int_to_string( i ) + ".smclvl";
			// try to open the file
			ifs.open( filename.c_str(), ios::in );

			// found unused name
			if( !ifs.is_open() )
			{
				break;
			}

			// stop on 99
			if( i > 99 )
			{
				return 0;
			}
			
			ifs.close();
			i++;
		}
	}
	else
	{
		// set file type
		if( filename.find( ".smclvl" ) == std::string::npos )
		{
			filename.insert( filename.length(), ".smclvl" );
		}

		// set user directory
		if( filename.find( pResource_Manager->user_data_dir + USER_LEVEL_DIR + "/" ) == std::string::npos )
		{
			filename.insert( 0, pResource_Manager->user_data_dir + USER_LEVEL_DIR + "/" );
		}
	}

	// open file
	ifs.open( filename.c_str(), ios::in );

	// Level doesn't exist
	if( !ifs )
	{
		// set filename
		m_level_filename = filename;
		m_engine_version = level_engine_version;

		return 1;
	}

	// Level already exists
	ifs.close();
	return 0;
}

bool cLevel :: Load( std::string filename )
{
	m_next_level_filename.clear();

	if( !pLevel_Manager->Get_Path( filename ) )
	{
		// show error without directory and file type
		printf( "Couldn't load level : %s\n", Trim_Filename( filename, 0, 0 ).c_str() );
		return 0;
	}

	Unload();

	// new level format
	if( filename.rfind( ".smclvl" ) != std::string::npos )
	{
		try
		{
			CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/Level.xsd", "" );
		}
		// catch CEGUI Exceptions
		catch( CEGUI::Exception &ex )
		{
			printf( "Loading Level %s CEGUI Exception %s\n", filename.c_str(), ex.getMessage().c_str() );
			pHud_Debug->Set_Text( _("Loading Level failed : ") + (const std::string)ex.getMessage().c_str() );
			return 0;
		}
	}
	// old level format
	else
	{
		pHud_Debug->Set_Text( _("Unsupported Level format : ") + (const std::string)filename.c_str() );
		return 0;
	}

	/* late initialization
	 * needed to create links to other objects
	*/
	for( cSprite_List::iterator itr = m_sprite_manager->objects.begin(); itr != m_sprite_manager->objects.end(); ++itr )
	{
		cSprite *obj = (*itr);

		obj->Init_Links();
	}

	m_level_filename = filename;

	// engine version entry not set
	if( m_engine_version < 0 )
	{
		m_engine_version = 0;
	}

	return 1;
}

void cLevel :: Unload( bool delayed /* = 0 */ )
{
	if( delayed )
	{
		m_delayed_unload = 1;
		return;
	}
	else
	{
		m_delayed_unload = 0;
	}

	// not loaded
	if( !Is_Loaded() )
	{
		return;
	}

	// delete backgrounds
	m_background_manager->Delete_All();

	// add default gradient layer
	cBackground *gradient_background = new cBackground( m_sprite_manager );
	gradient_background->Set_Type( BG_GR_VER );
	gradient_background->m_pos_z = 0.0001f;
	m_background_manager->Add( gradient_background );

	// reset music
	m_musicfile.clear();
	m_valid_music = 0;

	m_author.clear();
	m_version.clear();

	// clear global effect
	m_global_effect->Clear();

	// no version
	m_engine_version = -1;

	m_level_filename.clear();

	Reset_Settings();

	/* delete sprites
	 * do this at last
	*/
	m_sprite_manager->Delete_All();
}

void cLevel :: Save( void )
{
	pAudio->Play_Sound( "editor/save.ogg" );

	// use user level dir
	if( m_level_filename.find( pResource_Manager->user_data_dir + USER_LEVEL_DIR + "/" ) == std::string::npos )
	{
		// erase old directory
		m_level_filename = Trim_Filename( m_level_filename, 0, 1 );
		// set user directory
		m_level_filename.insert( 0, pResource_Manager->user_data_dir + USER_LEVEL_DIR + "/" );
	}

	ofstream file( m_level_filename.c_str(), ios::out | ios::trunc );

	if( !file )
	{
		pHud_Debug->Set_Text( _("Couldn't save level ") + m_level_filename );
		return;
	}

	// xml info
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	// begin level
	file << "<level>" << std::endl;

	// begin info
	file << "\t<information>" << std::endl;
		// game version
		file << "\t\t<Property name=\"game_version\" value=\"" << smc_version << "\" />" << std::endl;
		// engine version
		file << "\t\t<Property name=\"engine_version\" value=\"" << level_engine_version << "\" />" << std::endl;
		// time ( seconds since 1970 )
		file << "\t\t<Property name=\"save_time\" value=\"" << time( NULL ) << "\" />" << std::endl;
	// end info
	file << "\t</information>" << std::endl;

	// begin settings
	file << "\t<settings>" << std::endl;
		// level author
		file << "\t\t<Property name=\"lvl_author\" value=\"" << string_to_xml_string( m_author ) << "\" />" << std::endl;
		// level version
		file << "\t\t<Property name=\"lvl_version\" value=\"" << string_to_xml_string( m_version ) << "\" />" << std::endl;
		// music
		std::string music_file = Get_Musicfile( 1 );
		file << "\t\t<Property name=\"lvl_music\" value=\"" << string_to_xml_string( music_file ) << "\" />" << std::endl;
		// camera limits
		file << "\t\t<Property name=\"cam_limit_x\" value=\"" << static_cast<int>(m_camera_limits.m_x) << "\" />" << std::endl;
		file << "\t\t<Property name=\"cam_limit_y\" value=\"" << static_cast<int>(m_camera_limits.m_y) << "\" />" << std::endl;
		file << "\t\t<Property name=\"cam_limit_w\" value=\"" << static_cast<int>(m_camera_limits.m_w) << "\" />" << std::endl;
		file << "\t\t<Property name=\"cam_limit_h\" value=\"" << static_cast<int>(m_camera_limits.m_h) << "\" />" << std::endl;
		// fixed camera horizontal velocity
		file << "\t\t<Property name=\"cam_fixed_hor_vel\" value=\"" << m_fixed_camera_hor_vel << "\" />" << std::endl;
	// end settings
	file << "\t</settings>" << std::endl;

	// backgrounds
	for( vector<cBackground *>::iterator itr = m_background_manager->objects.begin(); itr != m_background_manager->objects.end(); ++itr )
	{
		(*itr)->Save_To_Stream( file );
	}

	// global effect
	m_global_effect->Save_To_Stream( file );

	// begin player
	file << "\t<player>" << std::endl;
		// position
		file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(pLevel_Player->m_start_pos_x) << "\" />" << std::endl;
		file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(pLevel_Player->m_start_pos_y) << "\" />" << std::endl;
		// direction
		file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_Name( pLevel_Player->m_start_direction ) << "\" />" << std::endl;
	// end player
	file << "\t</player>" << std::endl;

	// objects
	for( cSprite_List::iterator itr = m_sprite_manager->objects.begin(); itr != m_sprite_manager->objects.end(); ++itr )
	{
		cSprite *obj = (*itr);

		// skip spawned and destroyed objects
		if( obj->m_spawned || obj->m_auto_destroy )
		{
			continue;
		}

		// save to file stream
		obj->Save_To_Stream( file );
	}

	// end level
	file << "</level>" << std::endl;
	file.close();

	pHud_Debug->Set_Text( _("Level ") + Trim_Filename( m_level_filename, 0, 0 ) + _(" saved") );
}

void cLevel :: Delete( void )
{
	Delete_File( m_level_filename );

	Unload();
}

void cLevel :: Reset_Settings( void )
{
	// no engine version
	m_engine_version = -1;
	m_last_saved = 0;
	m_author.clear();
	m_version.clear();

	// set default music
	Set_Musicfile( DATA_DIR "/" GAME_MUSIC_DIR "/" LEVEL_DEFAULT_MUSIC );

	// player
	m_player_start_pos_x = cLevel_Player::m_default_pos_x;
	m_player_start_pos_y = cLevel_Player::m_default_pos_y;
	m_player_start_direction = DIR_RIGHT;

	// camera
	m_camera_limits = cCamera::m_default_limits;
	m_fixed_camera_hor_vel = 0.0f;
}

void cLevel :: Init( void )
{
	// if not loaded
	if( !Is_Loaded() )
	{
		return;
	}

	// player position
	pLevel_Player->Set_Pos( m_player_start_pos_x, m_player_start_pos_y, 1 );
	// player direction
	pLevel_Player->Set_Direction( m_player_start_direction, 1 );
	// player reset
	pLevel_Player->Reset();

	// init global effect
	m_global_effect->Init_Anim();
}

void cLevel :: Set_Sprite_Manager( void )
{
	pHud_Manager->Set_Sprite_Manager( m_sprite_manager );
	pMouseCursor->Set_Sprite_Manager( m_sprite_manager );
	pLevel_Editor->Set_Sprite_Manager( m_sprite_manager );
	pLevel_Editor->Set_Level( this );
	// camera
	pLevel_Manager->m_camera->Set_Sprite_Manager( m_sprite_manager );
	pLevel_Manager->m_camera->Set_Limits( m_camera_limits );
	pLevel_Manager->m_camera->m_fixed_hor_vel = m_fixed_camera_hor_vel;

	pLevel_Player->Set_Sprite_Manager( m_sprite_manager );
}

void cLevel :: Enter( const GameMode old_mode /* = MODE_NOTHING */ )
{
	// if not loaded
	if( !Is_Loaded() )
	{
		return;
	}

	Set_Sprite_Manager();
	// set active camera
	pActive_Camera = pLevel_Manager->m_camera;
	// set active player
	pActive_Player = pLevel_Player;
	// set animation manager
	pActive_Animation_Manager = m_animation_manager;

	// camera position
	pLevel_Manager->m_camera->Center();

	// play music
	if( m_valid_music )
	{
		pAudio->Play_Music( m_musicfile, -1, 0, 1000 );
	}
	else if( pAudio->m_music_enabled )
	{
		printf( "Warning : Music file not found %s\n", pActive_Level->m_musicfile.c_str() );
	}

	// disable world editor
	pWorld_Editor->Disable();

	// set editor enabled state
	editor_enabled = pLevel_Editor->m_enabled;

	if( pLevel_Editor->m_enabled )
	{
		if( !pLevel_Editor->m_editor_window->isVisible() )
		{
			pLevel_Editor->m_editor_window->show();
			pMouseCursor->Set_Active( 1 );
		}
	}
	// enable editor
	else if( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM_EDITOR )
	{
		Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM;
		pLevel_Editor->Enable();
	}

	// Update Hud Text and position
	pHud_Manager->Update_Text();

	// reset speedfactor
	pFramerate->Reset();
}

void cLevel :: Leave( const GameMode next_mode /* = MODE_NOTHING */ )
{
	// if not in level mode
	if( Game_Mode != MODE_LEVEL )
	{
		return;
	}

	// reset camera limits
	pLevel_Manager->m_camera->Reset_Limits();
	pLevel_Manager->m_camera->m_fixed_hor_vel = 0.0f;

	// reset mouse
	pMouseCursor->Reset( 0 );

	// level to level
	if( next_mode == MODE_LEVEL )
	{
		// fade out music
		pAudio->Fadeout_Music( 500 );
		return;
	}

	pJoystick->Reset_keys();

	// hide editor window if visible
	if( pLevel_Editor->m_enabled )
	{
		if( pLevel_Editor->m_editor_window->isVisible() )
		{
			pLevel_Editor->m_editor_window->hide();
		}
	}

	// if new mode should play different music
	if( next_mode != MODE_MENU && next_mode != MODE_LEVEL_SETTINGS )
	{
		// fade out music
		pAudio->Fadeout_Music( 1000 );
	}
}

void cLevel :: Update( void )
{
	if( m_delayed_unload )
	{
		Unload();
		return;
	}

	if( !m_next_level_filename.empty() )
	{
		Load( m_next_level_filename );
	}

	// if leveleditor is not active
	if( !editor_level_enabled )
	{
		// backgrounds
		for( vector<cBackground *>::iterator itr = m_background_manager->objects.begin(); itr != m_background_manager->objects.end(); ++itr )
		{
			(*itr)->Update();
		}

		// objects
		m_sprite_manager->Update_Items();
		// animations
		m_animation_manager->Update();
		// global effect
		m_global_effect->Update();
	}
}

void cLevel :: Update_Late( void )
{
	// if leveleditor is not active
	if( !editor_level_enabled )
	{
		// objects
		m_sprite_manager->Update_Items_Late();
	}
}

void cLevel :: Draw_Layer_1( LevelDrawType type /* = LVL_DRAW */ )
{
	// with background
	if( type != LVL_DRAW_NO_BG )
	{
		for( vector<cBackground *>::iterator itr = m_background_manager->objects.begin(); itr != m_background_manager->objects.end(); ++itr )
		{
			(*itr)->Draw();
		}
	}

	// only background
	if( type == LVL_DRAW_BG )
	{
		return;
	}

	// Objects
	m_sprite_manager->Draw_Items();
	// Animations
	m_animation_manager->Draw();
}

void cLevel :: Draw_Layer_2( LevelDrawType type /* = LVL_DRAW */ )
{
	// only background
	if( type == LVL_DRAW_BG )
	{
		return;
	}

	// global effect
	if( !editor_level_enabled )
	{
		m_global_effect->Draw();
	}

	// ghost
	if( pLevel_Player->m_maryo_type == MARYO_GHOST )
	{
		// create request
		cRect_Request *request = new cRect_Request();

		Color color = Color( 0.5f, 0.5f, 0.5f, 0.3f );

		// fade alpha in
		if( pLevel_Player->m_ghost_time > 220 )
		{
			color.alpha = static_cast<Uint8>(color.alpha * ( ( -pLevel_Player->m_ghost_time + 320 ) * 0.01f ));
		}
		// fade alpha out
		else if( pLevel_Player->m_ghost_time < 100 )
		{
			color.alpha = static_cast<Uint8>(color.alpha * ( pLevel_Player->m_ghost_time * 0.01f ));
		}

		pVideo->Draw_Rect( 0, 0, static_cast<float>(game_res_w), static_cast<float>(game_res_h), 0.12f, &color, request );

		request->combine_type = GL_MODULATE;

		request->combine_col[0] = 0.9f;
		request->combine_col[1] = 0.6f;
		request->combine_col[2] = 0.8f;

		// add request
		pRenderer->Add( request );
	}
}

void cLevel :: Process_Input( void )
{
	// only non editor
	if( !editor_level_enabled )
	{
		// none
	}
}

bool cLevel :: Key_Down( const SDLKey key )
{
	// debug key F2
	if( key == SDLK_F2 && game_debug && !editor_level_enabled )
	{
		pLevel_Player->Set_Type( MARYO_CAPE, 0 );
	}
	// debug key F3
	else if( key == SDLK_F3 && !editor_level_enabled )
	{
		//pLevel_Player->GotoNextLevel();
		//DrawEffect( HORIZONTAL_VERTICAL_FADE );
		//pLevel_Player->Draw_Animation( MARYO_FIRE );

		cParticle_Emitter *anim = new cParticle_Emitter( m_sprite_manager );
		anim->Set_Emitter_Rect( pLevel_Player->m_pos_x + static_cast<float>( pLevel_Player->m_col_rect.m_w / 2 ), pLevel_Player->m_pos_y - 100, 10, 10 );
		anim->Set_Emitter_Time_to_Live( -1 );
		anim->Set_Emitter_Iteration_Interval( 5 );
		anim->Set_Quota( 200 );
		anim->Set_Image( pVideo->Get_Surface( "animation/particles/star.png" ) );
		anim->Set_Time_to_Live( 3, 3 );
		anim->Set_Fading_Alpha( 1 );
		anim->Set_Speed( 1, 4 );
		anim->Set_Scale( 0.5f );
		anim->Set_Const_Rotation_Z( -5, 10 );
		m_animation_manager->Add( anim );
	}
	// debug key F4
	else if( key == SDLK_F4 )
	{
		Draw_Effect_Out( EFFECT_OUT_FIXED_COLORBOX );
		//pLevel_Player->Get_Item( TYPE_MUSHROOM_DEFAULT );

		/*if( m_global_effect->type != GL_EFF_FALLING )
		{
			m_global_effect->Set_image( "animation/particles/rain.png" );
			m_global_effect->Set_Type( GL_EFF_FALLING );
			m_global_effect->Set_Lifetime_mod( 4 );
			m_global_effect->Set_Scale( 0.5f, 0.5f );
			m_global_effect->Set_Creation_Speed( 2 );
			m_global_effect->Set_Speed( 13, 5 );
			m_global_effect->Set_Direction( 90, 0 );
			m_global_effect->Set_ConstRotationZ( 0, 0 );
			m_global_effect->Init_Anim();
		}*/
	}
	// Toggle leveleditor
	else if( key == SDLK_F8 )
	{
		pLevel_Editor->Toggle();
	}
	// ## Game
	// Shoot
	else if( key == pPreferences->m_key_shoot && !editor_enabled )
	{
		pLevel_Player->Action_Shoot();
	}
	// Jump
	else if( key == pPreferences->m_key_jump && !editor_enabled )
	{
		pLevel_Player->Action_Jump();
	}
	// Action
	else if( key == pPreferences->m_key_action && !editor_enabled )
	{
		pLevel_Player->Action_Interact( INP_ACTION );
	}
	// Up
	else if( key == pPreferences->m_key_up && !editor_enabled )
	{
		pLevel_Player->Action_Interact( INP_UP );
	}
	// Down
	else if( key == pPreferences->m_key_down && !editor_enabled )
	{
		pLevel_Player->Action_Interact( INP_DOWN );
	}
	// Left
	else if( key == pPreferences->m_key_left && !editor_enabled )
	{
		pLevel_Player->Action_Interact( INP_LEFT );
	}
	// Right
	else if( key == pPreferences->m_key_right && !editor_enabled )
	{
		pLevel_Player->Action_Interact( INP_RIGHT );
	}
	// Request Item
	else if( key == pPreferences->m_key_item && !editor_enabled )
	{
		pLevel_Player->Action_Interact( INP_ITEM );
	}
	// God Mode
	else if( pKeyboard->m_keys[SDLK_g] && pKeyboard->m_keys[SDLK_o] && pKeyboard->m_keys[SDLK_d] && !editor_enabled )
	{
		if( pLevel_Player->m_god_mode )
		{
			pHud_Debug->Set_Text( "Funky God Mode disabled" );
		}
		else
		{
			pHud_Debug->Set_Text( "Funky God Mode enabled" );
		}

		pLevel_Player->m_god_mode = !pLevel_Player->m_god_mode;
	}
	// Set Small state
	else if( pKeyboard->m_keys[SDLK_k] && pKeyboard->m_keys[SDLK_i] && pKeyboard->m_keys[SDLK_d] && !editor_enabled )
	{
		pLevel_Player->Set_Type( MARYO_SMALL, 0 );
	}
	// Exit
	else if( key == SDLK_ESCAPE )
	{
		pLevel_Player->Action_Interact( INP_EXIT );
	}
	// ## editor
	else if( pLevel_Editor->Key_Down( key ) )
	{
		// processed by the editor
		return 1;
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cLevel :: Key_Up( const SDLKey key )
{
	// only if not in Editor
	if( editor_level_enabled )
	{
		return 0;
	}

	// Interaction keys
	if( key == pPreferences->m_key_right )
	{
		pLevel_Player->Action_Stop_Interact( INP_RIGHT );
	}
	else if( key == pPreferences->m_key_left )
	{
		pLevel_Player->Action_Stop_Interact( INP_LEFT );
	}
	else if( key == pPreferences->m_key_down )
	{
		pLevel_Player->Action_Stop_Interact( INP_DOWN );
	}
	else if( key == pPreferences->m_key_jump )
	{
		pLevel_Player->Action_Stop_Interact( INP_JUMP );
	}
	else if( key == pPreferences->m_key_shoot )
	{
		pLevel_Player->Action_Stop_Interact( INP_SHOOT );
	}
	else if( key == pPreferences->m_key_action )
	{
		pLevel_Player->Action_Stop_Interact( INP_ACTION );
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cLevel :: Mouse_Down( Uint8 button )
{
	// ## editor
	if( pLevel_Editor->Mouse_Down( button ) )
	{
		// processed by the editor
		return 1;
	}
	else
	{
		// not processed
		return 0;
	}

	// button got processed
	return 1;
}

bool cLevel :: Mouse_Up( Uint8 button )
{
	// ## editor
	if( pLevel_Editor->Mouse_Up( button ) )
	{
		// processed by the editor
		return 1;
	}
	else
	{
		// not processed
		return 0;
	}

	// button got processed
	return 1;
}

bool cLevel :: Joy_Button_Down( Uint8 button )
{
	// Shoot
	if( button == pPreferences->m_joy_button_shoot && !editor_enabled )
	{
		pLevel_Player->Action_Interact( INP_SHOOT );
	}
	// Jump
	else if( button == pPreferences->m_joy_button_jump && !editor_enabled )
	{
		pLevel_Player->Action_Interact( INP_JUMP );
	}
	// Interaction keys
	else if( button == pPreferences->m_joy_button_action && !editor_enabled )
	{
		pLevel_Player->Action_Interact( INP_ACTION );
	}
	// Request Itembox Item
	else if( button == pPreferences->m_joy_button_item && !editor_enabled )
	{
		pLevel_Player->Action_Interact( INP_ITEM );
	}
	// Enter menu
	else if( button == pPreferences->m_joy_button_exit )
	{
		pLevel_Player->Action_Interact( INP_EXIT );
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cLevel :: Joy_Button_Up( Uint8 button )
{
	// only if not in Editor
	if( editor_level_enabled )
	{
		return 0;
	}

	if( button == pPreferences->m_joy_button_jump )
	{
		pLevel_Player->Action_Stop_Interact( INP_JUMP );
	}
	else if( button == pPreferences->m_joy_button_shoot )
	{
		pLevel_Player->Action_Stop_Interact( INP_SHOOT );
	}
	else if( button == pPreferences->m_joy_button_action )
	{
		pLevel_Player->Action_Stop_Interact( INP_ACTION );
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

std::string cLevel :: Get_Musicfile( int with_dir /* = 2 */, bool with_end /* = 1 */ ) const
{
	std::string filename = m_musicfile;

	// erase whole directory
	if( with_dir == 0 && filename.rfind( "/" ) != std::string::npos )
	{
		filename.erase( 0, filename.rfind( "/" ) + 1 );
	}
	// erase music directory
	else if( with_dir == 1 && filename.find( DATA_DIR "/" GAME_MUSIC_DIR "/" ) != std::string::npos )
	{
		filename.erase( 0, strlen( DATA_DIR "/" GAME_MUSIC_DIR "/" ) );
	}

	// erase file type
	if( !with_end && filename.rfind( "." ) != std::string::npos )
	{
		filename.erase( filename.rfind( "." ) );
	}

	return filename;
}

void cLevel :: Set_Musicfile( std::string filename )
{
	if( filename.length() < 4 )
	{
		return;
	}

	Convert_Path_Separators( filename );

	// add music dir
	if( filename.find( DATA_DIR "/" GAME_MUSIC_DIR "/" ) == std::string::npos )
	{
		filename.insert( 0, DATA_DIR "/" GAME_MUSIC_DIR "/" );
	}

	// already set
	if( m_musicfile.compare( filename ) == 0 )
	{
		return;
	}

	m_musicfile = filename;
	// check if music is available
	m_valid_music = File_Exists( filename );

	// switch to the new music if music is playing
	if( pAudio->Is_Music_Playing() )
	{
		pAudio->Fadeout_Music( 1000 );
	}
}

void cLevel :: Set_Levelfile( std::string filename, bool delete_old /* = 1 */ )
{
	// erase file type and directory
	filename = Trim_Filename( filename, 0, 0 );

	// if invalid
	if( filename.length() < 2 )
	{
		return;
	}

	// delete file with the old name
	if( delete_old )
	{
		Delete_File( m_level_filename );
	}

	Convert_Path_Separators( filename );

	// add level file type
	if( filename.find( ".smclvl" ) == std::string::npos )
	{
		filename.insert( filename.length(), ".smclvl" );
	}

	m_level_filename = filename;

	// add level dir
	if( m_level_filename.find( pResource_Manager->user_data_dir + USER_LEVEL_DIR + "/" ) == std::string::npos )
	{
		m_level_filename.insert( 0, pResource_Manager->user_data_dir + USER_LEVEL_DIR + "/" );
	}

	// save with new filename
	Save();
}

void cLevel :: Set_Author( const std::string &name )
{
	m_author = name;
}

void cLevel :: Set_Version( const std::string &level_version )
{
	m_version = level_version;
}

cLevel_Entry *cLevel :: Get_Entry( const std::string &name )
{
	if( name.empty() )
	{
		return NULL;
	}

	// Search for entry
	for( cSprite_List::iterator itr = m_sprite_manager->objects.begin(); itr != m_sprite_manager->objects.end(); ++itr )
	{
		cSprite *obj = (*itr);

		if( obj->m_type != TYPE_LEVEL_ENTRY || obj->m_auto_destroy )
		{
			continue;
		}

		cLevel_Entry *level_entry = static_cast<cLevel_Entry *>(obj);

		// found
		if( level_entry->m_entry_name.compare( name ) == 0 )
		{
			return level_entry;
		}
	}

	return NULL;
}

bool cLevel :: Is_Loaded( void ) const
{
	// if not loaded version is -1
	if( m_engine_version >= 0 )
	{
		return 1;
	}

	return 0;
}

// XML element start
void cLevel :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
{
	// Property/Item/Tag of an Element
	if( element == "Property" )
	{
		m_xml_attributes.add( attributes.getValueAsString( "name" ), attributes.getValueAsString( "value" ) );
	}
}

// XML element end
void cLevel :: elementEnd( const CEGUI::String &element )
{
	if( element != "Property" )
	{
		if( element == "information" )
		{
			// support 1.7 and lower which used float
			float engine_version_float = m_xml_attributes.getValueAsFloat( "engine_version" );

			// if float engine version
			if( engine_version_float < 3 )
			{
				// change to new format
				engine_version_float *= 10;
			}

			m_engine_version = static_cast<int>(engine_version_float);
			m_last_saved = m_xml_attributes.getValueAsInteger( "save_time" );
		}
		else if( element == "settings" )
		{
			// if V.1.9 and lower : move y coordinate bottom to 0
			if( m_engine_version < 35 )
			{
				if( m_xml_attributes.exists( "cam_limit_h" ) )
				{
					m_xml_attributes.add( "cam_limit_h", CEGUI::PropertyHelper::floatToString( m_xml_attributes.getValueAsFloat( "cam_limit_h" ) - 600.0f ) );
				}
			}

			// Author
			m_author = m_xml_attributes.getValueAsString( "lvl_author" ).c_str();
			// Version
			m_version = m_xml_attributes.getValueAsString( "lvl_version" ).c_str();
			// Music
			Set_Musicfile( m_xml_attributes.getValueAsString( "lvl_music" ).c_str() );
			// Camera Limits
			m_camera_limits = GL_rect( static_cast<float>(m_xml_attributes.getValueAsInteger( "cam_limit_x" )), static_cast<float>(m_xml_attributes.getValueAsInteger( "cam_limit_y" )), static_cast<float>(m_xml_attributes.getValueAsInteger( "cam_limit_w" )), static_cast<float>(m_xml_attributes.getValueAsInteger( "cam_limit_h" )) );
			// fixed camera horizontal velocity
			m_fixed_camera_hor_vel = m_xml_attributes.getValueAsFloat( "cam_fixed_hor_vel" );
		}
		else if( element == "background" )
		{
			BackgroundType bg_type = static_cast<BackgroundType>(m_xml_attributes.getValueAsInteger( "type" ));

			// use gradient background
			if( bg_type == BG_GR_HOR || bg_type == BG_GR_VER )
			{
				m_background_manager->Get_Pointer(0)->Create_From_Stream( m_xml_attributes );
			}
			// default background
			else
			{
				m_background_manager->Add( new cBackground( m_xml_attributes, m_sprite_manager ) );
			}
		}
		else if( element == "global_effect" )
		{
			m_global_effect->Create_From_Stream( m_xml_attributes );
		}
		else if( element == "player" )
		{
			// position
			m_player_start_pos_x = static_cast<float>(m_xml_attributes.getValueAsInteger( "posx", static_cast<int>(cLevel_Player::m_default_pos_x) ));
			m_player_start_pos_y = static_cast<float>(m_xml_attributes.getValueAsInteger( "posy", static_cast<int>(cLevel_Player::m_default_pos_y) ));

			// if V.1.9 and lower : move y coordinate bottom to 0
			if( m_engine_version < 35 )
			{
				m_player_start_pos_y = m_player_start_pos_y - 600.0f;
			}

			// check level engine version
			// 10 and lower
			if( m_engine_version <= 10 )
			{
				m_player_start_pos_y += 58;
			}
			// 20 and lower
			else if( m_engine_version <= 20 )
			{
				m_player_start_pos_y -= 48;
			}

			// direction
			m_player_start_direction = Get_Direction_Id( m_xml_attributes.getValueAsString( "direction" ).c_str() );

			// if invalid set default
			if( m_player_start_direction != DIR_LEFT && m_player_start_direction != DIR_RIGHT )
			{
				m_player_start_direction = DIR_RIGHT;
			}
		}
		else
		{
			// get Level object
			cSprite *object = Get_Level_Object( element, m_xml_attributes, m_engine_version, m_sprite_manager );
			
			// valid
			if( object )
			{
				m_sprite_manager->Add( object );
			}
			else if( element == "level" )
			{
				// ignore
			}
			else if( element.length() )
			{
				printf( "Warning : Level Unknown element : %s\n", element.c_str() );
			}
		}

		// clear
		m_xml_attributes = CEGUI::XMLAttributes();
	}
}

cSprite *Get_Level_Object( const CEGUI::String &xml_element, CEGUI::XMLAttributes &attributes, int engine_version /* = level_engine_version */, cSprite_Manager *sprite_manager )
{
	// element could change
	CEGUI::String element = xml_element;

	if( element == "sprite" )
	{
		// always : fix sprite with undefined massive-type
		if( attributes.exists( "type" ) && attributes.getValueAsString( "type" ) == "undefined" )
		{
			// change to passive
			attributes.add( "type", "passive" );
		}
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( engine_version < 35 )
		{
			if( attributes.exists( "posy" ) )
			{
				attributes.add( "posy", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "posy" ) - 600.0f ) );
			}
		}
		// if V.1.7 and lower : change green_1 ground to green_3 ground image paths
		if( engine_version < 34 )
		{
			// normal
			Relocate_Image( attributes, "ground/green_1/ground/left_up.png", "ground/green_3/ground/top/left.png" );
			Relocate_Image( attributes, "ground/green_1/ground/left_down.png", "ground/green_3/ground/bottom/left.png" );
			Relocate_Image( attributes, "ground/green_1/ground/right_up.png", "ground/green_3/ground/top/right.png" );
			Relocate_Image( attributes, "ground/green_1/ground/right_down.png", "ground/green_3/ground/bottom/right.png" );
			Relocate_Image( attributes, "ground/green_1/ground/up.png", "ground/green_3/ground/top/1.png" );
			Relocate_Image( attributes, "ground/green_1/ground/down.png", "ground/green_3/ground/bottom/1.png" );
			Relocate_Image( attributes, "ground/green_1/ground/right.png", "ground/green_3/ground/middle/right.png" );
			Relocate_Image( attributes, "ground/green_1/ground/left.png", "ground/green_3/ground/middle/left.png" );
			Relocate_Image( attributes, "ground/green_1/ground/middle.png", "ground/green_3/ground/middle/1.png" );

			// hill (not available)
			//Relocate_Image( attributes, "ground/green_1/ground/hill_left_up.png", "ground/green_3/ground/" );
			//Relocate_Image( attributes, "ground/green_1/ground/hill_right_up.png", "ground/green_3/ground/" );
			//Relocate_Image( attributes, "ground/green_1/ground/hill_right.png", "ground/green_3/ground/" );
			//Relocate_Image( attributes, "ground/green_1/ground/hill_left.png", "ground/green_3/ground/" );
		}
		// if V.1.7 and lower : change slider grey_1 to green_1 brown slider image paths
		if( engine_version < 32 )
		{
			Relocate_Image( attributes, "slider/grey_1/slider_left.png", "ground/green_1/slider/1/brown/left.png" );
			Relocate_Image( attributes, "slider/grey_1/slider_middle.png", "ground/green_1/slider/1/brown/middle.png" );
			Relocate_Image( attributes, "slider/grey_1/slider_right.png", "ground/green_1/slider/1/brown/right.png" );
		}
		// if V.1.7 and lower : change yoshi_1 hill_up to jungle_1 slider image paths
		if( engine_version < 31 )
		{
			Relocate_Image( attributes, "ground/yoshi_1/hill_up_1.png", "ground/jungle_1/slider/2_green_left.png" );
			Relocate_Image( attributes, "ground/yoshi_1/hill_up_2.png", "ground/jungle_1/slider/2_blue_left.png" );
			Relocate_Image( attributes, "ground/yoshi_1/hill_up_3.png", "ground/jungle_1/slider/2_brown_left.png" );
		}
		// if V.1.5 and lower : change pipe connection image paths
		if( engine_version < 28 )
		{
			Relocate_Image( attributes, "blocks/pipe/connection_left_down.png", "blocks/pipe/connection/plastic_1/orange/right_up.png" );
			Relocate_Image( attributes, "blocks/pipe/connection_left_up.png", "blocks/pipe/connection/plastic_1/orange/right_down.png" );
			Relocate_Image( attributes, "blocks/pipe/connection_right_down.png", "blocks/pipe/connection/plastic_1/orange/left_up.png" );
			Relocate_Image( attributes, "blocks/pipe/connection_right_up.png", "blocks/pipe/connection/plastic_1/orange/left_down.png" );
			Relocate_Image( attributes, "blocks/pipe/metal_connector.png", "blocks/pipe/connection/metal_1/grey/middle.png" );
		}

		// if V.1.4 and lower : change some image paths
		if( engine_version < 25 )
		{
			// change stone8 to metal stone 2 violet
			Relocate_Image( attributes, "game/box/stone8.png", "blocks/metal/stone_2_violet.png" );
			// move jungle_1 trees into a directory
			Relocate_Image( attributes, "ground/jungle_1/tree_type_1.png", "ground/jungle_1/tree/1.png" );
			Relocate_Image( attributes, "ground/jungle_1/tree_type_1_front.png", "ground/jungle_1/tree/1_front.png" );
			Relocate_Image( attributes, "ground/jungle_1/tree_type_2.png", "ground/jungle_1/tree/2.png" );
			// move yoshi_1 extra to jungle_2 hedge
			Relocate_Image( attributes, "ground/yoshi_1/extra_1_blue.png", "ground/jungle_2/hedge/1_blue.png" );
			Relocate_Image( attributes, "ground/yoshi_1/extra_1_green.png", "ground/jungle_2/hedge/1_green.png" );
			Relocate_Image( attributes, "ground/yoshi_1/extra_1_red.png", "ground/jungle_2/hedge/1_red.png" );
			Relocate_Image( attributes, "ground/yoshi_1/extra_1_yellow.png", "ground/jungle_2/hedge/1_yellow.png" );
			// move yoshi_1 rope to jungle_2
			Relocate_Image( attributes, "ground/yoshi_1/rope_1_leftright.png", "ground/jungle_2/rope_1_hor.png" );
		}

		cSprite *sprite = new cSprite( attributes, sprite_manager );

		// if image not available display its filename
		if( !sprite->m_start_image )
		{
			std::string text = attributes.getValueAsString( "image" ).c_str();

			if( text.empty() )
			{
				text = "Invalid image here";
			}

			cGL_Surface *text_image = pFont->Render_Text( pFont->m_font_small, text );
			text_image->m_filename = text;
			// set text image
			sprite->Set_Image( text_image, 1, 1 );
			// display it as front passive
			sprite->Set_Sprite_Type( TYPE_FRONT_PASSIVE );
			// only display it in the editor
			sprite->Set_Active( 0 );
		}

		// needs image
		if( sprite->m_image )
		{
			// if V.1.2 and lower : change pipe position
			if( engine_version < 22 )
			{
				if( sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/green/up.png" ) == 0 ||
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/green/ver.png" ) == 0 || 
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/green/down.png" ) == 0 || 
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/blue/up.png" ) == 0 ||
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/blue/ver.png" ) == 0 || 
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/blue/down.png" ) == 0 ||
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/yellow/up.png" ) == 0 ||
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/yellow/ver.png" ) == 0 || 
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/yellow/down.png" ) == 0 ||
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/grey/up.png" ) == 0 ||
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/grey/ver.png" ) == 0 || 
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/grey/down.png" ) == 0 )
				{
					sprite->Move( -6, 0, 1 );
					sprite->m_start_pos_x = sprite->m_pos_x;
				}
				else if( sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/green/right.png" ) == 0 ||
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/green/hor.png" ) == 0 || 
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/green/left.png" ) == 0 ||
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/blue/right.png" ) == 0 ||
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/blue/hor.png" ) == 0 || 
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/blue/left.png" ) == 0 ||
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/yellow/right.png" ) == 0 ||
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/yellow/hor.png" ) == 0 || 
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/yellow/left.png" ) == 0 ||
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/grey/right.png" ) == 0 ||
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/grey/hor.png" ) == 0 || 
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/grey/left.png" ) == 0 )
				{
					sprite->Move( 0, -6, 1 );
					sprite->m_start_pos_y = sprite->m_pos_y;
				}
			}
			// if V.1.2 and lower : change some hill positions
			if( engine_version < 23 )
			{
				if( sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "hills/green_1/head.png" ) == 0 ||
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "hills/light_blue_1/head.png" ) == 0 )
				{
					sprite->Move( 0, -6, 1 );
					sprite->m_start_pos_y = sprite->m_pos_y;
				}
			}
			// if V.1.7 and lower : change yoshi_1 hill_up to jungle_1 slider image paths
			if( engine_version < 31 )
			{
				// image filename is already changed but we need to add the middle and right tiles
				if( sprite_manager && ( sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "ground/jungle_1/slider/2_green_left.png" ) == 0 || 
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "ground/jungle_1/slider/2_blue_left.png" ) == 0 || 
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "ground/jungle_1/slider/2_brown_left.png" ) == 0 ) )
				{
					std::string color;

					// green
					if( sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "ground/jungle_1/slider/2_green_left.png" ) == 0 )
					{
						color = "green";
					}
					// blue
					else if( sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "ground/jungle_1/slider/2_blue_left.png" ) == 0 )
					{
						color = "blue";
					}
					// brown
					else
					{
						color = "brown";
					}

					cSprite *copy = sprite;

					// add middle tiles
					for( unsigned int i = 0; i < 4; i++ )
					{
						copy = copy->Copy();
						copy->Set_Image( pVideo->Get_Surface( "ground/jungle_1/slider/2_" + color + "_middle.png" ), 1 );
						copy->Set_Pos_X( copy->m_start_pos_x + 22, 1 );
						sprite_manager->Add( copy );
					}

					// add end tile
					copy = copy->Copy();
					copy->Set_Image( pVideo->Get_Surface( "ground/jungle_1/slider/2_" + color + "_right.png" ), 1 );
					copy->Set_Pos_X( copy->m_start_pos_x + 22, 1 );
					sprite_manager->Add( copy );
				}
			}
			// if V.1.7 and lower : change slider grey_1 to green_1 brown slider image paths
			if( engine_version < 32 )
			{
				// image filename is already changed but we need to add an additional middle tile for left and right
				if( sprite_manager && ( sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "ground/green_1/slider/1/brown/left.png" ) == 0 || 
					sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "ground/green_1/slider/1/brown/right.png" ) == 0 ) )
				{
					// add middle tile
					cSprite *copy = sprite->Copy();
					copy->Set_Image( pVideo->Get_Surface( "ground/green_1/slider/1/brown/middle.png" ), 1 );
					// if from left tile it must be moved
					if( sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "ground/green_1/slider/1/brown/left.png" ) == 0 )
					{
						copy->Set_Pos_X( copy->m_start_pos_x + 18, 1 );
					}
					sprite_manager->Add( copy );
				}
				// move right tile
				if( sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "ground/green_1/slider/1/brown/right.png" ) == 0 )
				{
					sprite->Move( 18, 0, 1 );
					sprite->m_start_pos_x = sprite->m_pos_x;
				}
			}
		}

		return sprite;
	}
	else if( element == "enemystopper" )
	{
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( engine_version < 35 )
		{
			if( attributes.exists( "posy" ) )
			{
				attributes.add( "posy", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "posy" ) - 600.0f ) );
			}
		}

		return new cEnemyStopper( attributes, sprite_manager );
	}
	else if( element == "levelexit" )
	{
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( engine_version < 35 )
		{
			if( attributes.exists( "posy" ) )
			{
				attributes.add( "posy", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "posy" ) - 600.0f ) );
			}
		}

		// if V.1.9 and lower : change "motion" to "camera_motion"
		if( engine_version < 36 )
		{
			if( attributes.exists( "motion" ) )
			{
				attributes.add( "camera_motion", CEGUI::PropertyHelper::intToString( attributes.getValueAsInteger( "motion" ) + 1 ) );
				attributes.remove( "motion" );
			}
		}

		return new cLevel_Exit( attributes, sprite_manager );
	}
	else if( element == "level_entry" )
	{
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( engine_version < 35 )
		{
			if( attributes.exists( "posy" ) )
			{
				attributes.add( "posy", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "posy" ) - 600.0f ) );
			}
		}

		return new cLevel_Entry( attributes, sprite_manager );
	}
	else if( element == "box" )
	{
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( engine_version < 35 )
		{
			if( attributes.exists( "posy" ) )
			{
				attributes.add( "posy", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "posy" ) - 600.0f ) );
			}
		}

		CEGUI::String str_type = attributes.getValueAsString( "type" );

		if( str_type == "bonus" )
		{
			return new cBonusBox( attributes, sprite_manager );
		}
		// gold is somewhere pre 0.99.5
		else if( str_type == "gold" )
		{
			// update old values
			attributes.add( "type", "bonus" );
			attributes.add( "animation", "Default" );
			attributes.add( "item", int_to_string( TYPE_GOLDPIECE ) );
			// renamed color to gold_color
			if( attributes.exists( "color" ) )
			{
				attributes.add( "gold_color", attributes.getValue( "color" ) );
				attributes.remove( "color" );
			}

			return new cBonusBox( attributes, sprite_manager );
		}
		else if( str_type == "spin" )
		{
			return new cSpinBox( attributes, sprite_manager );
		}
		else if( str_type == "text" )
		{
			return new cText_Box( attributes, sprite_manager );
		}
		// pre 0.99.4
		else if ( str_type == "empty" )
		{
			// update old values
			attributes.add( "type", "bonus" );
			attributes.add( "item", "0" );

			return new cBonusBox( attributes, sprite_manager );
		}
		// pre 0.99.4
		else if ( str_type == "invisible" )
		{
			// update old values
			attributes.add( "type", "bonus" );
			attributes.add( "item", "0" );
			attributes.add( "invisible", "1" );

			return new cBonusBox( attributes, sprite_manager );
		}
		else
		{
			printf( "Warning : Unknown Level Box type : %s\n", str_type.c_str() );
		}
	}
	// powerup is pre 0.99.5
	else if( element == "item" || element == "powerup" )
	{
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( engine_version < 35 )
		{
			if( attributes.exists( "posy" ) )
			{
				attributes.add( "posy", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "posy" ) - 600.0f ) );
			}
		}

		CEGUI::String str_type = attributes.getValueAsString( "type" );

		if( str_type == "goldpiece" )
		{
			return new cGoldpiece( attributes, sprite_manager );
		}
		else if( str_type == "mushroom" )
		{
			return new cMushroom( attributes, sprite_manager );
		}
		else if( str_type == "fireplant" )
		{
			return new cFirePlant( attributes, sprite_manager );
		}
		else if( str_type == "jstar" )
		{
			return new cjStar( attributes, sprite_manager );
		}
		else if( str_type == "moon" )
		{
			return new cMoon( attributes, sprite_manager );
		}
		else
		{
			printf( "Warning : Unknown Level Item type : %s\n", str_type.c_str() );
		}
	}
	else if( element == "moving_platform" )
	{
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( engine_version < 35 )
		{
			if( attributes.exists( "posy" ) )
			{
				attributes.add( "posy", CEGUI::PropertyHelper::floatToString(  attributes.getValueAsFloat( "posy" ) - 600.0f ) );
			}
		}
		// if V.1.7 and lower : change slider grey_1 to green_1 brown slider image paths
		if( engine_version < 32 )
		{
			Relocate_Image( attributes, "slider/grey_1/slider_left.png", "ground/green_1/slider/1/brown/left.png", "image_top_left" );
			Relocate_Image( attributes, "slider/grey_1/slider_middle.png", "ground/green_1/slider/1/brown/middle.png", "image_top_middle" );
			Relocate_Image( attributes, "slider/grey_1/slider_right.png", "ground/green_1/slider/1/brown/right.png", "image_top_right" );
		}

		cMoving_Platform *moving_platform = new cMoving_Platform( attributes, sprite_manager );

		// if V.1.7 and lower : change new slider middle count because start and end image is now half the width
		if( engine_version < 32 )
		{
			if( moving_platform->m_images[0].m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "ground/green_1/slider/1/brown/left.png" ) == 0 )
			{
				moving_platform->Set_Middle_Count( moving_platform->m_middle_count + 1 );
			}
			if( moving_platform->m_images[0].m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "ground/green_1/slider/1/brown/right.png" ) == 0 )
			{
				moving_platform->Set_Middle_Count( moving_platform->m_middle_count + 1 );
			}
		}

		return moving_platform;
	}
	// pre 1.5
	else if( element == "falling_platform" )
	{
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( engine_version < 35 )
		{
			if( attributes.exists( "posy" ) )
			{
				attributes.add( "posy", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "posy" ) - 600.0f ) );
			}
		}

		// no moving
		attributes.add( "speed", "0" );
		// renamed time_fall to touch_time and change to the new value
		if( attributes.exists( "time_fall" ) )
		{
			attributes.add( "touch_time", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "time_fall" ) * speedfactor_fps ) );
			attributes.remove( "time_fall" );
		}
		else
		{
			attributes.add( "touch_time", "48" );
		}
		// enable falling
		attributes.add( "shake_time", "12" );

		// if V.1.7 and lower : change slider grey_1 to green_1 brown slider image paths
		if( engine_version < 32 )
		{
			Relocate_Image( attributes, "slider/grey_1/slider_left.png", "ground/green_1/slider/1/brown/left.png", "image_top_left" );
			Relocate_Image( attributes, "slider/grey_1/slider_middle.png", "ground/green_1/slider/1/brown/middle.png", "image_top_middle" );
			Relocate_Image( attributes, "slider/grey_1/slider_right.png", "ground/green_1/slider/1/brown/right.png", "image_top_right" );
		}

		cMoving_Platform *moving_platform = new cMoving_Platform( attributes, sprite_manager );

		// if V.1.7 and lower : change new slider middle count because start and end image is now half the width
		if( engine_version < 32 )
		{
			if( moving_platform->m_images[0].m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "ground/green_1/slider/1/brown/left.png" ) == 0 )
			{
				moving_platform->Set_Middle_Count( moving_platform->m_middle_count + 1 );
			}
			if( moving_platform->m_images[0].m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "ground/green_1/slider/1/brown/right.png" ) == 0 )
			{
				moving_platform->Set_Middle_Count( moving_platform->m_middle_count + 1 );
			}
		}

		return moving_platform;
	}
	else if( element == "enemy" )
	{
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( engine_version < 35 )
		{
			if( attributes.exists( "posy" ) )
			{
				attributes.add( "posy", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "posy" ) - 600.0f ) );
			}
		}

		CEGUI::String str_type = attributes.getValueAsString( "type" );

		// if V.1.5 and lower
		if( engine_version < 26 )
		{
			// change gumba to furball
			if( str_type == "gumba" )
			{
				// change type
				str_type = "furball";
				attributes.add( "type", "furball" );
				// fix color : red was used in pre 1.0 but later was blue
				if( attributes.exists( "color" ) && attributes.getValueAsString( "color" ) == "red" )
				{
					attributes.add( "color", "blue" );
				}
			}
			// change rex to krush
			else if( str_type == "rex" )
			{
				// change type
				str_type = "krush";
				attributes.add( "type", "krush" );
			}
		}
		// if V.1.7 and lower
		if( engine_version < 29 )
		{
			// change jpiranha to flyon
			if( str_type == "jpiranha" )
			{
				// change type
				str_type = "flyon";
				attributes.add( "type", "flyon" );

				// change image dir
				if( attributes.exists( "image_dir" ) )
				{
					std::string img_dir = attributes.getValueAsString( "image_dir" ).c_str();

					std::string::size_type pos = img_dir.find( "jpiranha" );

					// change if found
					if( pos != std::string::npos )
					{
						img_dir.replace( pos, 8, "flyon" );
					}
				}
			}
		}

		if( str_type == "eato" )
		{
			return new cEato( attributes, sprite_manager );
		}
		else if( str_type == "furball" )
		{
			return new cFurball( attributes, sprite_manager );
		}
		else if( str_type == "turtle" )
		{
			return new cTurtle( attributes, sprite_manager );
		}
		else if( str_type == "turtleboss" )
		{
			// if V.1.5 and lower : max_downgrade_time changed to shell_time
			if( engine_version < 27 )
			{
				if( attributes.exists( "max_downgrade_time" ) )
				{
					attributes.add( "shell_time", attributes.getValueAsString( "max_downgrade_time" ) );
					attributes.remove( "max_downgrade_time" );
				}
			}

			return new cTurtleBoss( attributes, sprite_manager );
		}
		else if( str_type == "flyon" )
		{
			return new cFlyon( attributes, sprite_manager );
		}
		else if( str_type == "thromp" )
		{
			cThromp *thromp = new cThromp( attributes, sprite_manager );

			// if V.1.4 and lower : fix thromp distance was smaller
			if( engine_version < 25 )
			{
				thromp->Set_Max_Distance( thromp->m_max_distance + 36 );
			}

			return thromp;
		}
		else if( str_type == "rokko" )
		{
			return new cRokko( attributes, sprite_manager );
		}
		else if( str_type == "krush" )
		{
			return new cKrush( attributes, sprite_manager );
		}
		else if( str_type == "gee" )
		{
			return new cGee( attributes, sprite_manager );
		}
		else if( str_type == "spika" )
		{
			return new cSpika( attributes, sprite_manager );
		}
		else if( str_type == "static" )
		{
			return new cStaticEnemy( attributes, sprite_manager );
		}
		else if( str_type == "spikeball" )
		{
			return new cSpikeball( attributes, sprite_manager );
		}
		else
		{
			printf( "Warning : Unknown Level Enemy type : %s\n", str_type.c_str() );
		}
	}
	else if( element == "sound" )
	{
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( engine_version < 35 )
		{
			if( attributes.exists( "pos_y" ) )
			{
				attributes.add( "pos_y", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "pos_y" ) - 600.0f ) );
			}
		}

		return new cRandom_Sound( attributes, sprite_manager );
	}
	else if( element == "particle_emitter" )
	{
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( engine_version < 35 )
		{
			if( attributes.exists( "pos_y" ) )
			{
				attributes.add( "pos_y", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "pos_y" ) - 600.0f ) );
			}
		}

		cParticle_Emitter *particle_emitter = new cParticle_Emitter( attributes, sprite_manager );
		// set to not spawned
		particle_emitter->m_spawned = 0;
		// init animation
		particle_emitter->Init_Anim();

		return particle_emitter;
	}
	else if( element == "path" )
	{
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( engine_version < 35 )
		{
			if( attributes.exists( "posy" ) )
			{
				attributes.add( "posy", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "posy" ) - 600.0f ) );
			}
		}

		return new cPath( attributes, sprite_manager );
	}

	return NULL;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cLevel *pActive_Level = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
