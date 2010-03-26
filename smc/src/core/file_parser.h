/***************************************************************************
 * file_parser.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2005 - 2010 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_FILE_PARSER_H
#define SMC_FILE_PARSER_H

#include "../core/global_game.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** cFile_parser *** *** *** *** *** *** *** *** *** */

// Base class for parsing text files
class cFile_parser
{
public:
	cFile_parser( void );
	virtual ~cFile_parser( void );

	// Parses the given file
	bool Parse( const std::string &filename );

	// Parses a line of a description file into string parts
	bool Parse_Line( std::string str_line, int line_num );

	// Handle one line of the file
	virtual bool HandleMessage( const std::string *parts, unsigned int count, unsigned int line );

	// data filename
	std::string data_file;
	// data error count
	unsigned int error_count;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif

