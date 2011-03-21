/***************************************************************************
 * overworld.cpp  -  Overworld class
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

#include "../core/global_basic.h"
#include "../overworld/overworld.h"
#include "../audio/audio.h"
#include "../core/game_core.h"
#include "../level/level_editor.h"
#include "../overworld/world_editor.h"
#include "../core/framerate.h"
#include "../gui/menu.h"
#include "../user/preferences.h"
#include "../video/font.h"
#include "../input/mouse.h"
#include "../input/joystick.h"
#include "../input/keyboard.h"
#include "../level/level.h"
#include "../core/i18n.h"
#include "../core/filesystem/filesystem.h"
#include "../core/filesystem/resource_manager.h"
// CEGUI
#include "CEGUIXMLParser.h"
#include "CEGUIXMLAttributes.h"
#include "CEGUIExceptions.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** cOverworld_description *** *** *** *** *** *** *** *** *** */

cOverworld_description :: cOverworld_description( void )
{
	m_path = "world_1";
	m_name = _("Unnamed");
	m_visible = 1;
	m_user = 0;

	m_comment = _("Empty");
}

cOverworld_description :: ~cOverworld_description( void )
{
	//
}

void cOverworld_description :: Load( void )
{
	std::string filename = Get_Full_Path() + "/description.xml";

	// filename not valid
	if( !File_Exists( filename ) )
	{
		printf( "Error : Couldn't open World description file : %s\n", filename.c_str() );
		return;
	}

	// Load Description
// fixme : Workaround for std::string to CEGUI::String utf8 conversion. Check again if CEGUI 0.8 works with std::string utf8
#ifdef _WIN32
	CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, (const CEGUI::utf8*)filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/World/Description.xsd", "" );
#else
	CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/World/Description.xsd", "" );
#endif
}

void cOverworld_description :: Save( void )
{
	std::string save_dir = pResource_Manager->user_data_dir + USER_WORLD_DIR + "/" + m_path;
	std::string filename = save_dir + "/description.xml";

// fixme : Check if there is a more portable way f.e. with imbue()
#ifdef _WIN32
	ofstream file( utf8_to_ucs2( filename ).c_str(), ios::out | ios::trunc );
#else
	ofstream file( filename.c_str(), ios::out | ios::trunc );
#endif


	if( !file )
	{
		pHud_Debug->Set_Text( _("Couldn't save world description ") + filename, speedfactor_fps * 5.0f );
		return;
	}

	CEGUI::XMLSerializer stream( file );

	// begin 
	stream.openTag( "description" );

	// begin
	stream.openTag( "world" );
		// name
		Write_Property( stream, "name", m_name );
		// visible
		Write_Property( stream, "visible", m_visible );
	// end world
	stream.closeTag();


	// end description
	stream.closeTag();

	file.close();
}

std::string cOverworld_description :: Get_Full_Path( void ) const
{
	// return user world if available
	if( File_Exists( pResource_Manager->user_data_dir + USER_WORLD_DIR + "/" + m_path + "/description.xml" ) )
	{
		return pResource_Manager->user_data_dir + USER_WORLD_DIR + "/" + m_path;
	}

	// return game world
	return DATA_DIR "/" GAME_OVERWORLD_DIR "/" + m_path;
}

void cOverworld_description :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
{
	if( element == "property" )
	{
		m_xml_attributes.add( attributes.getValueAsString( "name" ), attributes.getValueAsString( "value" ) );
	}
	else if( element == "Property" )
	{
		m_xml_attributes.add( attributes.getValueAsString( "Name" ), attributes.getValueAsString( "Value" ) );
	}
}

void cOverworld_description :: elementEnd( const CEGUI::String &element )
{
	if( element == "property" || element == "Property" )
	{
		return;
	}

	if( element == "world" || element == "World" )
	{
		handle_world( m_xml_attributes );
	}
	else if( element == "description" || element == "Description" )
	{
		// ignore
	}
	else if( element.length() )
	{
		printf( "Warning : World Description Unknown element : %s\n", element.c_str() );
	}

	// clear
	m_xml_attributes = CEGUI::XMLAttributes();
}

void cOverworld_description :: handle_world( const CEGUI::XMLAttributes &attributes )
{
	m_name = xml_string_to_string( attributes.getValueAsString( "name", m_name.c_str() ).c_str() );
	m_visible = attributes.getValueAsBool( "visible", 1 );
}

/* *** *** *** *** *** *** *** *** cOverworld *** *** *** *** *** *** *** *** *** */

cOverworld :: cOverworld( void )
{
	m_sprite_manager = new cWorld_Sprite_Manager( this );
	m_animation_manager = new cAnimation_Manager();
	m_description = new cOverworld_description();
	m_layer = new cLayer( this );

	m_engine_version = -1;
	m_last_saved = 0;
	m_background_color = Color();
	m_musicfile = "overworld/land_1.ogg";
	m_hud_world_name = new cHudSprite( m_sprite_manager );
	m_hud_world_name->Set_Pos( 10, static_cast<float>(game_res_h) - 30 );
	m_hud_world_name->Set_Shadow( black, 1.5f );
	m_hud_level_name = new cHudSprite( m_sprite_manager );
	m_hud_level_name->Set_Pos( 350, 2 );
	m_hud_level_name->Set_Shadow( black, 1.5f );

	m_next_level = 0;

	m_player_start_waypoint = 0;
	m_player_moving_state = STA_STAY;
}

cOverworld :: ~cOverworld( void )
{
	Unload();

	delete m_sprite_manager;
	delete m_animation_manager;
	delete m_description;
	delete m_layer;
	delete m_hud_level_name;
	delete m_hud_world_name;
}

bool cOverworld :: New( std::string name )
{
	if( name.empty() )
	{
		return 0;
	}
	
	Unload();

	// set path
	m_description->m_path = name;
	// default name is the path
	m_description->m_name = name;

	m_background_color = Color( 0.2f, 0.5f, 0.1f );
	m_engine_version = world_engine_version;

	return 1;
}

bool cOverworld :: Load( void )
{
	Unload();

	// description
	m_description->Load();

	// world
	std::string world_filename = m_description->Get_Full_Path() + "/world.xml";

	if( !File_Exists( world_filename ) )
	{
		printf( "Couldn't find World file : %s from %s\n", world_filename.c_str(), m_description->m_path.c_str() );
		return 0;
	}

	try
	{
		// parse overworld
	// fixme : Workaround for std::string to CEGUI::String utf8 conversion. Check again if CEGUI 0.8 works with std::string utf8
	#ifdef _WIN32
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, (const CEGUI::utf8*)world_filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/World/World.xsd", "" );
	#else
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, world_filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/World/World.xsd", "" );	
	#endif
	}
	// catch CEGUI Exceptions
	catch( CEGUI::Exception &ex )
	{
		printf( "Loading World %s CEGUI Exception %s\n", world_filename.c_str(), ex.getMessage().c_str() );
		pHud_Debug->Set_Text( _("World Loading failed : ") + (const std::string)ex.getMessage().c_str() );
	}

	// engine version entry not set
	if( m_engine_version < 0 )
	{
		m_engine_version = 0;
	}

	// layer
	std::string layer_filename = m_description->Get_Full_Path() + "/layer.xml";

	if( !File_Exists( layer_filename ) )
	{
		printf( "Couldn't find World Layer file : %s from %s\n", layer_filename.c_str(), m_description->m_path.c_str() );
		return 0;
	}

	m_layer->Load( layer_filename );

	// set name
	m_hud_world_name->Set_Image( pFont->Render_Text( pFont->m_font_normal, m_description->m_name, yellow ), 1, 1 );

	return 1;
}

void cOverworld :: Unload( void )
{
	// not loaded
	if( !Is_Loaded() )
	{
		return;
	}

	// Objects
	m_sprite_manager->Delete_All();
	// Waypoints
	m_waypoints.clear();
	// Layer
	m_layer->Delete_All();
	// animations
	m_animation_manager->Delete_All();

	// no engine version
	m_engine_version = -1;
	m_last_saved = 0;
}

void cOverworld :: Save( void )
{
	pAudio->Play_Sound( "editor/save.ogg" );

	std::string save_dir = pResource_Manager->user_data_dir + USER_WORLD_DIR + "/" + m_description->m_path;
	// Create directory if new world
	if( !Dir_Exists( save_dir ) )
	{
		Create_Directory( save_dir );
	}

	std::string filename = save_dir + "/world.xml";

// fixme : Check if there is a more portable way f.e. with imbue()
#ifdef _WIN32
	ofstream file( utf8_to_ucs2( filename ).c_str(), ios::out | ios::trunc );
#else
	ofstream file( filename.c_str(), ios::out | ios::trunc );
#endif


	if( !file )
	{
		printf( "Error : Couldn't open world file for saving. Is the file read-only ?" );
		pHud_Debug->Set_Text( _("Couldn't save world ") + filename, speedfactor_fps * 5.0f );
		return;
	}

	CEGUI::XMLSerializer stream( file );


	// begin
	stream.openTag( "overworld" );

	// begin
	stream.openTag( "information" );
		// game version
		Write_Property( stream, "game_version", int_to_string(SMC_VERSION_MAJOR) + "." + int_to_string(SMC_VERSION_MINOR) + "." + int_to_string(SMC_VERSION_PATCH) );
		// engine version
		Write_Property( stream, "engine_version", world_engine_version );
		// time ( seconds since 1970 )
		Write_Property( stream, "save_time", static_cast<Uint64>( time( NULL ) ) );
	// end information
	stream.closeTag();

	// begin
	stream.openTag( "settings" );
		// music
		Write_Property( stream, "music", m_musicfile );
	// end settings
	stream.closeTag();

	// begin
	stream.openTag( "background" );
		// color
		Write_Property( stream, "color_red", static_cast<int>(m_background_color.red) );
		Write_Property( stream, "color_green", static_cast<int>(m_background_color.green) );
		Write_Property( stream, "color_blue", static_cast<int>(m_background_color.blue) );
	// end background
	stream.closeTag();

	// begin
	stream.openTag( "player" );
		// start waypoint
		Write_Property( stream, "waypoint", m_player_start_waypoint );
		// moving state
		Write_Property( stream, "moving_state", static_cast<int>(m_player_moving_state) );
	// end player
	stream.closeTag();

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
		obj->Save_To_XML( stream );
	}

	// end overworld
	stream.closeTag();

	file.close();

	// save layer
	m_layer->Save( save_dir + "/layer.xml" );
	// save description
	m_description->Save();

	// show info
	pHud_Debug->Set_Text( _("World ") + m_description->m_name + _(" saved") );
}

void cOverworld :: Enter( const GameMode old_mode /* = MODE_NOTHING */ )
{
	if( game_exit )
	{
		return;
	}

	// if not loaded
	if( !Is_Loaded() )
	{
		return;
	}

	// set active camera
	pActive_Camera = pOverworld_Manager->m_camera;
	// set active player
	pActive_Player = pOverworld_Player;
	// set animation manager
	pActive_Animation_Manager = m_animation_manager;

	pHud_Manager->Set_Sprite_Manager( m_sprite_manager );
	pMouseCursor->Set_Sprite_Manager( m_sprite_manager );

	// if player start waypoint not set
	if( pOverworld_Player->m_current_waypoint < 0 )
	{
		pOverworld_Player->Reset();
		pOverworld_Player->Set_Waypoint( m_player_start_waypoint, 1 );
	}

	// if goto next level
	if( m_next_level )
	{
		Goto_Next_Level();
	}

	// if on start waypoint
	if( pOverworld_Player->m_current_waypoint == static_cast<int>(m_player_start_waypoint) )
	{
		// if player state is walk
		if( m_player_moving_state == STA_WALK )
		{
			// walk to the next Waypoint
			pOverworld_Player->Start_Walk( m_waypoints[pOverworld_Player->m_current_waypoint]->m_direction_forward );
		}
	}

	if( old_mode == MODE_NOTHING || old_mode == MODE_OVERWORLD )
	{
		// set default camera limit
		pOverworld_Manager->m_camera->Set_Limit_Y( 0 );
	}

	Update_Camera();

	// play music
	pAudio->Play_Music( m_musicfile, -1, 0, 3000 );

	// reset custom level mode type
	if( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM )
	{
		Game_Mode_Type = MODE_TYPE_DEFAULT;
	}

	// disable level editor
	pLevel_Editor->Disable();

	// set editor enabled state
	editor_enabled = pWorld_Editor->m_enabled;

	if( pWorld_Editor->m_enabled )
	{
		if( !pWorld_Editor->m_editor_window->isVisible() )
		{
			pWorld_Editor->m_editor_window->show();
			pMouseCursor->Set_Active( 1 );
		}
	}

	// Update Hud Text and position
	pHud_Manager->Update_Text();

	// reset speedfactor
	pFramerate->Reset();
}

void cOverworld :: Leave( const GameMode next_mode /* = MODE_NOTHING */ )
{
	// if not in world mode
	if( Game_Mode != MODE_OVERWORLD )
	{
		return;
	}

	// reset mouse
	pMouseCursor->Reset( 0 );

	// world to world
	if( next_mode == MODE_OVERWORLD )
	{
		// fade out music
		pAudio->Fadeout_Music( 500 );
		return;
	}

	if( next_mode == MODE_LEVEL )
	{
		m_animation_manager->Delete_All();
	}

	// hide editor window if visible
	if( pWorld_Editor->m_enabled )
	{
		if( pWorld_Editor->m_editor_window->isVisible() )
		{
			pWorld_Editor->m_editor_window->hide();
		}
	}

	// if new mode is not menu
	if( next_mode != MODE_MENU )
	{
		// fade out music
		pAudio->Fadeout_Music( 1000 );

		// clear input
		Clear_Input_Events();
	}
}

void cOverworld :: Draw( void )
{
	// Background
	pVideo->Clear_Screen();
	Draw_Layer_1();

	// Player
	pOverworld_Player->Draw();
	// Hud
	Draw_HUD();

	// Editor
	pWorld_Editor->Draw();

	// update performance timer
	pFramerate->m_perf_timer[PERF_DRAW_OVERWORLD]->Update();
}

void cOverworld :: Draw_Layer_1( void )
{
	pVideo->Draw_Rect( NULL, 0.0001f, &m_background_color );

	// sprites
	m_sprite_manager->Draw_Items();
	// animations
	m_animation_manager->Draw();
}

void cOverworld :: Draw_HUD( void )
{
	// if not editor mode
	if( !editor_world_enabled )
	{
		// Background
		Color color = Color( static_cast<Uint8>(230), 170, 0, 128 );
		pVideo->Draw_Rect( 0, 0, static_cast<float>(game_res_w), 30, 0.12f, &color );
		// Line
		color = Color( static_cast<Uint8>(200), 150, 0, 128 );
		pVideo->Draw_Rect( 0, 30, static_cast<float>(game_res_w), 5, 0.121f, &color );

		// Overworld name and level
		m_hud_world_name->Draw();
		m_hud_level_name->Draw();
	}

	// hud
	pHud_Manager->Draw();
}

void cOverworld :: Update( void )
{
	// editor
	pWorld_Editor->Process_Input();

	if( !editor_world_enabled )
	{
		// Camera
		Update_Camera();
		// Map
		m_sprite_manager->Update_Items();
		// Player
		pOverworld_Player->Update();
		// Animations
		m_animation_manager->Update();
	}
	// if world-editor is enabled
	else
	{
		// only update particle emitters
		for( cSprite_List::iterator itr = m_sprite_manager->objects.begin(); itr != m_sprite_manager->objects.end(); ++itr )
		{
			cSprite *obj = (*itr);

			if( obj->m_type == TYPE_PARTICLE_EMITTER )
			{
				obj->Update();
			}
		}
	}

	// hud
	pHud_Manager->Update();
	// Editor
	pWorld_Editor->Update();

	// update performance timer
	pFramerate->m_perf_timer[PERF_UPDATE_OVERWORLD]->Update();
}

void cOverworld :: Update_Camera( void )
{
	if( editor_world_enabled )
	{
		return;
	}

	// todo : move to a Process_Input function
	if( pOverworld_Manager->m_camera_mode )
	{
		if( pKeyboard->m_keys[pPreferences->m_key_right] || ( pJoystick->m_right && pPreferences->m_joy_enabled ) )
		{
			pOverworld_Manager->m_camera->Move( pFramerate->m_speed_factor * 15, 0 );
		}
		else if( pKeyboard->m_keys[pPreferences->m_key_left] || ( pJoystick->m_left && pPreferences->m_joy_enabled ) )
		{
			pOverworld_Manager->m_camera->Move( pFramerate->m_speed_factor * -15, 0 );
		}
		if( pKeyboard->m_keys[pPreferences->m_key_up] || ( pJoystick->m_up && pPreferences->m_joy_enabled ) )
		{
			pOverworld_Manager->m_camera->Move( 0, pFramerate->m_speed_factor * -15 );
		}
		else if( pKeyboard->m_keys[pPreferences->m_key_down] || ( pJoystick->m_down && pPreferences->m_joy_enabled ) )
		{
			pOverworld_Manager->m_camera->Move( 0, pFramerate->m_speed_factor * 15 );
		}
	}
	// default player camera
	else
	{
		pOverworld_Manager->m_camera->Update();
	}
}

bool cOverworld :: Key_Down( SDLKey key )
{
	if( key == SDLK_LEFT )
	{
		if( !pOverworld_Manager->m_camera_mode && !editor_world_enabled )
		{
			pOverworld_Player->Action_Interact( INP_LEFT );
		}
		return 0;
	}
	else if( key == SDLK_RIGHT )
	{
		if( !pOverworld_Manager->m_camera_mode && !editor_world_enabled )
		{
			pOverworld_Player->Action_Interact( INP_RIGHT );
		}
		return 0;
	}
	else if( key == SDLK_UP )
	{
		if( !pOverworld_Manager->m_camera_mode && !editor_world_enabled )
		{
			pOverworld_Player->Action_Interact( INP_UP );
		}
		return 0;
	}
	else if( key == SDLK_DOWN )
	{
		if( !pOverworld_Manager->m_camera_mode && !editor_world_enabled )
		{
			pOverworld_Player->Action_Interact( INP_DOWN );
		}
		return 0;
	}
	else if( key == SDLK_c && !editor_world_enabled )
	{
		pOverworld_Manager->m_camera_mode = !pOverworld_Manager->m_camera_mode;
	}
	else if( key == SDLK_F8 )
	{
		pWorld_Editor->Toggle();
	}
	else if( key == SDLK_d && pKeyboard->Is_Ctrl_Down() )
	{
		pOverworld_Manager->m_debug_mode = !pOverworld_Manager->m_debug_mode;
		game_debug = pOverworld_Manager->m_debug_mode;
	}
	else if( key == SDLK_l && pOverworld_Manager->m_debug_mode )
	{
		// toggle layer drawing
		pOverworld_Manager->m_draw_layer = !pOverworld_Manager->m_draw_layer;
	}
	else if( pKeyboard->m_keys[SDLK_g] && pKeyboard->m_keys[SDLK_o] && pKeyboard->m_keys[SDLK_d] )
	{
		// all waypoint access
		Set_Progress( m_waypoints.size(), 1 );
	}
	else if( key == SDLK_F3 && pOverworld_Manager->m_debug_mode )
	{
		Goto_Next_Level();
	}
	// Exit
	else if( key == SDLK_ESCAPE || key == SDLK_BACKSPACE )
	{
		pOverworld_Player->Action_Interact( INP_EXIT );
	}
	// Action
	else if( key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE )
	{
		pOverworld_Player->Action_Interact( INP_ACTION );
	}
	// ## editor
	else if( pWorld_Editor->Key_Down( key ) )
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

bool cOverworld :: Key_Up( SDLKey key )
{
	// nothing yet
	if( 0 )
	{
		//
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cOverworld :: Mouse_Down( Uint8 button )
{
	// ## editor
	if( pWorld_Editor->Mouse_Down( button ) )
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

bool cOverworld :: Mouse_Up( Uint8 button )
{
	// ## editor
	if( pWorld_Editor->Mouse_Up( button ) )
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

bool cOverworld :: Joy_Button_Down( Uint8 button )
{
	// Exit
	if( button == pPreferences->m_joy_button_exit )
	{
		pOverworld_Player->Action_Interact( INP_EXIT );
	}
	// Action
	else if( button == pPreferences->m_joy_button_action )
	{
		pOverworld_Player->Action_Interact( INP_ACTION );
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cOverworld :: Joy_Button_Up( Uint8 button )
{
	// nothing yet
	if( 0 )
	{
		//
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

void cOverworld :: Set_Progress( unsigned int normal_level, bool force /* = 1 */ )
{
	unsigned int level_num = 0;

	for( WaypointList::iterator itr = m_waypoints.begin(); itr != m_waypoints.end(); ++itr )
	{
		cWaypoint *obj = (*itr);

		// accessible
		if( normal_level >= level_num )
		{
			obj->Set_Access( 1 );
		}
		// force unset
		else if( force )
		{
			obj->Set_Access( 0 );
		}

		level_num++;
	}
}

cWaypoint *cOverworld :: Get_Waypoint( const std::string &name )
{
	for( WaypointList::iterator itr = m_waypoints.begin(); itr != m_waypoints.end(); ++itr )
	{
		cWaypoint *obj = (*itr);

		// accessible
		if( obj->m_destination.compare( name ) == 0 )
		{
			return obj;
		}
	}

	return NULL;
}

cWaypoint *cOverworld :: Get_Waypoint( unsigned int num )
{
	if( num >= m_waypoints.size() )
	{
		// out of bounds
		return NULL;
	}

	// available
	return m_waypoints[num];
}

int cOverworld :: Get_Level_Waypoint_Num( std::string level_name )
{
	// erase file type if set
	if( level_name.rfind( ".txt" ) != std::string::npos || level_name.rfind( ".smclvl" ) != std::string::npos )
	{
		level_name.erase( level_name.rfind( "." ) );
	}

	return Get_Waypoint_Num( level_name );
}

int cOverworld :: Get_Waypoint_Num( const std::string &name )
{
	int count = 0;

	// search waypoint
	for( WaypointList::iterator itr = m_waypoints.begin(); itr != m_waypoints.end(); ++itr )
	{
		cWaypoint *obj = (*itr);

		if( obj->m_destination.compare( name ) == 0 )
		{
			// found
			return count;
		}

		count++;
	}

	// not found
	return -1;
}

int cOverworld :: Get_Waypoint_Collision( const GL_rect &rect_2 )
{
	int count = 0;

	for( WaypointList::iterator itr = m_waypoints.begin(); itr != m_waypoints.end(); ++itr )
	{
		cWaypoint *obj = (*itr);

		if( rect_2.Intersects( obj->m_rect ) )
		{
			return count;
		}

		count++;
	}

	return -1;
}

int cOverworld :: Get_Last_Valid_Waypoint( void )
{
	// no waypoints
	if( m_waypoints.empty() )
	{
		return -1;
	}

	for( int i = m_waypoints.size() - 1; i > 0; i-- )
	{
		if( m_waypoints[i]->m_access )
		{
			return i;
		}
	}

	return -1;
}

void cOverworld :: Update_Waypoint_text( void )
{
	// get waypoint
	cWaypoint *waypoint = m_waypoints[pOverworld_Player->m_current_waypoint];

	// set color
	Color color = static_cast<Uint8>(0);

	if( waypoint->m_waypoint_type == WAYPOINT_NORMAL )
	{
		color = lightblue;
	}
	else if( waypoint->m_waypoint_type == WAYPOINT_WORLD_LINK )
	{
		color = green;
	}
	
	m_hud_level_name->Set_Image( pFont->Render_Text( pFont->m_font_normal, waypoint->Get_Destination(), color ), 1, 1 );
}

bool cOverworld :: Goto_Next_Level( void )
{
	// if not in overworld only go to the next level on overworld enter
	if( Game_Mode != MODE_OVERWORLD )
	{
		m_next_level = 1;
		return 0;
	}

	m_next_level = 0;

	cWaypoint *current_waypoint = pOverworld_Player->Get_Waypoint();

	// no Waypoint
	if( !current_waypoint )
	{
		return 0;
	}

	// Waypoint forward direction is invalid/unset
	if( current_waypoint->m_direction_forward == DIR_UNDEFINED )
	{
		return 0;
	}

	// Get Layer Line in front
	cLayer_Line_Point_Start *front_line = pOverworld_Player->Get_Front_Line( current_waypoint->m_direction_forward );

	if( !front_line )
	{
		return 0;
	}

	// Get forward Waypoint
	cWaypoint *next_waypoint = front_line->Get_End_Waypoint();

	// if no next waypoint available
	if( !next_waypoint )
	{
		return 0;
	}

	// if next waypoint is new
	if( !next_waypoint->m_access )
	{
		next_waypoint->Set_Access( 1 );

		// animation
		cParticle_Emitter *anim = new cParticle_Emitter( m_sprite_manager );
		anim->Set_Emitter_Rect( next_waypoint->m_rect.m_x + ( next_waypoint->m_rect.m_w * 0.5f ), next_waypoint->m_rect.m_y + ( next_waypoint->m_rect.m_h * 0.5f ), 1.0f, 1.0f );
		anim->Set_Emitter_Time_to_Live( 1.5f );
		anim->Set_Emitter_Iteration_Interval( 0.05f );
		anim->Set_Quota( 1 );
		anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
		anim->Set_Pos_Z( 0.081f );
		anim->Set_Time_to_Live( 1.3f );
		anim->Set_Speed( 1.0f, 0.5f );
		anim->Set_Scale( 0.5f, 0.2f );
		anim->Set_Const_Rotation_Z( -6, 12 );

		// World Waypoint
		if( next_waypoint->m_waypoint_type == WAYPOINT_WORLD_LINK )
		{
			anim->Set_Color( whitealpha128, Color( static_cast<Uint8>(0), 0, 0, 128 ) );
		}
		else
		{
			anim->Set_Color( orange, Color( static_cast<Uint8>(6), 60, 20, 0 ) );
		}

		// add animation
		m_animation_manager->Add( anim );
	}

	pOverworld_Player->Start_Walk( current_waypoint->m_direction_forward );

	return  1;
}

void cOverworld :: Reset_Waypoints( void )
{
	for( WaypointList::iterator itr = m_waypoints.begin(); itr != m_waypoints.end(); ++itr )
	{
		cWaypoint *obj = (*itr);

		obj->Set_Access( obj->m_access_default );
	}
}

bool cOverworld :: Is_Loaded( void ) const
{
	// if not loaded version is -1
	if( m_engine_version >= 0 )
	{
		return 1;
	}

	return 0;
}

void cOverworld :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
{
	if( element == "property" || element == "Property" )
	{
		m_xml_attributes.add( attributes.getValueAsString( "name" ), attributes.getValueAsString( "value" ) );
	}
}

void cOverworld :: elementEnd( const CEGUI::String &element )
{
	if( element == "property" || element == "Property" )
	{
		return;
	}

	if( element == "information" )
	{
		m_engine_version = m_xml_attributes.getValueAsInteger( "engine_version" );
		m_last_saved = string_to_int64( m_xml_attributes.getValueAsString( "save_time" ).c_str() );
	}
	else if( element == "settings" )
	{
		// Author
		//author = m_xml_attributes.getValueAsString( "author" ).c_str();
		// Version
		//version = m_xml_attributes.getValueAsString( "version" ).c_str();
		// Music
		m_musicfile = xml_string_to_string( m_xml_attributes.getValueAsString( "music" ).c_str() );
		// Camera Limits
		//pOverworld_Manager->camera->Set_Limits( GL_rect( static_cast<float>(m_xml_attributes.getValueAsInteger( "cam_limit_x" )), static_cast<float>(m_xml_attributes.getValueAsInteger( "cam_limit_y" )), static_cast<float>(m_xml_attributes.getValueAsInteger( "cam_limit_w" )), static_cast<float>(m_xml_attributes.getValueAsInteger( "cam_limit_h" )) ) );
	}
	else if( element == "player" )
	{
		// Start Waypoint
		m_player_start_waypoint = m_xml_attributes.getValueAsInteger( "waypoint" );
		// Moving State
		m_player_moving_state = static_cast<Moving_state>(m_xml_attributes.getValueAsInteger( "moving_state" ));
	}
	else if( element == "background" )
	{
		m_background_color = Color( static_cast<Uint8>(m_xml_attributes.getValueAsInteger( "color_red" )), m_xml_attributes.getValueAsInteger( "color_green" ), m_xml_attributes.getValueAsInteger( "color_blue" ) );
	}
	else
	{
		// get World object
		cSprite *object = Create_World_Object_From_XML( element, m_xml_attributes, m_engine_version, m_sprite_manager, this );
		
		// valid
		if( object )
		{
			m_sprite_manager->Add( object );
		}
		else if( element == "overworld" )
		{
			// ignore
		}
		else if( element.length() )
		{
			printf( "Warning : Overworld Unknown element : %s\n", element.c_str() );
		}
	}

	// clear
	m_xml_attributes = CEGUI::XMLAttributes();
}

cSprite *Create_World_Object_From_XML( const CEGUI::String &element, CEGUI::XMLAttributes &attributes, int engine_version, cSprite_Manager *sprite_manager, cOverworld *overworld )
{
	if( element == "sprite" )
	{
		// old version : change file and position name
		if( engine_version < 2 )
		{
			if( attributes.exists( "filename" ) )
			{
				attributes.add( "image", attributes.getValueAsString( "filename" ) );
				attributes.add( "posx", attributes.getValueAsString( "pos_x" ) );
				attributes.add( "posy", attributes.getValueAsString( "pos_y" ) );
			}
		}
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( engine_version < 2 )
		{
			if( attributes.exists( "posy" ) )
			{
				attributes.add( "posy", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "posy" ) - 600.0f ) );
			}
		}
		// if V.1.9 and lower : change old bridge to bridge 1 vertical
		if( engine_version < 3 )
		{
			Relocate_Image( attributes, "world/objects/bridge/bridge_1.png", "world/objects/bridge/bridge_1_ver_start.png" );
		}

		// create sprite
		cSprite *sprite = new cSprite( attributes, sprite_manager );
		// set sprite type
		sprite->Set_Sprite_Type( TYPE_PASSIVE );

		// needs image
		if( sprite->m_image )
		{
			// if V.1.9 and lower : change old bridge to bridge 1 vertical
			if( engine_version < 3 )
			{
				if( sprite->m_image->m_filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "world/objects/bridge/bridge_1_ver_start.png" ) == 0 )
				{
					// move a bit to the left
					sprite->Move( -7.0f, 0.0f, 1 );
					sprite->m_start_pos_x = sprite->m_pos_x;

					// create other tiles now
					cSprite *copy = sprite->Copy();

					// middle
					copy->Set_Image( pVideo->Get_Surface( "world/objects/bridge/bridge_1_ver_middle.png" ), 1 );
					copy->Set_Pos_Y( copy->m_start_pos_y + 32, 1 );
					sprite_manager->Add( copy );
					// end
					copy = copy->Copy();
					copy->Set_Image( pVideo->Get_Surface( "world/objects/bridge/bridge_1_ver_end.png" ), 1 );
					copy->Set_Pos_Y( copy->m_start_pos_y + 32, 1 );
					sprite_manager->Add( copy );
				}
			}
		}

		return sprite;
	}
	else if( element == "waypoint" )
	{
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( engine_version < 2 )
		{
			if( attributes.exists( "y" ) )
			{
				attributes.add( "y", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "y" ) - 600.0f ) );
			}
		}

		return new cWaypoint( attributes, sprite_manager );
	}
	else if( element == "sound" )
	{
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( engine_version < 2 )
		{
			if( attributes.exists( "pos_y" ) )
			{
				attributes.add( "pos_y", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "pos_y" ) - 600.0f ) );
			}
		}

		return new cRandom_Sound( attributes, sprite_manager );
	}
	else if( element == "line" )
	{
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( engine_version < 2 )
		{
			if( attributes.exists( "Y1" ) )
			{
				attributes.add( "Y1", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "Y1" ) - 600.0f ) );
			}
			if( attributes.exists( "Y2" ) )
			{
				attributes.add( "Y2", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "Y2" ) - 600.0f ) );
			}
		}

		return new cLayer_Line_Point_Start( attributes, sprite_manager, overworld );
	}

	return NULL;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cOverworld *pActive_Overworld = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
