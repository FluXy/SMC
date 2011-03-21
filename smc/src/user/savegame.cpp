/***************************************************************************
 * savegame.cpp  -  Savegame handler
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

#include "../user/savegame.h"
#include "../user/preferences.h"
#include "../core/game_core.h"
#include "../core/obj_manager.h"
#include "../level/level.h"
#include "../overworld/world_manager.h"
#include "../level/level_player.h"
#include "../overworld/overworld.h"
#include "../core/i18n.h"
#include "../core/filesystem/filesystem.h"
#include "../core/filesystem/resource_manager.h"
// CEGUI
#include "CEGUIXMLParser.h"
#include "CEGUIExceptions.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cSave_Overworld_Waypoint *** *** *** *** *** *** *** *** *** *** */

cSave_Overworld_Waypoint :: cSave_Overworld_Waypoint( void )
{
	m_access = 0;
}

cSave_Overworld_Waypoint :: ~cSave_Overworld_Waypoint( void )
{
	
}

/* *** *** *** *** *** *** *** cSave_Overworld *** *** *** *** *** *** *** *** *** *** */

cSave_Overworld :: cSave_Overworld( void )
{
	
}

cSave_Overworld :: ~cSave_Overworld( void )
{
	for( unsigned int i = 0; i < m_waypoints.size(); i++ )
	{
		delete m_waypoints[i];
	}

	m_waypoints.clear();
}

/* *** *** *** *** *** *** *** cSave_Level_Object *** *** *** *** *** *** *** *** *** *** */

cSave_Level_Object_Property :: cSave_Level_Object_Property( const std::string &new_name /* = "" */, const std::string &new_value /* = "" */ )
{
	m_name = new_name;
	m_value = new_value;
}

/* *** *** *** *** *** *** *** cSave_Level_Object *** *** *** *** *** *** *** *** *** *** */

cSave_Level_Object :: cSave_Level_Object( void )
{
	m_type = TYPE_UNDEFINED;
}

cSave_Level_Object :: ~cSave_Level_Object( void )
{
	m_properties.clear();
}

bool cSave_Level_Object :: exists( const std::string &val_name )
{
	for( Save_Level_Object_ProprtyList::iterator itr = m_properties.begin(); itr != m_properties.end(); ++itr )
	{
		cSave_Level_Object_Property obj = (*itr);

		if( obj.m_name.compare( val_name ) == 0 )
		{
			// found
			return 1;
		}
	}

	// not found
	return 0;
}

std::string cSave_Level_Object :: Get_Value( const std::string &val_name )
{
	for( Save_Level_Object_ProprtyList::iterator itr = m_properties.begin(); itr != m_properties.end(); ++itr )
	{
		cSave_Level_Object_Property obj = (*itr);

		if( obj.m_name.compare( val_name ) == 0 )
		{
			// found
			return obj.m_value;
		}
	}

	// not found
	return "";
}

/* *** *** *** *** *** *** *** cSave_Level *** *** *** *** *** *** *** *** *** *** */

cSave_Level :: cSave_Level( void )
{
	// level
	m_level_pos_x = 0.0f;
	m_level_pos_y = 0.0f;
}

cSave_Level :: ~cSave_Level( void )
{
	for( Save_Level_ObjectList::iterator itr = m_level_objects.begin(); itr != m_level_objects.end(); ++itr )
	{
		delete *itr;
	}

	m_level_objects.clear();

	for( cSprite_List::iterator itr = m_spawned_objects.begin(); itr != m_spawned_objects.end(); ++itr )
	{
		delete *itr;
	}

	m_spawned_objects.clear();
}

/* *** *** *** *** *** *** *** cSave *** *** *** *** *** *** *** *** *** *** */

cSave :: cSave( void )
{
	Init();
}

cSave :: ~cSave( void )
{
	for( Save_LevelList::iterator itr = m_levels.begin(); itr != m_levels.end(); ++itr )
	{
		delete *itr;
	}

	m_levels.clear();

	for( Save_OverworldList::iterator itr = m_overworlds.begin(); itr != m_overworlds.end(); ++itr )
	{
		delete *itr;
	}

	m_overworlds.clear();
}

void cSave :: Init( void )
{
	// save
	m_save_time = 0;
	m_version = 0;

	// player
	m_lives = 0;
	m_points = 0;
	m_goldpieces = 0;
	m_player_type = 0;
	m_player_state = 0;
	m_itembox_item = 0;

	// level
	m_level_time = 0;
	// set earliest available version
	m_level_engine_version = 39;

	// overworld
	m_overworld_current_waypoint = 0;
}

std::string cSave :: Get_Active_Level( void )
{
	if( m_levels.empty() )
	{
		return "";
	}

	for( Save_LevelList::iterator itr = m_levels.begin(); itr != m_levels.end(); ++itr )
	{
		cSave_Level *save_level = (*itr);

		if( !pLevel_Manager->Get_Path( save_level->m_name ) )
		{
			continue;
		}

		// if active level
		if( !Is_Float_Equal( save_level->m_level_pos_x, 0.0f ) && !Is_Float_Equal( save_level->m_level_pos_y, 0.0f ) )
		{
			return save_level->m_name;
		}
	}

	return "";
}

/* *** *** *** *** *** *** *** cSavegame *** *** *** *** *** *** *** *** *** *** */

cSavegame :: cSavegame( void )
{
	m_savegame_dir = pResource_Manager->user_data_dir + USER_SAVEGAME_DIR;
}

cSavegame :: ~cSavegame( void )
{
	//
}

int cSavegame :: Load_Game( unsigned int save_slot )
{
	cSave *savegame = Load( save_slot );

	if( !savegame )
	{
		return 0;
	}
	
	// check if unsupported save
	if( savegame->m_version <= SAVEGAME_VERSION_UNSUPPORTED )
	{
		printf( "Warning : Savegame %d : Versions %d and below are unsupported\n", save_slot, SAVEGAME_VERSION_UNSUPPORTED );
	}

	// reset custom level mode type
	if( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM )
	{
		Game_Mode_Type = MODE_TYPE_DEFAULT;
	}

	// #### Overworld ####

	// set overworld progress
	if( !savegame->m_overworlds.empty() )
	{
		for( Save_OverworldList::iterator itr = savegame->m_overworlds.begin(); itr != savegame->m_overworlds.end(); ++itr )
		{
			// get savegame overworld pointer
			cSave_Overworld *save_overworld = (*itr);

			// get overworld
			cOverworld *overworld = pOverworld_Manager->Get_from_Name( save_overworld->m_name );

			if( !overworld )
			{
				printf( "Warning : Savegame %d : Overworld %s not found\n", save_slot, save_overworld->m_name.c_str() );
				continue;
			}

			for( Save_Overworld_WaypointList::iterator wp_itr = save_overworld->m_waypoints.begin(); wp_itr != save_overworld->m_waypoints.end(); ++wp_itr )
			{
				// get savegame waypoint pointer
				cSave_Overworld_Waypoint *save_waypoint = (*wp_itr);

				// get overworld waypoint
				cWaypoint *waypoint = overworld->Get_Waypoint( overworld->Get_Waypoint_Num( save_waypoint->m_destination ) );

				// not found
				if( !waypoint )
				{
					printf( "Warning : Savegame %d : Overworld %s Waypoint %s not found\n", save_slot, save_overworld->m_name.c_str(), save_waypoint->m_destination.c_str() );
					continue;
				}

				// set access
				waypoint->Set_Access( save_waypoint->m_access );
			}
		}
	}

	// if an overworld is active
	if( !savegame->m_overworld_active.empty() )
	{
		// Set active overworld
		if( !pOverworld_Manager->Set_Active( savegame->m_overworld_active ) )
		{
			printf( "Warning : Savegame %d : Couldn't set Overworld active %s\n", save_slot, savegame->m_overworld_active.c_str() );
		}

		// Current waypoint
		if( !pOverworld_Player->Set_Waypoint( savegame->m_overworld_current_waypoint ) )
		{
			printf( "Warning : Savegame %d : Overworld Current Waypoint %d is invalid\n", save_slot, savegame->m_overworld_current_waypoint );
		}
	}
	// overworld is not active
	else
	{
		// Set custom level mode
		Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM;
	}

	// #### Level ####

	// below version 8 the state was the type
	if( savegame->m_version < 8 )
	{
		pLevel_Player->Set_Type( static_cast<Maryo_type>(savegame->m_player_state), 0, 0 );
	}
	else
	{
		pLevel_Player->Set_Type( static_cast<Maryo_type>(savegame->m_player_type), 0, 0 );
		pLevel_Player->m_state = static_cast<Moving_state>(savegame->m_player_state);
	}

	// default is world savegame
	unsigned int save_type = 2;

	// load levels
	if( !savegame->m_levels.empty() )
	{
		for( Save_LevelList::iterator itr = savegame->m_levels.begin(); itr != savegame->m_levels.end(); ++itr )
		{
			cSave_Level *save_level = (*itr);
			cLevel *level = pLevel_Manager->Load( save_level->m_name );

			if( !level )
			{
				printf( "Warning : Couldn't load Savegame Level %s\n", save_level->m_name.c_str() );
				continue;
			}

			// active level
			if( !Is_Float_Equal( save_level->m_level_pos_x, 0.0f ) && !Is_Float_Equal( save_level->m_level_pos_y, 0.0f ) )
			{
				pLevel_Manager->Set_Active( level );
				level->Init();

				// if below version 9 : move y coordinate bottom to 0 and remove screen height adjustment
				if( savegame->m_version < 9 )
				{
					save_level->m_level_pos_y -= 1200.0f;
				}

				// time
				pHud_Time->Set_Time( savegame->m_level_time );
				// position
				pLevel_Player->Set_Pos( save_level->m_level_pos_x, save_level->m_level_pos_y );

				// invincible for a second
				pLevel_Player->m_invincible = speedfactor_fps;
				// level savegame
				save_type = 1;
			}

			// spawned objects
			for( cSprite_List::iterator itr = save_level->m_spawned_objects.begin(); itr != save_level->m_spawned_objects.end(); ++itr )
			{
				cSprite *sprite = (*itr);

				sprite->Set_Sprite_Manager( level->m_sprite_manager );
				sprite->Set_Spawned( 1 );
				level->m_sprite_manager->Add( sprite );
			}

			save_level->m_spawned_objects.clear();

			// objects data
			for( Save_Level_ObjectList::iterator itr = save_level->m_level_objects.begin(); itr != save_level->m_level_objects.end(); ++itr )
			{
				cSave_Level_Object *save_object = (*itr);

				// get position
				int posx = string_to_int( save_object->Get_Value( "posx" ) );
				int posy = string_to_int( save_object->Get_Value( "posy" ) );

				// get level object
				cSprite *level_object = level->m_sprite_manager->Get_from_Position( posx, posy, save_object->m_type, 1 );

				// if not anymore available
				if( !level_object )
				{
					printf( "Warning : Savegame object type %d on x %d, y %d not available\n", save_object->m_type, posx, posy );
					continue;
				}

				level_object->Load_From_Savegame( save_object );
			}
		}
	}

	// #### Player ####

	pHud_Points->Set_Points( savegame->m_points );
	pHud_Goldpieces->Set_Gold( savegame->m_goldpieces );
	pHud_Lives->Set_Lives( savegame->m_lives );
	pHud_Itembox->Set_Item( static_cast<SpriteType>(savegame->m_itembox_item), 0 );
	pHud_Debug->Set_Text( _("Savegame ") + int_to_string( save_slot ) + _(" loaded") );
	pHud_Manager->Update();

	delete savegame;
	return save_type;
}

bool cSavegame :: Save_Game( unsigned int save_slot, std::string description )
{
	if( pLevel_Player->m_maryo_type == MARYO_DEAD || pLevel_Player->m_lives < 0 )
	{
		printf( "Error : Couldn't save savegame %s because of invalid game state\n", description.c_str() );
		return 0;
	}

	cSave *savegame = new cSave();

	savegame->m_version = SAVEGAME_VERSION;
	savegame->m_level_engine_version = level_engine_version;
	savegame->m_save_time = time( NULL );

	savegame->m_description = description;
	savegame->m_goldpieces = pLevel_Player->m_goldpieces;

	// if in a level
	if( pActive_Level->Is_Loaded() )
	{
		savegame->m_level_time = pHud_Time->m_milliseconds;

		for( vector<cLevel *>::iterator itr = pLevel_Manager->objects.begin(); itr != pLevel_Manager->objects.end(); ++itr )
		{
			cLevel *level = (*itr);

			if( !level->Is_Loaded() )
			{
				continue;
			}
			
			// create level data
			cSave_Level *save_level = new cSave_Level();

			save_level->m_name = Trim_Filename( level->m_level_filename, 0, 0 );
			
			// save position if active level
			if( pActive_Level == level )
			{
				save_level->m_level_pos_x = pLevel_Player->m_pos_x;
				save_level->m_level_pos_y = pLevel_Player->m_pos_y - 5.0f;
			}

			// spawned objects
			for( cSprite_List::iterator itr = level->m_sprite_manager->objects.begin(); itr != level->m_sprite_manager->objects.end(); ++itr )
			{
				cSprite *obj = (*itr);

				// if spawned and active
				if( !obj->m_spawned || obj->m_auto_destroy )
				{
					continue;
				}

				// add
				save_level->m_spawned_objects.push_back( obj->Copy() );
			}

			// save object status
			for( cSprite_List::iterator itr = level->m_sprite_manager->objects.begin(); itr != level->m_sprite_manager->objects.end(); ++itr )
			{
				cSprite *obj = (*itr);

				// get save data
				cSave_Level_Object *save_obj = obj->Save_To_Savegame();

				// nothing to save
				if( !save_obj )
				{
					continue;
				}

				// add
				save_level->m_level_objects.push_back( save_obj );
			}

			savegame->m_levels.push_back( save_level );
		}
	}

	savegame->m_lives = pLevel_Player->m_lives;
	savegame->m_points = pLevel_Player->m_points;
	savegame->m_player_type = pLevel_Player->m_maryo_type;
	savegame->m_player_state = pLevel_Player->m_state;
	savegame->m_itembox_item = pHud_Itembox->m_item_id;

	// save overworld progress
	for( vector<cOverworld *>::iterator itr = pOverworld_Manager->objects.begin(); itr != pOverworld_Manager->objects.end(); ++itr )
	{
		// Get Overworld
		cOverworld *overworld = (*itr);

		// create Overworld
		cSave_Overworld *save_overworld = new cSave_Overworld();
		save_overworld->m_name = overworld->m_description->m_name;
		
		// Waypoints
		for( cSprite_List::iterator wp_itr = overworld->m_sprite_manager->objects.begin(); wp_itr != overworld->m_sprite_manager->objects.end(); ++wp_itr )
		{
			// get waypoint
			cSprite *obj = static_cast<cSprite *>(*wp_itr);

			if( obj->m_type != TYPE_OW_WAYPOINT )
			{
				continue;
			}

			// get waypoint
			cWaypoint *waypoint = static_cast<cWaypoint *>(obj);

			// create savegame waypoint
			cSave_Overworld_Waypoint *save_waypoint = new cSave_Overworld_Waypoint();
			
			// destination
			save_waypoint->m_destination = waypoint->Get_Destination();
			// set access
			save_waypoint->m_access = waypoint->m_access;
			// save
			save_overworld->m_waypoints.push_back( save_waypoint );
		}

		savegame->m_overworlds.push_back( save_overworld );
	}

	// if an overworld is active and not custom level mode
	if( pActive_Overworld && Game_Mode_Type != MODE_TYPE_LEVEL_CUSTOM )
	{
		// set overworld name
		savegame->m_overworld_active = pActive_Overworld->m_description->m_name;

		// if valid waypoint
		if( pOverworld_Player->m_current_waypoint >= 0 )
		{
			// set current waypoint
			savegame->m_overworld_current_waypoint = pOverworld_Player->m_current_waypoint;
		}
	}

	Save( save_slot, savegame );

	if( pHud_Debug )
	{
		pHud_Debug->Set_Text( _("Saved to Slot ") + int_to_string( save_slot ) );
	}

	delete savegame;

	return 1;
}

cSave *cSavegame :: Load( unsigned int save_slot )
{
	std::string filename = m_savegame_dir + "/" + int_to_string( save_slot ) + ".smcsav";

	// if not new format try the old
	if( !File_Exists( filename ) )
	{
		filename = m_savegame_dir + "/" + int_to_string( save_slot ) + ".save";
	}

	if( !File_Exists( filename ) )
	{
		printf( "Error : cSavegame::Load : No Savegame found at Slot : %s\n", filename.c_str() );
		return NULL;
	}

	cSavegame_XML_Handler *loader = new cSavegame_XML_Handler( filename );
	cSave *savegame = loader->Acquire_Savegame();
	delete loader;

	return savegame;
}

int cSavegame :: Save( unsigned int save_slot, cSave *savegame )
{
	const std::string filename = m_savegame_dir + "/" + int_to_string( save_slot ) + ".smcsav";
	// remove old format savegame
	Delete_File( m_savegame_dir + "/" + int_to_string( save_slot ) + ".save" );

// fixme : Check if there is a more portable way f.e. with imbue()
#ifdef _WIN32
	ofstream file( utf8_to_ucs2( filename ).c_str(), ios::out | ios::trunc );
#else
	ofstream file( filename.c_str(), ios::out | ios::trunc );
#endif

	
	if( !file.is_open() )
	{
		printf( "Error : Couldn't open savegame file for saving. Is the file read-only ?" );
		pHud_Debug->Set_Text( _("Couldn't save savegame ") + filename, speedfactor_fps * 5.0f );
		return 0;
	}

	CEGUI::XMLSerializer stream( file );

	// begin
	stream.openTag( "savegame" );

	// begin
	stream.openTag( "information" );
		Write_Property( stream, "version", savegame->m_version );
		Write_Property( stream, "level_engine_version", savegame->m_level_engine_version );
		Write_Property( stream, "save_time", static_cast<Uint64>(savegame->m_save_time) );
		Write_Property( stream, "description", savegame->m_description );
	// end information
	stream.closeTag();

	// begin
	stream.openTag( "player" );
		Write_Property( stream, "lives", savegame->m_lives );
		Write_Property( stream, "points", savegame->m_points );
		Write_Property( stream, "goldpieces", savegame->m_goldpieces );
		Write_Property( stream, "type", savegame->m_player_type );
		Write_Property( stream, "state", savegame->m_player_state );
		Write_Property( stream, "itembox_item", savegame->m_itembox_item );
		// if a level is available
		if( !savegame->m_levels.empty() )
		{
			Write_Property( stream, "level_time", savegame->m_level_time );
		}
		Write_Property( stream, "overworld_active", savegame->m_overworld_active );
		Write_Property( stream, "overworld_current_waypoint", savegame->m_overworld_current_waypoint );
	// end player
	stream.closeTag();

	// levels
	for( Save_LevelList::iterator itr = savegame->m_levels.begin(); itr != savegame->m_levels.end(); ++itr )
	{
		cSave_Level *level = (*itr);

		// begin
		stream.openTag( "level" );

		// name
		Write_Property( stream, "level_name", level->m_name );
		// save position if active level
		if( !Is_Float_Equal( level->m_level_pos_x, 0.0f ) && !Is_Float_Equal( level->m_level_pos_y, 0.0f ) )
		{
			Write_Property( stream, "player_posx", level->m_level_pos_x );
			Write_Property( stream, "player_posy", level->m_level_pos_y );
		}

		// begin
		stream.openTag( "spawned_objects" );

		for( cSprite_List::iterator itr = level->m_spawned_objects.begin(); itr != level->m_spawned_objects.end(); ++itr )
		{
			cSprite *obj = (*itr);
			obj->Save_To_XML( stream );
		}

		// end spawned_objects
		stream.closeTag();

		// begin
		stream.openTag( "objects_data" );

		for( Save_Level_ObjectList::iterator itr = level->m_level_objects.begin(); itr != level->m_level_objects.end(); ++itr )
		{
			cSave_Level_Object *obj = (*itr);

			// begin
			stream.openTag( "object" );

			// type
			Write_Property( stream, "type", obj->m_type );

			// Properties
			for( Save_Level_Object_ProprtyList::iterator prop_itr = obj->m_properties.begin(); prop_itr != obj->m_properties.end(); ++prop_itr )
			{
				cSave_Level_Object_Property Property = (*prop_itr);
				Write_Property( stream, Property.m_name, Property.m_value );
			}

			// end object
			stream.closeTag();
		}

		// end object_data
		stream.closeTag();

		// end level
		stream.closeTag();
	}

	// Overworlds
	for( Save_OverworldList::iterator itr = savegame->m_overworlds.begin(); itr != savegame->m_overworlds.end(); ++itr )
	{
		cSave_Overworld *overworld = (*itr);

		// begin
		stream.openTag( "overworld" );

		// name
		Write_Property( stream, "name", overworld->m_name );

		for( Save_Overworld_WaypointList::iterator wp_itr = overworld->m_waypoints.begin(); wp_itr != overworld->m_waypoints.end(); ++wp_itr )
		{
			cSave_Overworld_Waypoint *overworld_waypoint = (*wp_itr);

			// skip empty waypoints
			if( overworld_waypoint->m_destination.empty() )
			{
				continue;
			}

			// begin
			stream.openTag( "waypoint" );

			Write_Property( stream, "destination", overworld_waypoint->m_destination );
			Write_Property( stream, "access", overworld_waypoint->m_access );

			// end waypoint
			stream.closeTag();
		}

		// end overworld
		stream.closeTag();
	}

	// end savegame
	stream.closeTag();

	file.close();
	
	debug_print( "Saved savegame %s\n", filename.c_str() );
	
	return 1;
}

std::string cSavegame :: Get_Description( unsigned int save_slot, bool only_description /* = 0 */ )
{
	std::string str_description;

	if( !Is_Valid( save_slot ) )
	{
		str_description = int_to_string( save_slot ) + ". Free Save";
		return str_description;
	}
	
	cSave *savegame = Load( save_slot );

	if( !savegame )
	{
		return "Savegame loading failed";
	}

	// complete description
	if( !only_description )
	{
		str_description = int_to_string( save_slot ) + ". " + savegame->m_description;

		if( savegame->m_levels.empty() )
		{
			str_description += " - " + savegame->m_overworld_active;
		}
		else
		{
			bool found_active = 0;

			for( Save_LevelList::iterator itr = savegame->m_levels.begin(); itr != savegame->m_levels.end(); ++itr )
			{
				cSave_Level *level = (*itr);

				// if active level
				if( !Is_Float_Equal( level->m_level_pos_x, 0.0f ) && !Is_Float_Equal( level->m_level_pos_y, 0.0f ) )
				{
					str_description += _(" -  Level ") + level->m_name;
					found_active = 1;
					break;
				}
			}
			
			if( !found_active )
			{
				str_description += _(" -  Unknown");
			}
		}

		str_description += _(" - Date ") + Time_to_String( savegame->m_save_time, "%Y-%m-%d  %H:%M:%S" );
	}
	// only the user description
	else
	{
		str_description = savegame->m_description;
	}

	delete savegame;
	return str_description;
}

bool cSavegame :: Is_Valid( unsigned int save_slot ) const
{
	return ( File_Exists( m_savegame_dir + "/" + int_to_string( save_slot ) + ".smcsav" ) || File_Exists( m_savegame_dir + "/" + int_to_string( save_slot ) + ".save" ) );
}

/* *** *** *** *** *** *** *** cSavegame_XML_Handler *** *** *** *** *** *** *** *** *** *** */

cSavegame_XML_Handler :: cSavegame_XML_Handler( const std::string &filename )
{
	// new savegame format
	if( filename.rfind( ".smcsav" ) != std::string::npos )
	{
		m_old_format = 0;
	}
	else
	{
		m_old_format = 1;
	}

	m_savegame = new cSave();

	std::string xsd_name;
	if( m_old_format )
	{
		xsd_name = "Savegame_old.xsd";
	}
	else
	{
		xsd_name = "Savegame.xsd";
	}

	try
	{
	// fixme : Workaround for std::string to CEGUI::String utf8 conversion. Check again if CEGUI 0.8 works with std::string utf8
	#ifdef _WIN32
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, (const CEGUI::utf8*)filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/" + xsd_name, "" );
	#else
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/" + xsd_name, "" );
	#endif
	}
	// catch CEGUI Exceptions
	catch( CEGUI::Exception &ex )
	{
		printf( "Loading Savegame %s CEGUI Exception %s\n", filename.c_str(), ex.getMessage().c_str() );
		pHud_Debug->Set_Text( _("Savegame Loading failed : ") + (const std::string)ex.getMessage().c_str() );
	}

	// if no description is set
	if( m_savegame->m_description.empty() )
	{
		m_savegame->m_description = _("No Description");
	}
}

cSavegame_XML_Handler :: ~cSavegame_XML_Handler( void )
{
	if( m_savegame )
	{
		delete m_savegame;
	}
}

cSave *cSavegame_XML_Handler :: Acquire_Savegame( void )
{
	cSave *save = m_savegame;
	m_savegame = NULL;
	return save;
}

void cSavegame_XML_Handler :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
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

void cSavegame_XML_Handler :: elementEnd( const CEGUI::String &element )
{
	if( element == "property" || element == "Property" )
	{
		return;
	}

	if( element == "information" || element == "Information" )
	{
		m_savegame->m_version = m_xml_attributes.getValueAsInteger( "version" );
		m_savegame->m_level_engine_version = m_xml_attributes.getValueAsInteger( "level_engine_version", m_savegame->m_level_engine_version );
		m_savegame->m_save_time = string_to_int64( m_xml_attributes.getValueAsString( "save_time" ).c_str() );
		m_savegame->m_description = m_xml_attributes.getValueAsString( "description" ).c_str();
	}
	else if( element == "level" || element == "Level" )
	{
		Handle_Level( m_xml_attributes );
	}
	else if( element == "objects_data" )
	{
		// don't clear attributes
		return;
	}
	else if( element == "object" || element == "Level_Object" )
	{
		Handle_Level_Object( m_xml_attributes );
		// don't clear attributes
		return;
	}
	else if( element == "spawned_objects" )
	{
		// don't clear attributes
		return;
	}
	else if( element != "player" && cLevel::Is_Level_Object_Element( element ) )
	{
		Handle_Level_Spawned_Object( element, m_xml_attributes );
		// don't clear attributes
		return;
	}
	else if( element == "player" || element == "Player" )
	{
		Handle_Player( m_xml_attributes );
	}
	else if( m_old_format && element == "Overworld_Data" )
	{
		Handle_Overworld_Data( m_xml_attributes );
	}
	else if( element == "overworld" || element == "Overworld" )
	{
		Handle_Overworld( m_xml_attributes );
	}
	else if( element == "waypoint" || element == "Overworld_Level" )
	{
		Handle_Overworld_Waypoint( m_xml_attributes );
		// don't clear attributes to keep the world "name"
		return;
	}
	else if( element == "savegame" || element == "Savegame" )
	{
		// ignore
	}
	else if( element.length() )
	{
		printf( "Warning : Savegame Unknown Element : %s\n", element.c_str() );
	}

	// clear
	m_xml_attributes = CEGUI::XMLAttributes();
}

void cSavegame_XML_Handler :: Handle_Level( const CEGUI::XMLAttributes &attributes )
{
	cSave_Level *save_level = new cSave_Level();
	save_level->m_name = m_xml_attributes.getValueAsString( "level_name" ).c_str();
	save_level->m_level_pos_x = m_xml_attributes.getValueAsFloat( "player_posx" );
	save_level->m_level_pos_y = m_xml_attributes.getValueAsFloat( "player_posy" );
	// set level objects
	save_level->m_level_objects.swap( m_level_objects );
	// set level spawned objects
	save_level->m_spawned_objects.swap( m_level_spawned_objects );

	// save
	m_savegame->m_levels.push_back( save_level );
}

void cSavegame_XML_Handler :: Handle_Level_Object( const CEGUI::XMLAttributes &attributes )
{
	int type = m_xml_attributes.getValueAsInteger( "type" );

	if( type <= 0 )
	{
		printf( "Warning : cSavegame::Handle_Level_Object : Unknown type %d\n", type );
		return;
	}

	cSave_Level_Object *object = new cSave_Level_Object();

	// type
	object->m_type = static_cast<SpriteType>(type);
	m_xml_attributes.remove( "type" );


	// Get properties
	for( unsigned int i = 0; i < m_xml_attributes.getCount(); i++ )
	{
		// get property
		std::string property_name = m_xml_attributes.getName( i ).c_str();

		// ignore level attributes
		if( property_name.compare( "level_name" ) == 0 || property_name.compare( "player_posx" ) == 0 || property_name.compare( "player_posy" ) == 0 )
		{
			continue;
		}

		object->m_properties.push_back( cSave_Level_Object_Property( property_name, m_xml_attributes.getValue( i ).c_str() ) );
	}

	// remove used properties
	for( Save_Level_Object_ProprtyList::iterator prop_itr = object->m_properties.begin(); prop_itr != object->m_properties.end(); ++prop_itr )
	{
		cSave_Level_Object_Property Property = (*prop_itr);

		m_xml_attributes.remove( Property.m_name );
	}

	// add object
	m_level_objects.push_back( object );
}

void cSavegame_XML_Handler :: Handle_Level_Spawned_Object( const CEGUI::String &element, CEGUI::XMLAttributes &attributes )
{
	cSprite *sprite = Create_Level_Object_From_XML( element, attributes, m_savegame->m_level_engine_version, pActive_Level->m_sprite_manager );

	if( !sprite )
	{
		return;
	}
	
	m_level_spawned_objects.push_back( sprite );
}

void cSavegame_XML_Handler :: Handle_Player( const CEGUI::XMLAttributes &attributes )
{
	m_savegame->m_lives = m_xml_attributes.getValueAsInteger( "lives" );
	m_savegame->m_points = string_to_long( m_xml_attributes.getValueAsString( "points", "0" ).c_str() );
	m_savegame->m_goldpieces = m_xml_attributes.getValueAsInteger( "goldpieces" );
	m_savegame->m_player_type = m_xml_attributes.getValueAsInteger( "type" );
	m_savegame->m_player_state = m_xml_attributes.getValueAsInteger( "state" );
	m_savegame->m_itembox_item = m_xml_attributes.getValueAsInteger( "itembox_item" );
	// new in V.11
	m_savegame->m_level_time = m_xml_attributes.getValueAsInteger( "level_time" );
	if( !m_old_format )
	{
		m_savegame->m_overworld_active = m_xml_attributes.getValueAsString( "overworld_active" ).c_str();
		m_savegame->m_overworld_current_waypoint = m_xml_attributes.getValueAsInteger( "overworld_current_waypoint" );
	}
}

void cSavegame_XML_Handler :: Handle_Overworld_Data( const CEGUI::XMLAttributes &attributes )
{
	// savegame format V.10 and lower
	m_savegame->m_overworld_active = m_xml_attributes.getValueAsString( "active" ).c_str();
	m_savegame->m_overworld_current_waypoint = m_xml_attributes.getValueAsInteger( "current_waypoint" );
}

void cSavegame_XML_Handler :: Handle_Overworld( const CEGUI::XMLAttributes &attributes )
{
	std::string name = m_xml_attributes.getValueAsString( "name" ).c_str();

	// is overworld available
	cOverworld *overworld = pOverworld_Manager->Get_from_Name( name );

	if( !overworld )
	{
		printf( "Warning : Savegame %s Overworld %s not found\n", m_savegame->m_description.c_str(), name.c_str() );
	}

	// Create savegame overworld
	cSave_Overworld *save_overworld = new cSave_Overworld();
	save_overworld->m_name = name;
	save_overworld->m_waypoints.swap( m_active_waypoints );
	m_active_waypoints.clear();
	// save
	m_savegame->m_overworlds.push_back( save_overworld );
}

void cSavegame_XML_Handler :: Handle_Overworld_Waypoint( const CEGUI::XMLAttributes &attributes )
{
	bool access = m_xml_attributes.getValueAsBool( "access" );

	cSave_Overworld_Waypoint *waypoint = new cSave_Overworld_Waypoint();

	// destination ( level_name and world_name is pre 0.99.6 )
	if( m_xml_attributes.exists( "world_name" ) )
	{
		waypoint->m_destination = m_xml_attributes.getValueAsString( "world_name" ).c_str();
	}
	else if( m_xml_attributes.exists( "level_name" ) )
	{
		waypoint->m_destination = m_xml_attributes.getValueAsString( "level_name" ).c_str();
	}
	// default
	else
	{
		waypoint->m_destination = m_xml_attributes.getValueAsString( "destination" ).c_str();
	}

	waypoint->m_access = access;

	m_active_waypoints.push_back( waypoint );

	// remove level data
	m_xml_attributes.remove( "destination" );
	m_xml_attributes.remove( "access" );
	m_xml_attributes.remove( "level_name" );
	m_xml_attributes.remove( "world_name" );
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cSavegame *pSavegame = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
