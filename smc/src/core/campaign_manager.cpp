/***************************************************************************
 * campaign_manager.cpp  -  class for handling campaigns
 *
 * copyright (C) 2010 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/campaign_manager.h"
#include "../gui/hud.h"
#include "../core/game_core.h"
#include "../core/filesystem/filesystem.h"
#include "../core/filesystem/resource_manager.h"
#include "../core/i18n.h"
// CEGUI
#include "CEGUIXMLParser.h"
#include "CEGUIExceptions.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cCampaign *** *** *** *** *** *** *** *** *** *** */

cCampaign :: cCampaign( void )
{
	m_is_target_level = 0;
	m_user = 0;
}

cCampaign :: ~cCampaign( void )
{

}

bool cCampaign :: Save( const std::string &filename )
{
// fixme : Check if there is a more portable way f.e. with imbue()
#ifdef _WIN32
	ofstream file( utf8_to_ucs2( filename ).c_str(), ios::out | ios::trunc );
#else
	ofstream file( filename.c_str(), ios::out | ios::trunc );
#endif

	
	if( !file.is_open() )
	{
		printf( "Error : Couldn't open campaign file for saving. Is the file read-only ?" );
		pHud_Debug->Set_Text( _("Couldn't save campaign ") + filename, speedfactor_fps * 5.0f );
		return 0;
	}

	CEGUI::XMLSerializer stream( file );

	// begin
	stream.openTag( "campaign" );

	// begin
	stream.openTag( "information" );
		Write_Property( stream, "name", m_name );
		Write_Property( stream, "description", m_description );
		Write_Property( stream, "save_time", static_cast<Uint64>( time( NULL ) ) );
	// end information
	stream.closeTag();

	// begin
	stream.openTag( "target" );
		Write_Property( stream, "name", m_target );
		Write_Property( stream, "is_level", m_is_target_level );
	// end target
	stream.closeTag();

	// end campaign
	stream.closeTag();

	file.close();
	
	debug_print( "Saved campaign %s\n", filename.c_str() );
	
	return 1;
}

/* *** *** *** *** *** *** *** cCampaign_Manager *** *** *** *** *** *** *** *** *** *** */

cCampaign_Manager :: cCampaign_Manager( void )
{
	Load();
}

cCampaign_Manager :: ~cCampaign_Manager( void )
{
	Delete_All();
}

void cCampaign_Manager :: Load( void )
{
	std::string user_dir = pResource_Manager->user_data_dir + USER_CAMPAIGN_DIR;
	std::string game_dir = DATA_DIR "/" GAME_CAMPAIGN_DIR;

	vector<std::string> user_files = Get_Directory_Files( pResource_Manager->user_data_dir + USER_CAMPAIGN_DIR, "smccpn", 0, 0 );
	vector<std::string> game_files = Get_Directory_Files( DATA_DIR "/" GAME_CAMPAIGN_DIR, "smccpn", 0, 0 );

	// get directory length for erasing
	int user_dir_length = user_dir.length() + 1;
	int game_dir_length = game_dir.length() + 1;

	for( vector<std::string>::iterator itr = user_files.begin(); itr != user_files.end(); ++itr )
	{
		std::string user_name = (*itr);
		cCampaign *campaign = Load_Campaign( user_name );

		// remove base directory
		user_name.erase( 0, user_dir_length );

		if( campaign )
		{
			Add( campaign );
			campaign->m_user = 1;

			// remove name from game files
			for( vector<std::string>::iterator game_itr = game_files.begin(); game_itr != game_files.end(); ++game_itr )
			{
				std::string game_name = (*game_itr);
				// remove base directory
				game_name.erase( 0, game_dir_length );

				if( user_name.compare( game_name ) == 0 )
				{
					campaign->m_user = 2;
					game_files.erase( game_itr );
					break;
				}
			}
		}
	}

	for( vector<std::string>::iterator itr = game_files.begin(); itr != game_files.end(); ++itr )
	{
		std::string name = (*itr);

		cCampaign *campaign = Load_Campaign( name );

		if( campaign )
		{
			Add( campaign );
		}
	}
}

cCampaign *cCampaign_Manager :: Load_Campaign( const std::string &filename )
{
	if( !File_Exists( filename ) )
	{
		printf( "Error : Campaign loading failed : %s\n", filename.c_str() );
		return NULL;
	}

	cCampaign_XML_Handler *loader = new cCampaign_XML_Handler( filename );
	cCampaign *campaign = loader->m_campaign;
	loader->m_campaign = NULL;
	delete loader;

	return campaign;
}

cCampaign *cCampaign_Manager :: Get_from_Name( const std::string &name )
{
	for( vector<cCampaign *>::iterator itr = objects.begin(); itr != objects.end(); ++itr )
	{
		cCampaign *obj = (*itr);

		if( obj->m_name.compare( name ) == 0 )
		{
			return obj;
		}
	}

	return NULL;
}

/* *** *** *** *** *** *** *** cCampaign_XML_Handler *** *** *** *** *** *** *** *** *** *** */

cCampaign_XML_Handler :: cCampaign_XML_Handler( const CEGUI::String &filename )
{
	m_campaign = new cCampaign();

	try
	{
	// fixme : Workaround for std::string to CEGUI::String utf8 conversion. Check again if CEGUI 0.8 works with std::string utf8
	#ifdef _WIN32
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, (const CEGUI::utf8*)filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/" "Campaign.xsd", "" );
	#else
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/" "Campaign.xsd", "" );
	#endif
	}
	// catch CEGUI Exceptions
	catch( CEGUI::Exception &ex )
	{
		printf( "Loading Campaign %s CEGUI Exception %s\n", filename.c_str(), ex.getMessage().c_str() );
		pHud_Debug->Set_Text( _("Campaign Loading failed : ") + (const std::string)ex.getMessage().c_str() );
	}
}

cCampaign_XML_Handler :: ~cCampaign_XML_Handler( void )
{
	if( m_campaign )
	{
		delete m_campaign;
	}
}

void cCampaign_XML_Handler :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
{
	if( element == "property" )
	{
		m_xml_attributes.add( attributes.getValueAsString( "name" ), attributes.getValueAsString( "value" ) );
	}
}

void cCampaign_XML_Handler :: elementEnd( const CEGUI::String &element )
{
	if( element == "property" )
	{
		return;
	}

	if( element == "information" )
	{
		m_campaign->m_name = m_xml_attributes.getValueAsString( "name" ).c_str();
		m_campaign->m_description = m_xml_attributes.getValueAsString( "description" ).c_str();
		m_campaign->m_last_saved = string_to_int64( m_xml_attributes.getValueAsString( "save_time" ).c_str() );
	}
	else if( element == "target" )
	{
		m_campaign->m_target = m_xml_attributes.getValueAsString( "name" ).c_str();
		m_campaign->m_is_target_level = m_xml_attributes.getValueAsBool( "is_level" );
	}
	else if( element == "campaign" )
	{
		// ignore
	}
	else if( element.length() )
	{
		printf( "Warning : Campaign Unknown Element : %s\n", element.c_str() );
	}

	// clear
	m_xml_attributes = CEGUI::XMLAttributes();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cCampaign_Manager *pCampaign_Manager = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
