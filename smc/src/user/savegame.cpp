/***************************************************************************
 * savegame.cpp  -  Savegame handler
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

/* *** *** *** *** *** *** *** cSave *** *** *** *** *** *** *** *** *** *** */

cSave :: cSave( void )
{
	Init();
}

cSave :: ~cSave( void )
{
	// clear
	for( Save_Level_ObjectList::iterator itr = m_level_objects.begin(); itr != m_level_objects.end(); ++itr )
	{
		delete *itr;
	}

	m_level_objects.clear();

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
	m_level_pos_x = 0;
	m_level_pos_y = 0;

	// overworld
	m_overworld_current_waypoint = 0;
}

cSavegame :: cSavegame( void )
{
	m_save_temp = NULL;

	m_savegame_dir = pResource_Manager->user_data_dir + USER_SAVEGAME_DIR;
}

cSavegame :: ~cSavegame( void )
{
	//
}

int cSavegame :: Load_Game( unsigned int save_slot )
{
	cSave *savegame = Load( save_slot );
	
	// check if unsupported save
	if( savegame->m_version <= SAVEGAME_VERSION_UNSUPPORTED )
	{
		printf( "Warning : Savegame %d : Versions %d and below are unsupported\n", save_slot, SAVEGAME_VERSION_UNSUPPORTED );
	}

	// level available
	if( savegame->m_level_name.length() )
	{
		std::string level_name = savegame->m_level_name;

		// level not found
		if( !pLevel_Manager->Get_Path( level_name ) )
		{
			printf( "Warning : Savegame %d : Level not found : %s\n", save_slot, level_name.c_str() );
		}
	}

	// reset saved data
	pLevel_Player->Reset_Save();
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
		// Set Active Overworld
		if( !pOverworld_Manager->Set_Active( savegame->m_overworld_active ) )
		{
			printf( "Warning : Savegame %d : Couldn't set Overworld active %s\n", save_slot, savegame->m_overworld_active.c_str() );
		}

		// Current Waypoint
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

	// below version 8 the sate was the type
	if( savegame->m_version < 8 )
	{
		// type
		pLevel_Player->Set_Type( static_cast<Maryo_type>(savegame->m_player_state), 0, 0 );
	}
	else
	{
		// type
		pLevel_Player->Set_Type( static_cast<Maryo_type>(savegame->m_player_type), 0, 0 );
		// state
		pLevel_Player->m_state = static_cast<Moving_state>(savegame->m_player_state);
	}


	// in a level
	if( !savegame->m_level_name.empty() )
	{
		// load level
		cLevel *level = pLevel_Manager->Load( savegame->m_level_name );

		if( level )
		{
			pLevel_Manager->Set_Active( level );
			level->Init();

			// if below version 9 : move y coordinate bottom to 0 and remove screen height adjustment
			if( savegame->m_version < 9 )
			{
				savegame->m_level_pos_y -= 1200.0f;
			}

			// position
			pLevel_Player->Set_Pos( savegame->m_level_pos_x, savegame->m_level_pos_y );

			// Level Objects
			for( Save_Level_ObjectList::iterator itr = savegame->m_level_objects.begin(); itr != savegame->m_level_objects.end(); ++itr )
			{
				// get object pointer
				cSave_Level_Object *save_object = (*itr);

				// get position
				int posx = string_to_int( save_object->Get_Value( "posx" ) );
				int posy = string_to_int( save_object->Get_Value( "posy" ) );

				// get level object
				cSprite *level_object = level->m_sprite_manager->Get_from_Position( posx, posy, save_object->m_type );

				// if not anymore available
				if( !level_object )
				{
					printf( "Warning : Savegame object type %d on x %d, y %d not available\n", save_object->m_type, posx, posy );
					continue;
				}

				level_object->Load_From_Savegame( save_object );
			}

			// invincible for a second
			pLevel_Player->m_invincible = speedfactor_fps;
		}
		else
		{
			printf( "Error : Couldn't load Savegame Level %s\n", savegame->m_level_name.c_str() );
		}
	}

	// #### Player ####

	pHud_Points->Set_Points( savegame->m_points );
	pHud_Goldpieces->Set_Gold( savegame->m_goldpieces );
	pHud_Lives->Set_Lives( savegame->m_lives );
	pHud_Itembox->Set_Item( static_cast<SpriteType>(savegame->m_itembox_item), 0 );
	pHud_Debug->Set_Text( _("Savegame ") + int_to_string( save_slot ) + _(" loaded") );
	pHud_Manager->Update();

	// default is level save
	int retval = 1;

	// if Overworld Save
	if( savegame->m_level_name.empty() )
	{
		retval = 2;
	}

	delete savegame;
	return retval;
}

bool cSavegame :: Save_Game( unsigned int save_slot, std::string description )
{
	if( pLevel_Player->m_maryo_type == MARYO_DEAD || pLevel_Player->m_lives < 0 )
	{
		printf( "Error : Couldn't save savegame %s because of invalid game state\n", description.c_str() );
		return 0;
	}

	cSave *savegame = new cSave();

	// Time ( seconds since 1970 )
	savegame->m_save_time = time( NULL );
	// Version
	savegame->m_version = SAVEGAME_VERSION;
	// Description
	savegame->m_description = description;
	// Goldpieces
	savegame->m_goldpieces = pLevel_Player->m_goldpieces;

	// Level
	if( pActive_Level->Is_Loaded() )
	{
		// name
		savegame->m_level_name = Trim_Filename( pActive_Level->m_level_filename, 0, 0 );

		// position
		savegame->m_level_pos_x = pLevel_Player->m_pos_x;
		savegame->m_level_pos_y = pLevel_Player->m_pos_y - 5;

		// Level Objects
		for( cSprite_List::iterator itr = pActive_Level->m_sprite_manager->objects.begin(); itr != pActive_Level->m_sprite_manager->objects.end(); ++itr )
		{
			// get object pointer
			cSprite *object = (*itr);

			// get save data
			cSave_Level_Object *save_object = object->Save_To_Savegame();

			// nothing to save
			if( !save_object )
			{
				continue;
			}

			// add
			savegame->m_level_objects.push_back( save_object );
		}
	}

	// Lives
	savegame->m_lives = pLevel_Player->m_lives;
	// Points
	savegame->m_points = pLevel_Player->m_points;

	// Player type
	savegame->m_player_type = pLevel_Player->m_maryo_type;
	// Player state
	savegame->m_player_state = pLevel_Player->m_state;
	// Itembox Item
	savegame->m_itembox_item = pHud_Itembox->m_item_id;

	// save overworld progress
	for( vector<cOverworld *>::iterator itr = pOverworld_Manager->objects.begin(); itr != pOverworld_Manager->objects.end(); ++itr )
	{
		// Get Overworld
		cOverworld *overworld = (*itr);

		// create Overworld
		cSave_Overworld *save_overworld = new cSave_Overworld();
		// name
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

		// save
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

	// Save it
	Save( save_slot, savegame );

	// Print
	if( pHud_Debug )
	{
		pHud_Debug->Set_Text( _("Saved to Slot ") + int_to_string( save_slot ) );
	}

	// Clear
	delete savegame;

	return 1;
}

cSave *cSavegame :: Load( unsigned int save_slot )
{
	m_save_temp = new cSave();

	std::string filename = m_savegame_dir + "/" + int_to_string( save_slot ) + ".save";

	if( !File_Exists( filename ) )
	{
		printf( "Error : Savegame Loading : No Savegame found at Slot : %s\n", filename.c_str() );
		cSave *savegame = m_save_temp;
		m_save_temp = NULL;
		return savegame;
	}

	try
	{
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/Savegame.xsd", "" );
	}
	// catch CEGUI Exceptions
	catch( CEGUI::Exception &ex )
	{
		printf( "Loading Savegame %s CEGUI Exception %s\n", filename.c_str(), ex.getMessage().c_str() );
		pHud_Debug->Set_Text( _("Savegame Loading failed : ") + (const std::string)ex.getMessage().c_str() );
	}

	// if no description is available
	if( !m_save_temp->m_description.length() )
	{
		m_save_temp->m_description = _("No Description");
	}

	cSave *savegame = m_save_temp;
	m_save_temp = NULL;
	return savegame;
}

int cSavegame :: Save( unsigned int save_slot, cSave *savegame )
{
	std::string filename = m_savegame_dir + "/" + int_to_string( save_slot ) + ".save";

	if( File_Exists( filename ) )
	{
		ifstream ifs( filename.c_str(), ios::trunc ); // Delete existing
		ifs.close();
	}

	// empty overworld active
	if( savegame->m_overworld_active.empty() )
	{
		printf( "Warning : Savegame %s saving : Empty Overworld Active\n", savegame->m_description.c_str() );
	}

	ofstream file( filename.c_str(), ios::out );
	
	if( !file.is_open() )
	{
		printf( "Error : Couldn't open savegame file for saving. Is the file read-only ?" );
		return 0;
	}

	// xml info
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	// begin Savegame
	file << "<Savegame>" << std::endl;

	// begin Information
	file << "\t<Information>" << std::endl;
		// Description
		file << "\t\t<Property Name=\"description\" Value=\"" << savegame->m_description << "\" />" << std::endl;
		// Version
		file << "\t\t<Property Name=\"version\" Value=\"" << savegame->m_version << "\" />" << std::endl;
		// Time
		file << "\t\t<Property Name=\"save_time\" Value=\"" << savegame->m_save_time << "\" />" << std::endl;
	// end Information
	file << "\t</Information>" << std::endl;

	if( !savegame->m_level_name.empty() )
	{
		// begin Level
		file << "\t<Level>" << std::endl;

		// Level name
		file << "\t\t<Property Name=\"level_name\" Value=\"" << savegame->m_level_name << "\" />" << std::endl;
		// Level position
		file << "\t\t<Property Name=\"player_posx\" Value=\"" << savegame->m_level_pos_x << "\" />" << std::endl;
		file << "\t\t<Property Name=\"player_posy\" Value=\"" << savegame->m_level_pos_y << "\" />" << std::endl;

		// Level Objects
		for( Save_Level_ObjectList::iterator itr = savegame->m_level_objects.begin(); itr != savegame->m_level_objects.end(); ++itr )
		{
			// get object pointer
			cSave_Level_Object *object = (*itr);

			// begin Level Object
			file << "\t\t<Level_Object>" << std::endl;

			// Object type
			file << "\t\t\t<Property Name=\"type\" Value=\"" << object->m_type << "\" />" << std::endl;

			// Properties
			for( Save_Level_Object_ProprtyList::iterator prop_itr = object->m_properties.begin(); prop_itr != object->m_properties.end(); ++prop_itr )
			{
				// get properties pointer
				cSave_Level_Object_Property Property = (*prop_itr);

				// property
				file << "\t\t\t<Property Name=\"" << Property.m_name <<"\" Value=\"" << Property.m_value << "\" />" << std::endl;
			}

			// end Level Object
			file << "\t\t</Level_Object>" << std::endl;
		}
		// end Level
		file << "\t</Level>" << std::endl;
	}

	// begin Player
	file << "\t<Player>" << std::endl;
		// Lives
		file << "\t\t<Property Name=\"lives\" Value=\"" << savegame->m_lives << "\" />" << std::endl;
		// Points
		file << "\t\t<Property Name=\"points\" Value=\"" << savegame->m_points << "\" />" << std::endl;
		// Goldpieces
		file << "\t\t<Property Name=\"goldpieces\" Value=\"" << savegame->m_goldpieces << "\" />" << std::endl;
		// type
		file << "\t\t<Property Name=\"type\" Value=\"" << savegame->m_player_type << "\" />" << std::endl;
		// state
		file << "\t\t<Property Name=\"state\" Value=\"" << savegame->m_player_state << "\" />" << std::endl;
		// Itembox item
		file << "\t\t<Property Name=\"itembox_item\" Value=\"" << savegame->m_itembox_item << "\" />" << std::endl;
	// end Player
	file << "\t</Player>" << std::endl;

	// begin Overworld_Data
	file << "\t<Overworld_Data>" << std::endl;
		// active Overworld
		file << "\t\t<Property Name=\"active\" Value=\"" << savegame->m_overworld_active << "\" />" << std::endl;
		// current Overworld Waypoint
		file << "\t\t<Property Name=\"current_waypoint\" Value=\"" << savegame->m_overworld_current_waypoint << "\" />" << std::endl;
	// end Overworld_Data
	file << "\t</Overworld_Data>" << std::endl;

	// Overworlds
	for( Save_OverworldList::iterator itr = savegame->m_overworlds.begin(); itr != savegame->m_overworlds.end(); ++itr )
	{
		// get object pointer
		cSave_Overworld *overworld = (*itr);

		// begin Overworld
		file << "\t<Overworld>" << std::endl;

		// current Overworld
		file << "\t\t<Property Name=\"name\" Value=\"" << overworld->m_name << "\" />" << std::endl;

		for( Save_Overworld_WaypointList::iterator wp_itr = overworld->m_waypoints.begin(); wp_itr != overworld->m_waypoints.end(); ++wp_itr )
		{
			// get object pointer
			cSave_Overworld_Waypoint *overworld_waypoint = (*wp_itr);

			// skip empty waypoints
			if( overworld_waypoint->m_destination.empty() )
			{
				continue;
			}

			// begin Overworld Level
			file << "\t\t<Overworld_Level>" << std::endl;

			// destination
			file << "\t\t\t<Property Name=\"destination\" Value=\"" << overworld_waypoint->m_destination << "\" />" << std::endl;
			// access
			file << "\t\t\t<Property Name=\"access\" Value=\"" << overworld_waypoint->m_access << "\" />" << std::endl;

			// end Overworld Level
			file << "\t\t</Overworld_Level>" << std::endl;
		}

		// end Overworld
		file << "\t</Overworld>" << std::endl;
	}

	// end Savegame
	file << "</Savegame>" << std::endl;

	file.close();
	
	debug_print( "Saved Savegame %s to slot %d\n", filename.c_str(), save_slot );
	
	return 1;
}

std::string cSavegame :: Get_Description( unsigned int save_slot, bool only_description /* = 0 */ )
{
	std::string savefile, str_description;

	savefile = m_savegame_dir + "/" + int_to_string( save_slot ) + ".save";

	if( !File_Exists( savefile ) )
	{
		str_description = int_to_string( save_slot ) + ". Free Save";
		return str_description;
	}
	
	cSave *temp_savegame = Load( save_slot );

	// complete description
	if( !only_description )
	{
		str_description = int_to_string( save_slot ) + ". " + temp_savegame->m_description;

		if( temp_savegame->m_level_name.empty() )
		{
			str_description += " - " + temp_savegame->m_overworld_active;
		}
		else
		{
			str_description += _(" -  Level ") + temp_savegame->m_level_name;
		}

		str_description += _(" - Date ") + Time_to_String( temp_savegame->m_save_time, "%Y-%m-%d  %H:%M:%S" );
	}
	// only the user description
	else
	{
		return temp_savegame->m_description;
	}

	delete temp_savegame;
	return str_description;
}

bool cSavegame :: Is_Valid( unsigned int save_slot ) const
{
	return File_Exists( m_savegame_dir + "/" + int_to_string( save_slot ) + ".save" );
}

// XML element start
void cSavegame :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
{
	// Property/Item/Tag of an Element
	if( element == "Property" )
	{
		m_xml_attributes.add( attributes.getValueAsString( "Name" ), attributes.getValueAsString( "Value" ) );
	}
}

// XML element end
void cSavegame :: elementEnd( const CEGUI::String &element )
{
	if( element != "Property" )
	{
		if( element == "Information" )
		{
			m_save_temp->m_description = m_xml_attributes.getValueAsString( "description" ).c_str();
			m_save_temp->m_version = m_xml_attributes.getValueAsInteger( "version" );
			m_save_temp->m_save_time = m_xml_attributes.getValueAsInteger( "save_time" );
		}
		else if( element == "Level" )
		{
			Handle_Level( m_xml_attributes );
		}
		else if( element == "Level_Object" )
		{
			Handle_Level_Object( m_xml_attributes );
			// don't clear attributes
			return;
		}
		else if( element == "Player" )
		{
			Handle_Player( m_xml_attributes );
		}
		else if( element == "Overworld_Data" )
		{
			Handle_Overworld_Data( m_xml_attributes );
		}
		else if( element == "Overworld" )
		{
			Handle_Overworld( m_xml_attributes );
		}
		else if( element == "Overworld_Level" )
		{
			Handle_Overworld_Level( m_xml_attributes );
			// don't clear attributes
			return;
		}
		else if( element == "Savegame" )
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
}

void cSavegame :: Handle_Level( const CEGUI::XMLAttributes &attributes )
{
	m_save_temp->m_level_name = m_xml_attributes.getValueAsString( "level_name" ).c_str();
	m_save_temp->m_level_pos_x = m_xml_attributes.getValueAsFloat( "player_posx" );
	m_save_temp->m_level_pos_y = m_xml_attributes.getValueAsFloat( "player_posy" );

	// set level objects
	m_save_temp->m_level_objects.swap( m_level_objects );
	m_level_objects.clear();
}

void cSavegame :: Handle_Level_Object( const CEGUI::XMLAttributes &attributes )
{
	int type = m_xml_attributes.getValueAsInteger( "type" );

	if( type <= 0 )
	{
		printf( "Warning : Unknown Savegame Level Object type %d\n", type );
		return;
	}

	cSave_Level_Object *object = new cSave_Level_Object();

	// type
	object->m_type = static_cast<SpriteType>(type);
	m_xml_attributes.remove( "type" );


	// Get Properties
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

	// remove used Properties
	for( Save_Level_Object_ProprtyList::iterator prop_itr = object->m_properties.begin(); prop_itr != object->m_properties.end(); ++prop_itr )
	{
		// get property pointer
		cSave_Level_Object_Property Property = (*prop_itr);

		m_xml_attributes.remove( Property.m_name );
	}

	// add object
	m_level_objects.push_back( object );
}

void cSavegame :: Handle_Player( const CEGUI::XMLAttributes &attributes )
{
	m_save_temp->m_lives = m_xml_attributes.getValueAsInteger( "lives" );
	m_save_temp->m_points = string_to_long( m_xml_attributes.getValueAsString( "points", "0" ).c_str() );
	m_save_temp->m_goldpieces = m_xml_attributes.getValueAsInteger( "goldpieces" );
	m_save_temp->m_player_type = m_xml_attributes.getValueAsInteger( "type" );
	m_save_temp->m_player_state = m_xml_attributes.getValueAsInteger( "state" );
	m_save_temp->m_itembox_item = m_xml_attributes.getValueAsInteger( "itembox_item" );
}

void cSavegame :: Handle_Overworld_Data( const CEGUI::XMLAttributes &attributes )
{
	m_save_temp->m_overworld_active = m_xml_attributes.getValueAsString( "active" ).c_str();
	m_save_temp->m_overworld_current_waypoint = m_xml_attributes.getValueAsInteger( "current_waypoint" );
}

void cSavegame :: Handle_Overworld( const CEGUI::XMLAttributes &attributes )
{
	std::string name = m_xml_attributes.getValueAsString( "name" ).c_str();

	// Search if Overworld is available
	cOverworld *overworld = pOverworld_Manager->Get_from_Name( name );

	if( !overworld )
	{
		printf( "Warning : Savegame %s Overworld %s not found\n", m_save_temp->m_description.c_str(), name.c_str() );
	}

	// Create Savegame Overworld
	cSave_Overworld *save_overworld = new cSave_Overworld();
	// set name
	save_overworld->m_name = name;
	// set waypoints
	save_overworld->m_waypoints.swap( m_active_waypoints );
	m_active_waypoints.clear();
	// save
	m_save_temp->m_overworlds.push_back( save_overworld );
}

void cSavegame :: Handle_Overworld_Level( const CEGUI::XMLAttributes &attributes )
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

	// clear
	m_xml_attributes.remove( "level_name" );
	m_xml_attributes.remove( "world_name" );
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cSavegame *pSavegame = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
