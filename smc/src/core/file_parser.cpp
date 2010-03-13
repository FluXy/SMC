/***************************************************************************
 * file_parser.cpp  -  Image Settings Handler
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

#include "../core/global_basic.h"
#include "../core/file_parser.h"
#include <cstdio>

namespace SMC
{

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cFile_parser :: cFile_parser( void )
{
	error_count = 0;
}

cFile_parser :: ~cFile_parser( void )
{
	//
}

bool cFile_parser :: Parse( const std::string &filename )
{
	error_count = 0;

	ifstream ifs( filename.c_str(), ios::in );
	
	if( !ifs )
	{
		printf( "Could not load data file : %s\n", filename.c_str() );
		return 0;
	}

	data_file = filename;

	// maximum length of a line
	char contents[500];

	for( unsigned int i = 0; ifs.getline( contents, sizeof( contents ) ); i++ )
	{
		if( !Parse_Line( contents, i ) )
		{
			error_count++;
		}
	}

	return 1;
}

bool cFile_parser :: Parse_Line( std::string command, int line )
{
	// completely empty
	if( command.empty() )
	{
		return 1;
	}

	// linux support
	while( 1 )
	{
		std::string::size_type pos = command.find( '\r' );

		if( pos == std::string::npos )
		{
			break;
		}

		command.erase( pos, 1 );

		if( command.empty() )
		{
			return 1;
		}
	}

	// no tabs
	while( 1 )
	{
		std::string::size_type pos = command.find( '\t' );

		if( pos == std::string::npos )
		{
			break;
		}

		command.replace( pos, 1, " " );
	}

	// remove trailing spaces
	while( 1 )
	{
		std::string::size_type pos = command.find_last_of( ' ' );

		if( pos == std::string::npos || pos != command.length() - 1 )
		{
			break;
		}

		command.erase( pos, 1 );

		if( command.empty() )
		{
			return 1;
		}
	}

	// remove beginning spaces
	while( *command.begin() == ' ' )
	{
		command.erase( 0, 1 );

		if( command.empty() )
		{
			return 1;
		}
	}

	// ignore comments and empty lines
	if( command.empty() || *command.begin() == '#' )
	{
		// no error
		return 1;
	}

	std::string tempstr = command;
	int count = 1;

	// Count spaces
	while( tempstr.find( ' ' ) != std::string::npos  )
	{
		tempstr.erase( tempstr.find( ' ' ) , 1 );
		count++;
	}

	tempstr = command;
	
	std::string *parts = new std::string[ count + 1 ];
	
	std::string::size_type len;
	int part_count = 0;

	while( count > 0 )
	{
		// get length of the part
		len = tempstr.find_first_of( ' ' );
		// create part
		parts[part_count] = tempstr.substr( 0, len );
		// remove part from temp string
		tempstr.erase( 0, len + 1 );


		part_count++;
		count--;
	}

	parts[part_count] = tempstr;

	// Message handler
	bool success = HandleMessage( parts, part_count, line );

	delete []parts;

	return success;
}

bool cFile_parser :: HandleMessage( const std::string *parts, unsigned int count, unsigned int line )
{
	// virtual
	return 1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
