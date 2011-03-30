/***************************************************************************
 * property_helper.h
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

#ifndef SMC_PROPERTY_HELPER_H
#define SMC_PROPERTY_HELPER_H

#include "../core/global_basic.h"
#include "../core/global_game.h"
#include "SDL.h"
#include <algorithm>
// CEGUI
#include "CEGUIString.h"

namespace SMC
{

/* Replace all occurrences of the search with the format string
 * todo : use boost::algorithm::replace_all ?
*/
void string_replace_all( std::string &str, const std::string &search, const std::string &format );
void cegui_string_replace_all( CEGUI::String &str, const CEGUI::String &search, const CEGUI::String &format );

/* Remove all occurrences of the search in the string
 * todo : use boost::algorithm::erase_all ?
*/
inline void string_erase_all( std::string &str, const char search )
{
	str.erase( std::remove(str.begin(), str.end(), search), str.end() );
};

/* Trim the string from the beginning with the given character
 * todo : use boost::algorithm::trim_left ?
*/
inline void string_trim_from_begin( std::string &str, const char search )
{
	str.erase( str.find_last_not_of( search ) + 1 );
};
/* Trim the string from the end with the given character
 * todo : use boost::algorithm::trim_right ?
*/
std::string string_trim_from_end( std::string str, const char search );
/* Trim the string from the beginning and end with the given character
 * todo : use boost::algorithm::trim ?
*/
inline void string_trim( std::string &str, const char search )
{
	string_trim_from_begin( str, search );
	string_trim_from_end( str, search );
};

// Return the number as a string
std::string int_to_string( const int number );
std::string int64_to_string( const Uint64 number );
std::string long_to_string( const long number );
/* Return the float as a string
 * prec: the precision after the decimal point
 * keep_zeros: keep trailing zeros in the fractional part
*/
std::string float_to_string( double value, int prec = 6, bool keep_zeros = 1 );
// Return the string as a number
int string_to_int( const std::string &str );
Uint64 string_to_int64( const std::string &str );
long string_to_long( const std::string &str );
// Return the string as a float
float string_to_float( const std::string &str );
// Return the string as a double
double string_to_double( const std::string &str );
// Return the version number
unsigned int string_to_version_number( std::string str );
// Return the non-xml string
std::string xml_string_to_string( std::string str );
#ifdef _WIN32
// Return it as UTF-8 string
std::string ucs2_to_utf8( const std::wstring &str );
// Return it as UCS-2 string
std::wstring utf8_to_ucs2( const std::string &str );
#endif

// Return the given time as string
std::string Time_to_String( time_t t, const char *format );

// Return the opposite Direction
ObjectDirection Get_Opposite_Direction( const ObjectDirection direction );
// Return the Direction Name
std::string Get_Direction_Name( const ObjectDirection dir );
// Return the Direction identifier
ObjectDirection Get_Direction_Id( const std::string &str_direction );

// Return the SpriteType identifier
SpriteType Get_Sprite_Type_Id( const std::string &str_type );
/* Return the Color of the given Sprite
 * based mostly on sprite array
*/
Color Get_Sprite_Color( const cSprite *sprite );

// Return the massive type Name
std::string Get_Massive_Type_Name( const MassiveType mtype );
// Return the massive type identifier
MassiveType Get_Massive_Type_Id( const std::string &str_massivetype );
// Return the Color of the given Massivetype
Color Get_Massive_Type_Color( const MassiveType mtype );

// Return the ground type name
std::string Get_Ground_Type_Name( const GroundType gtype );
// Return the ground type identifier
GroundType Get_Ground_Type_Id( const std::string &str_groundtype );

// Return the level land type name
std::string Get_Level_Land_Type_Name( const LevelLandType land_type );
// Return the level land type identifier
LevelLandType Get_Level_Land_Type_Id( const std::string &str_type );

// Return the Color Name
std::string Get_Color_Name( const DefaultColor color );
// Return the Color identifier
DefaultColor Get_Color_Id( const std::string &str_color );

// Return the Difficulty name
std::string Get_Difficulty_Name( Uint8 difficulty );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
