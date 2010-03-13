/***************************************************************************
 * preferences.cpp  -  Game settings handler
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
// boost filesystem
#include "boost/filesystem/convenience.hpp"
namespace fs = boost::filesystem;
// CEGUI
#include "CEGUIXMLParser.h"

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
		/* fixme : this crashes in CEGUI::DefaultResourceProvider::loadRawDataContainer because of the é
		 * The CEGUI string encoding is UTF-8 but std::string seems to be ISO-8859-1 or Windows-1252
		*/
		//config_filename.insert( 0, "N:/Dokumente und Einstellungen/smc_Invité/Anwendungsdaten/smc/" );

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
		//CEGUI::String str = "N:/Dokumente und Einstellungen/smc_Invité/Anwendungsdaten/smc/config.xml";
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, m_config_filename, DATA_DIR "/" GAME_SCHEMA_DIR "/Config.xsd", "" );
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

	ofstream file( m_config_filename.c_str(), ios::out );

	if( !file.is_open() )
	{
		printf( "Error : couldn't open config %s for saving. Is the file read-only ?\n", m_config_filename.c_str() );
		return;
	}

	// xml info
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	// begin preferences
	file << "<Preferences>" << std::endl;
	// Game
	file << "\t<Item Name=\"game_version\" Value=\"" << smc_version << "\" />" << std::endl;
	file << "\t<Item Name=\"game_language\" Value=\"" << m_language << "\" />" << std::endl;
	file << "\t<Item Name=\"game_always_run\" Value=\"" << m_always_run << "\" />" << std::endl;
	file << "\t<Item Name=\"game_menu_level\" Value=\"" << string_to_xml_string( m_menu_level ) << "\" />" << std::endl;
	file << "\t<Item Name=\"game_user_data_dir\" Value=\"" << string_to_xml_string( m_force_user_data_dir ) << "\" />" << std::endl;
	file << "\t<Item Name=\"game_camera_hor_speed\" Value=\"" << m_camera_hor_speed << "\" />" << std::endl;
	file << "\t<Item Name=\"game_camera_ver_speed\" Value=\"" << m_camera_ver_speed << "\" />" << std::endl;
	// Video
	file << "\t<Item Name=\"video_fullscreen\" Value=\"" << m_video_fullscreen << "\" />" << std::endl;
	file << "\t<Item Name=\"video_screen_w\" Value=\"" << m_video_screen_w << "\" />" << std::endl;
	file << "\t<Item Name=\"video_screen_h\" Value=\"" << m_video_screen_h << "\" />" << std::endl;
	file << "\t<Item Name=\"video_screen_bpp\" Value=\"" << static_cast<int>(m_video_screen_bpp) << "\" />" << std::endl;
	file << "\t<Item Name=\"video_vsync\" Value=\"" << m_video_vsync << "\" />" << std::endl;
	file << "\t<Item Name=\"video_geometry_quality\" Value=\"" << pVideo->m_geometry_quality << "\" />" << std::endl;
	file << "\t<Item Name=\"video_texture_quality\" Value=\"" << pVideo->m_texture_quality << "\" />" << std::endl;
	// Audio
	file << "\t<Item Name=\"audio_music\" Value=\"" << m_audio_music << "\" />" << std::endl;
	file << "\t<Item Name=\"audio_sound\" Value=\"" << m_audio_sound << "\" />" << std::endl;
	file << "\t<Item Name=\"audio_sound_volume\" Value=\"" << static_cast<int>(pAudio->m_sound_volume) << "\" />" << std::endl;
	file << "\t<Item Name=\"audio_music_volume\" Value=\"" << static_cast<int>(pAudio->m_music_volume) << "\" />" << std::endl;
	file << "\t<Item Name=\"audio_hz\" Value=\"" << m_audio_hz << "\" />" << std::endl;
	// Keyboard
	file << "\t<Item Name=\"keyboard_key_up\" Value=\"" << m_key_up << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_down\" Value=\"" << m_key_down << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_left\" Value=\"" << m_key_left << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_right\" Value=\"" << m_key_right << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_jump\" Value=\"" << m_key_jump << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_shoot\" Value=\"" << m_key_shoot << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_item\" Value=\"" << m_key_item << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_action\" Value=\"" << m_key_action << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_scroll_speed\" Value=\"" << m_scroll_speed << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_screenshot\" Value=\"" << m_key_screenshot << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_editor_fast_copy_up\" Value=\"" << m_key_editor_fast_copy_up << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_editor_fast_copy_down\" Value=\"" << m_key_editor_fast_copy_down << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_editor_fast_copy_left\" Value=\"" << m_key_editor_fast_copy_left << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_editor_fast_copy_right\" Value=\"" << m_key_editor_fast_copy_right << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_editor_pixel_move_up\" Value=\"" << m_key_editor_pixel_move_up << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_editor_pixel_move_down\" Value=\"" << m_key_editor_pixel_move_down << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_editor_pixel_move_left\" Value=\"" << m_key_editor_pixel_move_left << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_editor_pixel_move_right\" Value=\"" << m_key_editor_pixel_move_right << "\" />" << std::endl;
	// Joystick/Gamepad
	file << "\t<Item Name=\"joy_enabled\" Value=\"" << m_joy_enabled << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_name\" Value=\"" << string_to_xml_string( m_joy_name ) << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_analog_jump\" Value=\"" << m_joy_analog_jump << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_axis_hor\" Value=\"" << m_joy_axis_hor << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_axis_ver\" Value=\"" << m_joy_axis_ver << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_axis_threshold\" Value=\"" << m_joy_axis_threshold << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_button_jump\" Value=\"" << static_cast<int>(m_joy_button_jump) << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_button_item\" Value=\"" << static_cast<int>(m_joy_button_item) << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_button_shoot\" Value=\"" << static_cast<int>(m_joy_button_shoot) << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_button_action\" Value=\"" << static_cast<int>(m_joy_button_action) << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_button_exit\" Value=\"" << static_cast<int>(m_joy_button_exit) << "\" />" << std::endl;
	// Special
	file << "\t<Item Name=\"level_background_images\" Value=\"" << m_level_background_images << "\" />" << std::endl;
	file << "\t<Item Name=\"image_cache_enabled\" Value=\"" << m_image_cache_enabled << "\" />" << std::endl;
	// Editor
	file << "\t<Item Name=\"editor_mouse_auto_hide\" Value=\"" << m_editor_mouse_auto_hide << "\" />" << std::endl;
	file << "\t<Item Name=\"editor_show_item_images\" Value=\"" << m_editor_show_item_images << "\" />" << std::endl;
	file << "\t<Item Name=\"editor_item_image_size\" Value=\"" << m_editor_item_image_size << "\" />" << std::endl;
	// end preferences
	file << "</Preferences>" << std::endl;

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

// XML element start
void cPreferences :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
{
	if( element == "Item" )
	{
		handle_item( attributes );
	}
}

// XML element end
void cPreferences :: elementEnd( const CEGUI::String &element )
{
	
}

void cPreferences :: handle_item( const CEGUI::XMLAttributes& attributes )
{
	std::string name = attributes.getValueAsString( "Name" ).c_str();

	// Game
	if( name.compare( "game_version" ) == 0 )
	{
		m_game_version = attributes.getValueAsFloat( "Value" );
	}
	else if( name.compare( "game_language" ) == 0 )
	{
		m_language = attributes.getValueAsString( "Value" ).c_str();
	}
	else if( name.compare( "game_always_run" ) == 0 || name.compare( "always_run" ) == 0 )
	{
		m_always_run = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "game_menu_level" ) == 0 )
	{
		m_menu_level = attributes.getValueAsString( "Value" ).c_str();
	}
	else if( name.compare( "game_user_data_dir" ) == 0 || name.compare( "user_data_dir" ) == 0 )
	{
		m_force_user_data_dir = attributes.getValueAsString( "Value" ).c_str();

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
		m_camera_hor_speed = attributes.getValueAsFloat( "Value" );
	}
	else if( name.compare( "game_camera_ver_speed" ) == 0 || name.compare( "camera_ver_speed" ) == 0 )
	{
		m_camera_ver_speed = attributes.getValueAsFloat( "Value" );
	}
	// Video
	else if( name.compare( "video_screen_h" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

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
		int val = attributes.getValueAsInteger( "Value" );

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
		int val = attributes.getValueAsInteger( "Value" );

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
		m_video_vsync = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "video_fullscreen" ) == 0 )
	{
		m_video_fullscreen = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "video_geometry_detail" ) == 0 || name.compare( "video_geometry_quality" ) == 0 )
	{
		pVideo->m_geometry_quality = attributes.getValueAsFloat( "Value" );
	}
	else if( name.compare( "video_texture_detail" ) == 0 || name.compare( "video_texture_quality" ) == 0 )
	{
		pVideo->m_texture_quality = attributes.getValueAsFloat( "Value" );
	}
	// Audio
	else if( name.compare( "audio_music" ) == 0 )
	{
		m_audio_music = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "audio_sound" ) == 0 )
	{
		m_audio_sound = attributes.getValueAsBool( "Value" );
	}
	if( name.compare( "audio_music_volume" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= MIX_MAX_VOLUME )
		{
			pAudio->m_music_volume = val;
		}
	}
	else if( name.compare( "audio_sound_volume" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= MIX_MAX_VOLUME )
		{
			pAudio->m_sound_volume = val;
		}
	}
	else if( name.compare( "audio_hz" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 96000 )
		{
			m_audio_hz = val;
		}
	}
	// Keyboard
	else if( name.compare( "keyboard_key_up" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_up = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_down" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_down = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_left" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_left = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_right" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_right = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_jump" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_jump = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_shoot" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_shoot = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_item" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_item = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_action" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_action = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_scroll_speed" ) == 0 )
	{
		m_scroll_speed = attributes.getValueAsFloat( "Value" );
	}
	else if( name.compare( "keyboard_key_screenshot" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_screenshot = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_editor_fast_copy_up" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_editor_fast_copy_up = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_editor_fast_copy_down" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_editor_fast_copy_down = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_editor_fast_copy_left" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_editor_fast_copy_left = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_editor_fast_copy_right" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_editor_fast_copy_right = static_cast<SDLKey>(val);
		}
	}
		else if( name.compare( "keyboard_key_editor_pixel_move_up" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_editor_pixel_move_up = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_editor_pixel_move_down" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_editor_pixel_move_down = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_editor_pixel_move_left" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_editor_pixel_move_left = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "keyboard_key_editor_pixel_move_right" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= SDLK_LAST )
		{
			m_key_editor_pixel_move_right = static_cast<SDLKey>(val);
		}
	}
	// Joypad
	else if( name.compare( "joy_enabled" ) == 0 )
	{
		m_joy_enabled = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "joy_name" ) == 0 )
	{
		m_joy_name = attributes.getValueAsString( "Value" ).c_str();
	}
	else if( name.compare( "joy_analog_jump" ) == 0 )
	{
		m_joy_analog_jump = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "joy_axis_hor" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 256 )
		{
			m_joy_axis_hor = val;
		}
	}
	else if( name.compare( "joy_axis_ver" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 256 )
		{
			m_joy_axis_ver = val;
		}
	}
	else if( name.compare( "joy_axis_threshold" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 32767 )
		{
			m_joy_axis_threshold = val;
		}
	}
	else if( name.compare( "joy_button_jump" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 256 )
		{
			m_joy_button_jump = val;
		}
	}
	else if( name.compare( "joy_button_item" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 256 )
		{
			m_joy_button_item = val;
		}
	}
	else if( name.compare( "joy_button_shoot" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 256 )
		{
			m_joy_button_shoot = val;
		}
	}
	else if( name.compare( "joy_button_action" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 256 )
		{
			m_joy_button_action = val;
		}
	}
	else if( name.compare( "joy_button_exit" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 256 )
		{
			m_joy_button_exit = val;
		}
	}
	// Special
	else if( name.compare( "level_background_images" ) == 0 )
	{
		m_level_background_images = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "image_cache_enabled" ) == 0 )
	{
		m_image_cache_enabled = attributes.getValueAsBool( "Value" );
	}
	// Editor
	else if( name.compare( "editor_mouse_auto_hide" ) == 0 )
	{
		m_editor_mouse_auto_hide = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "editor_show_item_images" ) == 0 )
	{
		m_editor_show_item_images = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "editor_item_image_size" ) == 0 )
	{
		m_editor_item_image_size = attributes.getValueAsInteger( "Value" );
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cPreferences *pPreferences = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
