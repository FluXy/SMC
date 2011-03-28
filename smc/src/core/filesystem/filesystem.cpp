/***************************************************************************
 * filesystem.cpp  -  File System
 *
 * Copyright (C) 2005 - 2011 Florian Richter
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
#include "../../core/game_core.h"
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
// fixme : boost should use a codecvt_facet but for now we convert to UCS-2
#ifdef _WIN32
	fs::file_type type = fs::status( fs::path( utf8_to_ucs2( filename ) ) ).type();
#else
	fs::file_type type = fs::status( fs::path( filename ) ).type();
#endif

	return type == fs::regular_file || type == fs::symlink_file;
}

bool Dir_Exists( const std::string &dir )
{
// fixme : boost should use a codecvt_facet but for now we convert to UCS-2
#ifdef _WIN32
	fs::file_type type = fs::status( fs::path( utf8_to_ucs2( dir ) ) ).type();
#else
	fs::file_type type = fs::status( fs::path( dir ) ).type();
#endif

	return type == fs::directory_file || type == fs::symlink_file;
}

bool Delete_File( const std::string &filename )
{
// fixme : boost should use a codecvt_facet but for now we convert to UCS-2
#ifdef _WIN32
	return DeleteFile( utf8_to_ucs2( filename ).c_str() ) != 0;
#else
	return remove( filename.c_str() ) == 0;
#endif
}

bool Delete_Dir( const std::string &dir )
{
// fixme : boost should use a codecvt_facet but for now we convert to UCS-2
#ifdef _WIN32
	return RemoveDirectory( utf8_to_ucs2( dir ).c_str() ) != 0;
#else
	return rmdir( dir.c_str() ) == 0;
#endif
}

bool Delete_Dir_And_Content( const std::string &dir )
{
// fixme : boost should use a codecvt_facet but for now we convert to UCS-2
#ifdef _WIN32
	return fs::remove_all( fs::path( utf8_to_ucs2( dir ).c_str() ) ) > 0;
#else
	return fs::remove_all( fs::path( dir ) ) > 0;
#endif
}

bool Rename_File( const std::string &old_filename, const std::string &new_filename )
{
// fixme : boost should use a codecvt_facet but for now we convert to UCS-2
#ifdef _WIN32
	return MoveFileEx( utf8_to_ucs2( old_filename ).c_str(), utf8_to_ucs2( new_filename ).c_str(), MOVEFILE_REPLACE_EXISTING ) != 0;
#else
	return rename( old_filename.c_str(), new_filename.c_str() ) == 0;
#endif
}

bool Create_Directory( const std::string &dir )
{
	if( dir.empty() )
	{
		return 0;
	}

// fixme : boost should use a codecvt_facet but for now we convert to UCS-2
#ifdef _WIN32
	return CreateDirectory( utf8_to_ucs2( dir ).c_str(), NULL ) != 0;
#else
	return fs::create_directory( fs::path( dir ) );
#endif
}

bool Create_Directories( const std::string &dir )
{
// fixme : boost should use a codecvt_facet but for now we convert to UCS-2
#ifdef _WIN32
	return fs::create_directories( fs::path( utf8_to_ucs2( dir ).c_str() ) );
#else
	return fs::create_directories( fs::path( dir ) );
#endif
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
		// convert it
		if( *itr == '\\' || *itr == '!' )
		{
			*itr = '/';
		}
	}
}

vector<std::string> Get_Directory_Files( const std::string &dir, const std::string &file_type /* = "" */, bool with_directories /* = 0 */, bool search_in_sub_directories /* = 1 */ )
{
	vector<std::string> valid_files;

// fixme : boost should use a codecvt_facet but for now we convert to UCS-2
#ifdef _WIN32
	fs::path full_path( utf8_to_ucs2( dir ) );
#else
	fs::path full_path( dir );
#endif
	fs::directory_iterator end_iter;

	// load all available objects
	for( fs::directory_iterator dir_itr( full_path ); dir_itr != end_iter; ++dir_itr )
	{
		try
		{
			const std::string filename_str = dir_itr->path().filename().string();

			// if directory
			if( fs::is_directory( *dir_itr ) )
			{
				// ignore hidden directories
				if( filename_str.find( "." ) == 0 )
				{
					continue;
				}

				if( with_directories )
				{
					valid_files.push_back( dir + "/" + filename_str );
				}

				// load all items from the sub-directory
				if( search_in_sub_directories )
				{
					vector<std::string> new_valid_files = Get_Directory_Files( dir + "/" + filename_str, file_type, with_directories );
					valid_files.insert( valid_files.end(), new_valid_files.begin(), new_valid_files.end() );
				}
			}
			// valid file
			else if( file_type.empty() || filename_str.rfind( file_type ) != std::string::npos )
			{
				valid_files.push_back( dir + "/" + filename_str );
			}
		}
		catch( const std::exception &ex )
		{
			printf( "%s %s\n", dir_itr->path().string().c_str(), ex.what() );
		}
	}

	return valid_files;
}

std::string Get_Temp_Directory( void )
{
#ifdef _WIN32
	TCHAR path[MAX_PATH];

	DWORD retval = GetTempPath( MAX_PATH, path );

	if( retval > MAX_PATH || retval == 0 )
	{
		printf( "Error : Couldn't get Windows temp directory.\n" );
		return "";
	}

	std::string str_path = ucs2_to_utf8( path );
	Convert_Path_Separators( str_path );

	return str_path;
#else
	return fs::temp_directory_path().generic_string();
#endif
}

std::string Get_User_Directory( void )
{
#ifdef _WIN32
	TCHAR path_appdata[MAX_PATH + 1];

	if( FAILED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path_appdata ) ) )
	{
		printf( "Error : Couldn't get Windows user data directory. Defaulting to the Application directory.\n" );
		return "";
	}

	std::string str_path = ucs2_to_utf8( path_appdata );
	Convert_Path_Separators( str_path );

	/*std::wstring str = utf8_to_ucs2( str_path );
	str.insert( str.length(), L"\n" );
	HANDLE std_out = GetStdHandle( STD_OUTPUT_HANDLE );
	unsigned long chars;
	WriteConsole( std_out, str.c_str(), lstrlen(str.c_str()), &chars, NULL );*/

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
