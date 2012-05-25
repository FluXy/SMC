/***************************************************************************
 * preferences.cpp  -  Game settings handler
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

#include "../user/preferences.h"
#include "../audio/audio.h"
#include "../video/video.h"
#include "../core/game_core.h"
#include "../input/joystick.h"
#include "../gui/hud.h"
#include "../level/level_manager.h"
#include "../core/i18n.h"
#include "../core/filesystem/resource_manager.h"
#include "../core/filesystem/filesystem.h"
// CEGUI
#include "CEGUIXMLParser.h"
#include "CEGUIExceptions.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cPreferences *** *** *** *** *** *** *** *** *** *** */

// Game
const bool cPreferences::m_always_run_default = 0;
const std::string cPreferences::m_menu_level_default = "menu_green_1";
const float cPreferences::m_camera_hor_speed_default = 0.3f;
const float cPreferences::m_camera_ver_speed_default = 0.2f;
// Video
#ifdef _DEBUG
const bool cPreferences::m_video_fullscreen_default = 0;
#else
const bool cPreferences::m_video_fullscreen_default = 1;
#endif
const Uint16 cPreferences::m_video_screen_w_default = 1024;
const Uint16 cPreferences::m_video_screen_h_default = 768;
const Uint8 cPreferences::m_video_screen_bpp_default = 32;
/* disable by default because of possible bad drivers
 * which can't handle visual sync
*/
const bool cPreferences::m_video_vsync_default = 0;
const Uint16 cPreferences::m_video_fps_limit_default = 240;
// default geometry detail is medium
const float cPreferences::m_geometry_quality_default = 0.5f;
// default texture detail is high
const float cPreferences::m_texture_quality_default = 0.75f;
// Audio
const bool cPreferences::m_audio_music_default = 1;
const bool cPreferences::m_audio_sound_default = 1;
const unsigned int cPreferences::m_audio_hz_default = 44100;
const Uint8 cPreferences::m_sound_volume_default = 100;
const Uint8 cPreferences::m_music_volume_default = 80;
// Keyboard
const SDLKey cPreferences::m_key_up_default = SDLK_UP;
const SDLKey cPreferences::m_key_down_default = SDLK_DOWN;
const SDLKey cPreferences::m_key_left_default = SDLK_LEFT;
const SDLKey cPreferences::m_key_right_default = SDLK_RIGHT;
const SDLKey cPreferences::m_key_jump_default = SDLK_s;
const SDLKey cPreferences::m_key_shoot_default = SDLK_SPACE;
const SDLKey cPreferences::m_key_item_default = SDLK_RETURN;
const SDLKey cPreferences::m_key_action_default = SDLK_a;
const SDLKey cPreferences::m_key_screenshot_default = SDLK_PRINT;
const SDLKey cPreferences::m_key_editor_fast_copy_up_default = SDLK_KP8;
const SDLKey cPreferences::m_key_editor_fast_copy_down_default = SDLK_KP2;
const SDLKey cPreferences::m_key_editor_fast_copy_left_default = SDLK_KP4;
const SDLKey cPreferences::m_key_editor_fast_copy_right_default = SDLK_KP6;
const SDLKey cPreferences::m_key_editor_pixel_move_up_default = SDLK_KP8;
const SDLKey cPreferences::m_key_editor_pixel_move_down_default = SDLK_KP2;
const SDLKey cPreferences::m_key_editor_pixel_move_left_default = SDLK_KP4;
const SDLKey cPreferences::m_key_editor_pixel_move_right_default = SDLK_KP6;
const float cPreferences::m_scroll_speed_default = 1.0f;
// Joystick
const bool cPreferences::m_joy_enabled_default = 1;
const bool cPreferences::m_joy_analog_jump_default = 0;
const int cPreferences::m_joy_axis_hor_default = 0;
const int cPreferences::m_joy_axis_ver_default = 1;
const Sint16 cPreferences::m_joy_axis_threshold_default = 10000;
const Uint8 cPreferences::m_joy_button_jump_default = 0;
const Uint8 cPreferences::m_joy_button_shoot_default = 1;
const Uint8 cPreferences::m_joy_button_item_default = 3;
const Uint8 cPreferences::m_joy_button_action_default = 2;
const Uint8 cPreferences::m_joy_button_exit_default = 4;
// Editor
const bool cPreferences::m_editor_mouse_auto_hide_default = 0;
const bool cPreferences::m_editor_show_item_images_default = 1;
const unsigned int cPreferences::m_editor_item_image_size_default = 50;

cPreferences :: cPreferences( void )
{
	Reset_All();
}

cPreferences :: ~cPreferences( void )
{
	//
}

bool cPreferences :: Load( const std::string &filename /* = "" */ )
{
	Reset_All();
	
	// if config file is given
	if( filename.length() )
	{
		m_config_filename = filename;
	}

	// prefer local config file
	if( File_Exists( m_config_filename ) )
	{
		printf( "Using local preferences file : %s\n", m_config_filename.c_str() );
	}
	// user dir
	else
	{
		m_config_filename.insert( 0, pResource_Manager->user_data_dir );

		// does not exist in user dir
		if( !File_Exists( m_config_filename ) )
		{
			// only print warning if file is given
			if( !filename.empty() )
			{
				printf( "Couldn't open preferences file : %s\n", m_config_filename.c_str() );
			}
			return 0;
		}
	}

	try
	{
	// fixme : Workaround for std::string to CEGUI::String utf8 conversion. Check again if CEGUI 0.8 works with std::string utf8
	#ifdef _WIN32
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, (const CEGUI::utf8*)m_config_filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/Config.xsd", "" );
	#else
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, m_config_filename, DATA_DIR "/" GAME_SCHEMA_DIR "/Config.xsd", "" );
	#endif
	}
	// catch CEGUI Exceptions
	catch( CEGUI::Exception &ex )
	{
		printf( "Preferences Loading CEGUI Exception %s\n", ex.getMessage().c_str() );
		pHud_Debug->Set_Text( _("Preferences Loading failed : ") + (const std::string)ex.getMessage().c_str() );
	}

	// if user data dir is set
	if( !m_force_user_data_dir.empty() )
	{
		pResource_Manager->Set_User_Directory( m_force_user_data_dir );
	}

	return 1;
}

void cPreferences :: Save( void )
{
	Update();

// fixme : Check if there is a more portable way f.e. with imbue()
#ifdef _WIN32
	ofstream file( utf8_to_ucs2( m_config_filename ).c_str(), ios::out | ios::trunc );
#else
	ofstream file( m_config_filename.c_str(), ios::out | ios::trunc );
#endif

	if( !file.is_open() )
	{
		printf( "Error : couldn't open config %s for saving. Is the file read-only ?\n", m_config_filename.c_str() );
		return;
	}

	CEGUI::XMLSerializer stream( file );

	// begin
	stream.openTag( "config" );
	// Game
	Write_Property( stream, "game_version", int_to_string(SMC_VERSION_MAJOR) + "." + int_to_string(SMC_VERSION_MINOR) + "." + int_to_string(SMC_VERSION_PATCH) );
	Write_Property( stream, "game_language", m_language );
	Write_Property( stream, "game_always_run", m_always_run );
	Write_Property( stream, "game_menu_level", m_menu_level );
	Write_Property( stream, "game_user_data_dir", m_force_user_data_dir );
	Write_Property( stream, "game_camera_hor_speed", m_camera_hor_speed );
	Write_Property( stream, "game_camera_ver_speed", m_camera_ver_speed );
	// Video
	Write_Property( stream, "video_fullscreen", m_video_fullscreen );
	Write_Property( stream, "video_screen_w", m_video_screen_w );
	Write_Property( stream, "video_screen_h", m_video_screen_h );
	Write_Property( stream, "video_screen_bpp", static_cast<int>(m_video_screen_bpp) );
	Write_Property( stream, "video_vsync", m_video_vsync );
	Write_Property( stream, "video_fps_limit", m_video_fps_limit );
	Write_Property( stream, "video_geometry_quality", pVideo->m_geometry_quality );
	Write_Property( stream, "video_texture_quality", pVideo->m_texture_quality );
	// Audio
	Write_Property( stream, "audio_music", m_audio_music );
	Write_Property( stream, "audio_sound", m_audio_sound );
	Write_Property( stream, "audio_sound_volume", static_cast<int>(pAudio->m_sound_volume) );
	Write_Property( stream, "audio_music_volume", static_cast<int>(pAudio->m_music_volume) );
	Write_Property( stream, "audio_hz", m_audio_hz );
	// Keyboard
	Write_Property( stream, "keyboard_key_up", m_key_up );
	Write_Property( stream, "keyboard_key_down", m_key_down );
	Write_Property( stream, "keyboard_key_left", m_key_left );
	Write_Property( stream, "keyboard_key_right", m_key_right );
	Write_Property( stream, "keyboard_key_jump", m_key_jump );
	Write_Property( stream, "keyboard_key_shoot", m_key_shoot );
	Write_Property( stream, "keyboard_key_item", m_key_item );
	Write_Property( stream, "keyboard_key_action", m_key_action );
	Write_Property( stream, "keyboard_scroll_speed", m_scroll_speed );
	Write_Property( stream, "keyboard_key_screenshot", m_key_screenshot );
	Write_Property( stream, "keyboard_key_editor_fast_copy_up", m_key_editor_fast_copy_up );
	Write_Property( stream, "keyboard_key_editor_fast_copy_down", m_key_editor_fast_copy_down );
	Write_Property( stream, "keyboard_key_editor_fast_copy_left", m_key_editor_fast_copy_left );
	Write_Property( stream, "keyboard_key_editor_fast_copy_right", m_key_editor_fast_copy_right );
	Write_Property( stream, "keyboard_key_editor_pixel_move_up", m_key_editor_pixel_move_up );
	Write_Property( stream, "keyboard_key_editor_pixel_move_down", m_key_editor_pixel_move_down );
	Write_Property( stream, "keyboard_key_editor_pixel_move_left", m_key_editor_pixel_move_left );
	Write_Property( stream, "keyboard_key_editor_pixel_move_right", m_key_editor_pixel_move_right );
	// Joystick/Gamepad
	Write_Property( stream, "joy_enabled", m_joy_enabled );
	Write_Property( stream, "joy_name", m_joy_name );
	Write_Property( stream, "joy_analog_jump", m_joy_analog_jump );
	Write_Property( stream, "joy_axis_hor", m_joy_axis_hor );
	Write_Property( stream, "joy_axis_ver", m_joy_axis_ver );
	Write_Property( stream, "joy_axis_threshold", m_joy_axis_threshold );
	Write_Property( stream, "joy_button_jump", static_cast<int>(m_joy_button_jump) );
	Write_Property( stream, "joy_button_item", static_cast<int>(m_joy_button_item) );
	Write_Property( stream, "joy_button_shoot", static_cast<int>(m_joy_button_shoot) );
	Write_Property( stream, "joy_button_action", static_cast<int>(m_joy_button_action) );
	Write_Property( stream, "joy_button_exit", static_cast<int>(m_joy_button_exit) );
	// Special
	Write_Property( stream, "level_background_images", m_level_background_images );
	Write_Property( stream, "image_cache_enabled", m_image_cache_enabled );
	// Editor
	Write_Property( stream, "editor_mouse_auto_hide", m_editor_mouse_auto_hide );
	Write_Property( stream, "editor_show_item_images", m_editor_show_item_images );
	Write_Property( stream, "editor_item_image_size", m_editor_item_image_size );
	// end config
	stream.closeTag();

	file.close();
}

void cPreferences :: Reset_All( void )
{
	// Game
	m_game_version = smc_version;
	m_force_user_data_dir.clear();

	Reset_Game();
	Reset_Video();
	Reset_Audio();
	Reset_Keyboard();
	Reset_Joystick();
	Reset_Editor();

	// Special
	m_level_background_images = 1;
	m_image_cache_enabled = 1;

	// filename
	m_config_filename = "config.xml";
}

void cPreferences :: Reset_Game( void )
{
	m_language = "";
	m_always_run = m_always_run_default;
	m_menu_level = m_menu_level_default;
	m_camera_hor_speed = m_camera_hor_speed_default;
	m_camera_ver_speed = m_camera_ver_speed_default;
}

void cPreferences :: Reset_Video( void )
{
	// Video
	m_video_screen_w = m_video_screen_w_default;
	m_video_screen_h = m_video_screen_h_default;
	m_video_screen_bpp = m_video_screen_bpp_default;
	m_video_vsync = m_video_vsync_default;
	m_video_fps_limit = m_video_fps_limit_default;
	m_video_fullscreen = m_video_fullscreen_default;
	pVideo->m_geometry_quality = m_geometry_quality_default;
	pVideo->m_texture_quality = m_texture_quality_default;
}

void cPreferences :: Reset_Audio( void )
{
	// Audio
	m_audio_music = m_audio_music_default;
	m_audio_sound = m_audio_sound_default;
	m_audio_hz = m_audio_hz_default;
	pAudio->m_sound_volume = m_sound_volume_default;
	pAudio->m_music_volume = m_music_volume_default;
}

void cPreferences :: Reset_Keyboard( void )
{
	m_key_up = m_key_up_default;
	m_key_down = m_key_down_default;
	m_key_left = m_key_left_default;
	m_key_right = m_key_right_default;
	m_key_jump = m_key_jump_default;
	m_key_shoot = m_key_shoot_default;
	m_key_item = m_key_item_default;
	m_key_action = m_key_action_default;
	m_scroll_speed = m_scroll_speed_default;
	m_key_screenshot = m_key_screenshot_default;
	m_key_editor_fast_copy_up = m_key_editor_fast_copy_up_default;
	m_key_editor_fast_copy_down = m_key_editor_fast_copy_down_default;
	m_key_editor_fast_copy_left = m_key_editor_fast_copy_left_default;
	m_key_editor_fast_copy_right = m_key_editor_fast_copy_right_default;
	m_key_editor_pixel_move_up = m_key_editor_pixel_move_up_default;
	m_key_editor_pixel_move_down = m_key_editor_pixel_move_down_default;
	m_key_editor_pixel_move_left = m_key_editor_pixel_move_left_default;
	m_key_editor_pixel_move_right = m_key_editor_pixel_move_right_default;
}

void cPreferences :: Reset_Joystick( void )
{
	m_joy_enabled = m_joy_enabled_default;
	m_joy_name.clear();
	m_joy_analog_jump = m_joy_analog_jump_default;
	// axes
	m_joy_axis_hor = m_joy_axis_hor_default;
	m_joy_axis_ver = m_joy_axis_ver_default;
	// axis threshold
	m_joy_axis_threshold = m_joy_axis_threshold_default;
	// buttons
	m_joy_button_jump = m_joy_button_jump_default;
	m_joy_button_shoot = m_joy_button_shoot_default;
	m_joy_button_item = m_joy_button_item_default;
	m_joy_button_action = m_joy_button_action_default;
	m_joy_button_exit = m_joy_button_exit_default;
}

void cPreferences :: Reset_Editor( void )
{
	m_editor_mouse_auto_hide = m_editor_mouse_auto_hide_default;
	m_editor_show_item_images = m_editor_show_item_images_default;
	m_editor_item_image_size = m_editor_item_image_size_default;
}

void cPreferences :: Update( void )
{
	m_camera_hor_speed = pLevel_Manager->m_camera->m_hor_offset_speed;
	m_camera_ver_speed = pLevel_Manager->m_camera->m_ver_offset_speed;

	m_audio_music = pAudio->m_music_enabled;
	m_audio_sound = pAudio->m_sound_enabled;

	// if not default joy used
	if( pJoystick->m_current_joystick > 0 )
	{
		m_joy_name = pJoystick->Get_Name();
	}
	// using default joy
	else
	{
		m_joy_name.clear();
	}
}

void cPreferences :: Apply( void )
{
	pLevel_Manager->m_camera->m_hor_offset_speed = m_camera_hor_speed;
	pLevel_Manager->m_camera->m_ver_offset_speed = m_camera_ver_speed;
	
	// disable joystick if the joystick initialization failed
	if( pVideo->m_joy_init_failed )
	{
		m_joy_enabled = 0;
	}
}

void cPreferences :: Apply_Video( Uint16 screen_w, Uint16 screen_h, Uint8 screen_bpp, bool fullscreen, bool vsync, float geometry_detail, float texture_detail )
{
	/* if resolution, bpp, vsync or texture detail changed
	 * a texture reload is necessary
	*/
	if( m_video_screen_w != screen_w || m_video_screen_h != screen_h || m_video_screen_bpp != screen_bpp || m_video_vsync != vsync || !Is_Float_Equal( pVideo->m_texture_quality, texture_detail ) )
	{
		// new settings
		m_video_screen_w = screen_w;
		m_video_screen_h = screen_h;
		m_video_screen_bpp = screen_bpp;
		m_video_vsync = vsync;
		m_video_fullscreen = fullscreen;
		pVideo->m_texture_quality = texture_detail;
		pVideo->m_geometry_quality = geometry_detail;

		// reinitialize video and reload textures from file
		pVideo->Init_Video( 1 );
	}
	// no texture reload necessary
	else
	{
		// geometry detail changed
		if( !Is_Float_Equal( pVideo->m_geometry_quality, geometry_detail ) )
		{
			pVideo->m_geometry_quality = geometry_detail;
			pVideo->Init_Geometry();
		}

		// fullscreen changed
		if( m_video_fullscreen != fullscreen )
		{
			// toggle fullscreen and switches video_fullscreen itself
			pVideo->Toggle_Fullscreen();
		}
	}
}

void cPreferences :: Apply_Audio( bool sound, bool music )
{
	// disable sound and music if the audio initialization failed
	if( pVideo->m_audio_init_failed )
	{
		m_audio_sound = 0;
		m_audio_music = 0;
		return;
	}

	m_audio_sound = sound;
	m_audio_music = music;

	// init audio settings
	pAudio->Init();
}

void cPreferences :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
{
	if( element == "property" || element == "Item" )
	{
		handle_item( attributes );
	}
}

void cPreferences :: elementEnd( const CEGUI::String &element )
{
	
}

void cPreferences :: handle_item( CEGUI::XMLAttributes attributes )
{
	std::string name;

	if( attributes.exists( "name" ) )
	{
		name = attributes.getValueAsString( "name" ).c_str();
	}
	// V.1.9 and lower
	else
	{
		name = attributes.getValueAsString( "Name" ).c_str();
		attributes.add( "value", attributes.getValueAsString( "Value" ) );
	}

	// Game
	if( name.compare( "game_version" ) == 0 )
	{
		m_game_version = string_to_version_number( attributes.getValueAsString( "value" ).c_str() );
	}
	else if( name.compare( "game_language" ) == 0 )
	{
		m_language = attributes.getValueAsString( "value" ).c_str();
	}
	else if( name.compare( "game_always_run" ) == 0 || name.compare( "always_run" ) == 0 )
	{
		m_always_run = attributes.getValueAsBool( "value" );
	}
	else if( name.compare( "game_menu_level" ) == 0 )
	{
		m_menu_level = attributes.getValueAsString( "value" ).c_str();
	}
	else if( name.compare( "game_user_data_dir" ) == 0 || name.compare( "user_data_dir" ) == 0 )
	{
		m_force_user_data_dir = attributes.getValueAsString( "value" ).c_str();

		// if user data dir is set
		if( !m_force_user_data_dir.empty() ) 
		{
			Convert_Path_Separators( m_force_user_data_dir );

			// add trailing slash if missing
			if( *(m_force_user_data_dir.end() - 1) != '/' )
			{
				m_force_user_data_dir.insert( m_force_user_data_dir.length(), "/" );
			}
		}
	}
	else if( name.compare( "game_camera_hor_speed" ) == 0 || name.compare( "camera_hor_speed" ) == 0 )
	{
		m_camera_hor_speed = attributes.getValueAsFloat( "value" );
	}
	else if( name.compare( "game_camera_ver_speed" ) == 0 || name.compare( "camera_ver_speed" ) == 0 )
	{
		m_camera_ver_speed = attributes.getValueAsFloat( "value" );
	}
	// Video
	else if( name.compare( "video_screen_h" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val < 200 )
		{
			val = 200;
		}
		else if( val > 2560 )
		{
			val = 2560;
		}
		
		m_video_screen_h = val;
	}
	else if( name.compare( "video_screen_w" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val < 200 )
		{
			val = 200;
		}
		else if( val > 2560 )
		{
			val = 2560;
		}

		m_video_screen_w = val;
	}
	else if( name.compare( "video_screen_bpp" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val < 8 )
		{
			val = 8;
		}
		else if( val > 32 )
		{
			val = 32;
		}

		m_video_screen_bpp = val;
	}
	else if( name.compare( "video_vsync" ) == 0 )
	{
		m_video_vsync = attributes.getValueAsBool( "value" );
	}
	else if( name.compare( "video_fps_limit" ) == 0 )
	{
		m_video_fps_limit = attributes.getValueAsInteger( "value" );
	}
	else if( name.compare( "video_fullscreen" ) == 0 )
	{
		m_video_fullscreen = attributes.getValueAsBool( "value" );
	}
	else if( name.compare( "video_geometry_detail" ) == 0 || name.compare( "video_geometry_quality" ) == 0 )
	{
		pVideo->m_geometry_quality = attributes.getValueAsFloat( "value" );
	}
	else if( name.compare( "video_texture_detail" ) == 0 || name.compare( "video_texture_quality" ) == 0 )
	{
		pVideo->m_texture_quality = attributes.getValueAsFloat( "value" );
	}
	// Audio
	else if( name.compare( "audio_music" ) == 0 )
	{
		m_audio_music = attributes.getValueAsBool( "value" );
	}
	else if( name.compare( "audio_sound" ) == 0 )
	{
		m_audio_sound = attributes.getValueAsBool( "value" );
	}
	if( name.compare( "audio_music_volume" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= MIX_MAX_VOLUME )
		{
			pAudio->m_music_volume = val;
		}
	}
	else if( name.compare( "audio_sound_volume" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= MIX_MAX_VOLUME )
		{
			pAudio->m_sound_volume = val;
		}
	}
	else if( name.compare( "audio_hz" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= 96000 )
		{
			m_audio_hz = val;
		}
	}
	// Keyboard
	else if( name.compare( "keyboard_key_up" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_up = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_down" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_down = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_left" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_left = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_right" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_right = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_jump" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_jump = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_shoot" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_shoot = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_item" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_item = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_action" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_action = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_scroll_speed" ) == 0 )
	{
		m_scroll_speed = attributes.getValueAsFloat( "value" );
	}
	else if( name.compare( "keyboard_key_screenshot" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_screenshot = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_editor_fast_copy_up" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_editor_fast_copy_up = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_editor_fast_copy_down" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_editor_fast_copy_down = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_editor_fast_copy_left" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_editor_fast_copy_left = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_editor_fast_copy_right" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_editor_fast_copy_right = static_cast<SDLKey>(val);
		}
	}
		else if( name.compare( "keyboard_key_editor_pixel_move_up" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_editor_pixel_move_up = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_editor_pixel_move_down" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_editor_pixel_move_down = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_editor_pixel_move_left" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_editor_pixel_move_left = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_editor_pixel_move_right" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_editor_pixel_move_right = static_cast<SDLKey>(val);
		}
	}
	// Joypad
	else if( name.compare( "joy_enabled" ) == 0 )
	{
		m_joy_enabled = attributes.getValueAsBool( "value" );
	}
	else if( name.compare( "joy_name" ) == 0 )
	{
		m_joy_name = attributes.getValueAsString( "value" ).c_str();
	}
	else if( name.compare( "joy_analog_jump" ) == 0 )
	{
		m_joy_analog_jump = attributes.getValueAsBool( "value" );
	}
	else if( name.compare( "joy_axis_hor" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= 256 )
		{
			m_joy_axis_hor = val;
		}
	}
	else if( name.compare( "joy_axis_ver" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= 256 )
		{
			m_joy_axis_ver = val;
		}
	}
	else if( name.compare( "joy_axis_threshold" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= 32767 )
		{
			m_joy_axis_threshold = val;
		}
	}
	else if( name.compare( "joy_button_jump" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= 256 )
		{
			m_joy_button_jump = val;
		}
	}
	else if( name.compare( "joy_button_item" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= 256 )
		{
			m_joy_button_item = val;
		}
	}
	else if( name.compare( "joy_button_shoot" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= 256 )
		{
			m_joy_button_shoot = val;
		}
	}
	else if( name.compare( "joy_button_action" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= 256 )
		{
			m_joy_button_action = val;
		}
	}
	else if( name.compare( "joy_button_exit" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "value" );

		if( val >= 0 && val <= 256 )
		{
			m_joy_button_exit = val;
		}
	}
	// Special
	else if( name.compare( "level_background_images" ) == 0 )
	{
		m_level_background_images = attributes.getValueAsBool( "value" );
	}
	else if( name.compare( "image_cache_enabled" ) == 0 )
	{
		m_image_cache_enabled = attributes.getValueAsBool( "value" );
	}
	// Editor
	else if( name.compare( "editor_mouse_auto_hide" ) == 0 )
	{
		m_editor_mouse_auto_hide = attributes.getValueAsBool( "value" );
	}
	else if( name.compare( "editor_show_item_images" ) == 0 )
	{
		m_editor_show_item_images = attributes.getValueAsBool( "value" );
	}
	else if( name.compare( "editor_item_image_size" ) == 0 )
	{
		m_editor_item_image_size = attributes.getValueAsInteger( "value" );
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cPreferences *pPreferences = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
