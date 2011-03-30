/***************************************************************************
 * filesystem.h
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

#ifndef SMC_FILESYSTEM_H
#define SMC_FILESYSTEM_H

#include "../../core/global_basic.h"

namespace SMC
{

/* *** *** *** *** *** filesystem functions *** *** *** *** *** *** *** *** *** *** *** *** */

// Return the trimmed filename with the given options
std::string Trim_Filename( std::string filename, bool keep_dir = 1, bool keep_end = 1 );

// Check if the file exists
bool File_Exists( const std::string &filename );
// Check if the directory exists
bool Dir_Exists( const std::string &dir );
/* Deletes the given file.
 * Use with caution.
 * Returns true on success
*/
bool Delete_File( const std::string &filename );
/* Deletes the given directory, which must be empty.
 * Use with caution.
 * Returns true on success
*/
bool Delete_Dir( const std::string &dir );
/* Deletes the given directory including the contents.
 * Use with caution.
 * Returns true on success
*/
bool Delete_Dir_And_Content( const std::string &dir );
/* Rename the given file
 * Returns true on success
*/
bool Rename_File( const std::string &old_filename, const std::string &new_filename );

/* Create directory.
* Returns 1 if a new directory was created, otherwise 0
*/
bool Create_Directory( const std::string &dir );
/* Create all needed directories.
* Returns 1 if the directories were created, otherwise 0
*/
bool Create_Directories( const std::string &dir );

/* Get the file size in bytes.
* returns 0 if the file does not exist
*/
size_t Get_File_Size( const std::string &filename );

// Converts "\" and "!" to "/"
void Convert_Path_Separators( std::string &str );

/* Get all files from the directory.
 * dir : the directory to scan
 * file_type : if set only this file type is returned
 * with_directories : if set adds directories to the returned objects
 * search_in_sub_directories : searches in every sub-directory
*/
vector<std::string> Get_Directory_Files( const std::string &dir, const std::string &file_type = "", bool with_directories = 0, bool search_in_sub_directories = 1 );

// Return the operating system temporary files directory
std::string Get_Temp_Directory( void );
// Return the default smc user directory in the operating system application/home directory
std::string Get_User_Directory( void );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
