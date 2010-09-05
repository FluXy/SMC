/***************************************************************************
 * worlds.cpp  -  class for handling worlds data
 *
 * Copyright (C) 2004 - 2010 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
 
#include "../overworld/world_manager.h"
#include "../core/game_core.h"
#include "../overworld/overworld.h"
#include "../core/filesystem/filesystem.h"
#include "../core/filesystem/resource_manager.h"
#include "../overworld/world_editor.h"
#include "../input/mouse.h"
#include "../video/animation.h"
// boost filesystem
#include "boost/filesystem/convenience.hpp"
namespace fs = boost::filesystem;
// CEGUI
#include "CEGUIXMLParser.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** cOverworld_Manager *** *** *** *** *** *** *** *** *** */

cOverworld_Manager :: cOverworld_Manager( cSprite_Manager *sprite_manager )
: cObject_Manager<cOverworld>()
{
	m_worlds_filename = DATA_DIR "/" GAME_OVERWORLD_DIR "/worlds.xml";

	m_debug_mode = 0;
	m_draw_layer = 0;
	m_camera_mode = 0;

	m_camera = new cCamera( sprite_manager );

	Init();
}

cOverworld_Manager :: ~cOverworld_Manager( void )
{
	Delete_All();

	delete m_camera;
}

bool cOverworld_Manager :: New( std::string name )
{
	string_trim( name, ' ' );

	// no name given
	if( name.empty() )
	{
		return 0;
	}

	// name already exists
	if( Get_from_Path( name ) )
	{
		return 0;
	}

	cOverworld *overworld = new cOverworld();
	overworld->New( name );
	objects.push_back( overworld );

	return 1;
}

void cOverworld_Manager :: Init( void )
{
	// if already loaded
	if( !objects.empty() )
	{
		Delete_All();
	}

	// Load Worlds
	Load_Dir( pResource_Manager->user_data_dir + USER_WORLD_DIR, 1 );
	Load_Dir( DATA_DIR "/" GAME_OVERWORLD_DIR );

	// Get Overworld user comments
	if( File_Exists( m_worlds_filename ) )
	{
		// Parse
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, m_worlds_filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/Worlds_User_Data.xsd", "" );
	}
	else
	{
		// filename not valid
		printf( "Warning : Couldn't open Worlds description file : %s\n", m_worlds_filename.c_str() );
	}
}

void cOverworld_Manager :: Load_Dir( const std::string &dir, bool user_dir /* = 0 */ ) 
{
	// set world directory
	fs::path full_path( dir, fs::native );
	fs::directory_iterator end_iter;

	for( fs::directory_iterator dir_itr( full_path ); dir_itr != end_iter; ++dir_itr )
	{
		try
		{
			std::string current_dir = dir_itr->path().leaf();

			// only directories with an existing description
			if( fs::is_directory( *dir_itr ) && File_Exists( dir + "/" + current_dir + "/description.xml" ) )
			{
				cOverworld *overworld = Get_from_Path( current_dir );

				// already available
				if( overworld )
				{
					overworld->m_description->m_user = 2;
					continue;
				}

				overworld = new cOverworld();

				// set path
				overworld->m_description->m_path = current_dir;
				// default name is the path
				overworld->m_description->m_name = current_dir;
				// set user
				overworld->m_description->m_user = user_dir;

				objects.push_back( overworld );

				overworld->Load();
			}
		}
		catch( const std::exception &ex )
		{
			printf( "%s %s\n", dir_itr->path().leaf().c_str(), ex.what() );
		}
	}
}

bool cOverworld_Manager :: Set_Active( const std::string &str ) 
{
	return Set_Active( Get( str ) );
}

bool cOverworld_Manager :: Set_Active( cOverworld *world )
{
	if( !world )
	{
		return 0;
	}

	pActive_Overworld = world;

	pWorld_Editor->Set_Sprite_Manager( world->m_sprite_manager );
	pWorld_Editor->Set_Overworld( world );
	m_camera->Set_Sprite_Manager( world->m_sprite_manager );
	pOverworld_Player->Set_Sprite_Manager( world->m_sprite_manager );
	pOverworld_Player->Set_Overworld( world );
	pOverworld_Player->Reset();

	// pre-update animations
	for( cSprite_List::iterator itr = world->m_sprite_manager->objects.begin(); itr != world->m_sprite_manager->objects.end(); ++itr )
	{
		cSprite *obj = (*itr);

		if( obj->m_type == TYPE_PARTICLE_EMITTER )
		{
			cParticle_Emitter *emitter = static_cast<cParticle_Emitter *>(obj);
			emitter->Pre_Update();
		}
	}

	return 1;
}

void cOverworld_Manager :: Reset( void )
{
	// default Overworld
	Set_Active( "World 1" );

	// Set Player to first Waypoint
	pOverworld_Player->Set_Waypoint( pActive_Overworld->m_player_start_waypoint );

	// Reset all Waypoints
	for( vector<cOverworld *>::iterator itr = objects.begin(); itr != objects.end(); ++itr )
	{
		(*itr)->Reset_Waypoints();
	}
}

cOverworld *cOverworld_Manager :: Get( const std::string &str )
{
	cOverworld *world = Get_from_Name( str );

	if( world )
	{
		return world;
	}

	return Get_from_Path( str );
}

cOverworld *cOverworld_Manager :: Get_from_Path( const std::string &path )
{
	for( vector<cOverworld *>::iterator itr = objects.begin(); itr != objects.end(); ++itr )
	{
		cOverworld *obj = (*itr);

		if( obj->m_description->m_path.compare( path ) == 0 )
		{
			return obj;
		}
	}

	return NULL;
}

cOverworld *cOverworld_Manager :: Get_from_Name( const std::string &name )
{
	for( vector<cOverworld *>::iterator itr = objects.begin(); itr != objects.end(); ++itr )
	{
		cOverworld *obj = (*itr);

		if( obj->m_description->m_name.compare( name ) == 0 )
		{
			return obj;
		}
	}

	return NULL;
}

int cOverworld_Manager :: Get_Array_Num( const std::string &path ) const
{
	for( unsigned int i = 0; i < objects.size(); i++ )
	{
		if( objects[i]->m_description->m_path.compare( path ) == 0 )
		{
			return i;
		}
	}

	return -1;
}

void cOverworld_Manager :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
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

void cOverworld_Manager :: elementEnd( const CEGUI::String &element )
{
	if( element == "property" || element == "Property" )
	{
		return;
	}

	if( element == "World" )
	{
		handle_world( m_xml_attributes );
	}
	else if( element == "Worlds" )
	{
		// ignore
	}
	else if( element.length() )
	{
		printf( "Warning : Overworld Description Unknown element : %s\n", element.c_str() );
	}

	// clear
	m_xml_attributes = CEGUI::XMLAttributes();
}

void cOverworld_Manager :: handle_world( const CEGUI::XMLAttributes &attributes )
{
	std::string ow_name = attributes.getValueAsString( "Name" ).c_str();
	std::string ow_comment = attributes.getValueAsString( "Comment" ).c_str();

	// if available
	cOverworld *overworld = Get_from_Name( ow_name );

	// set comment
	if( overworld )
	{
		overworld->m_description->m_comment = ow_comment;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Overworld information handler
cOverworld_Manager *pOverworld_Manager = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
