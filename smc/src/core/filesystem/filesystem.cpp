/***************************************************************************
 * filesystem.cpp  -  File System
 *
 * Copyright (C) 2005 - 2009 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../../core/filesystem/filesystem.h"
// boost filesystem
#include "boost/filesystem/convenience.hpp"
namespace fs = boost::filesystem;
// needed for the stat function and to get the user directory on unix
#include <sys/stat.h>
#include <sys/types.h>
#if _WIN32
	// needed to get the user directory (SHGetFolderPath)
	#include <shlobj.h>
#endif

namespace SMC
{

/* *** *** *** *** *** *** cResource_Manager *** *** *** *** *** *** *** *** *** *** *** */

std::string Trim_Filename( std::string filename, bool keep_dir /* = 1 */, bool keep_end /* = 1 */ )
{
	if( !keep_dir && filename.find( "/" ) != std::string::npos ) 
	{
		filename.erase( 0, filename.rfind( "/" ) + 1 );
	}

	if( !keep_end && filename.rfind( "." ) != std::string::npos ) 
	{
		filename.erase( filename.rfind( "." ) );
	}

	return filename;
}

bool File_Exists( const std::string &filename )
{
	struct stat file_info; 

	if( stat( filename.c_str(), &file_info ) == 0 )
	{
		// check if regular file
	#ifdef _WIN32
		return (file_info.st_mode & S_IFREG) > 0;
	#else
		return !S_ISDIR(file_info.st_mode);
	#endif
	}

	return 0;
}

bool Dir_Exists( const std::string &dir )
{
	return fs::exists( fs::path( dir, fs::native ) );

	/*struct stat file_info;

	// if file exists
	if( stat( dir.c_str(), &file_info ) == 0 )
	{
		// check if directory
	#ifdef _WIN32
		return (file_info.st_mode & S_IFDIR) > 0;
	#else
		return S_ISDIR(file_info.st_mode);
	#endif
	}

	return 0;*/
}

bool Delete_File( const std::string &filename )
{
	return remove( filename.c_str() ) == 0;
}

bool Create_Directory( const std::string &dir )
{
	return fs::create_directory( fs::path( dir, fs::native ) );
}

size_t Get_File_Size( const std::string &filename )
{
	struct stat file_info; 

	// if file exists
	if( stat( filename.c_str(), &file_info ) == 0 )
	{
		// if regular file
	#ifdef _WIN32
		if( (file_info.st_mode & S_IFREG) > 0 )
	#else
		if( !S_ISDIR(file_info.st_mode) )
	#endif
		{
			// return size
			return file_info.st_size;
		}
	}

	return 0;
}

void Convert_Path_Separators( std::string &str )
{
	for( std::string::iterator itr = str.begin(); itr != str.end(); ++itr )
	{
		// fix it
		if( *itr == '\\' || *itr == '!' )
		{
			*itr = '/';
		}
	}
}

vector<std::string> Get_Directory_Files( const std::string &dir, const std::string &file_type /* = "" */, bool with_directories /* = 0 */, bool search_in_sub_directories /* = 1 */ )
{
	vector<std::string> valid_files;

	fs::path full_path( dir, fs::native );
	fs::directory_iterator end_iter;

	// load all available objects
	for( fs::directory_iterator dir_itr( full_path ); dir_itr != end_iter; ++dir_itr )
	{
		try
		{
			// if directory
			if( fs::is_directory( *dir_itr ) )
			{
				// ignore hidden directories
				if( dir_itr->path().leaf().find( "." ) == 0 )
				{
					continue;
				}

				if( with_directories )
				{
					valid_files.push_back( dir + "/" + dir_itr->path().leaf() );
				}

				// load all items from the sub-directory
				if( search_in_sub_directories )
				{
					vector<std::string> new_valid_files = Get_Directory_Files( dir + "/" + dir_itr->path().leaf(), file_type, with_directories );
					valid_files.insert( valid_files.end(), new_valid_files.begin(), new_valid_files.end() );
				}
			}
			// valid file
			else if( file_type.empty() || dir_itr->path().leaf().rfind( file_type ) != std::string::npos )
			{
				valid_files.push_back( dir + "/" + dir_itr->path().leaf() );
			}
		}
		catch( const std::exception &ex )
		{
			printf( "%s %s\n", dir_itr->path().leaf().c_str(), ex.what() );
		}
	}

	return valid_files;
}

std::string Get_Temp_Directory( void )
{
#ifdef _WIN32
	char path[MAX_PATH];

	DWORD retval = GetTempPath( MAX_PATH, path );

	if( retval > MAX_PATH || ( retval == 0 ) )
	{
		printf( "Error : Couldn't get Windows temp directory.\n" );
		return "";
	}

	std::string str_path = path;
	Convert_Path_Separators( str_path );

	return str_path;
#elif __unix__
	return (std::string)getenv( "TMPDIR" ) + "/";
#elif __APPLE__
	// fixme : same as unix ?
	return "";
#else
	return "";
#endif
}

std::string Get_User_Directory( void )
{
#ifdef _WIN32
	TCHAR path_appdata[MAX_PATH + 1];

	if( FAILED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0 /*SHGFP_TYPE_CURRENT*/, path_appdata ) ) )
	{
		printf( "Error : Couldn't get Windows user data directory. Defaulting to the Application directory.\n" );
		return "";
	}

	std::string str_path = path_appdata;
	Convert_Path_Separators( str_path );

	return str_path + "/smc/";
#elif __unix__
	return (std::string)getenv( "HOME" ) + "/.smc/";
#elif __APPLE__
	return (std::string)getenv( "HOME" ) + "/Library/Application Support/smc/";
#else
	return "";
#endif
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
