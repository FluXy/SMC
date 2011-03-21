/***************************************************************************
 * resource_manager.cpp  -  Resource Manager
 *
 * Copyright (C) 2009 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../../core/filesystem/resource_manager.h"
#include "../../core/filesystem/filesystem.h"
#include <cstdio>


namespace SMC
{

/* *** *** *** *** *** *** cResource_Manager *** *** *** *** *** *** *** *** *** *** *** */

cResource_Manager :: cResource_Manager( void )
{
	user_data_dir = "";
}

cResource_Manager :: ~cResource_Manager( void )
{

}

void cResource_Manager :: Init_User_Directory( void )
{
	// Create user directory
	if( !Dir_Exists( user_data_dir ) )
	{
		// first run if not available
		Create_Directory( user_data_dir );
	}
	// Create savegame directory
	if( !Dir_Exists( user_data_dir + USER_SAVEGAME_DIR ) )
	{
		Create_Directory( user_data_dir + USER_SAVEGAME_DIR );
	}
	// Create screenshot directory
	if( !Dir_Exists( user_data_dir + USER_SCREENSHOT_DIR ) )
	{
		Create_Directory( user_data_dir + USER_SCREENSHOT_DIR );
	}
	// Create level directory
	if( !Dir_Exists( user_data_dir + USER_LEVEL_DIR ) )
	{
		Create_Directory( user_data_dir + USER_LEVEL_DIR );
	}
	// Create world directory
	if( !Dir_Exists( user_data_dir + USER_WORLD_DIR ) )
	{
		Create_Directory( user_data_dir + USER_WORLD_DIR );
	}
	// Create campaign directory
	if( !Dir_Exists( user_data_dir + USER_CAMPAIGN_DIR ) )
	{
		Create_Directory( user_data_dir + USER_CAMPAIGN_DIR );
	}
	// Create cache directory
	if( !Dir_Exists( user_data_dir + USER_IMGCACHE_DIR ) )
	{
		Create_Directory( user_data_dir + USER_IMGCACHE_DIR );
	}
}

bool cResource_Manager :: Set_User_Directory( const std::string &dir )
{
	user_data_dir = dir;

	return 1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cResource_Manager *pResource_Manager = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
