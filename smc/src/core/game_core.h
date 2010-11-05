/***************************************************************************
 * game_core.h  -  header for the corresponding cpp file
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

#ifndef SMC_GAME_CORE_H
#define SMC_GAME_CORE_H

#include "../objects/movingsprite.h"
#include "../core/camera.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** Variables *** *** *** *** *** *** *** *** *** */

// quit game if true
extern bool game_exit;
// current Game Mode
extern GameMode Game_Mode;
// current Game Mode Type
extern GameModeType Game_Mode_Type;
// next global Game Action
extern GameAction Game_Action;
// Game Action data
extern CEGUI::XMLAttributes Game_Action_Data;
// Game Action pointer
extern void *Game_Action_ptr;

// internal game resolution and is used for global scaling
extern int game_res_w;
extern int game_res_h;

// global debugging
extern bool game_debug;
extern bool game_debug_performance;

// Game Input event
extern SDL_Event input_event;

// global up scale ( f.e. default image scale )
extern float global_upscalex;
extern float global_upscaley;
// global down scale ( f.e. mouse/CEGUI scale )
extern float global_downscalex;
extern float global_downscaley;

// The global editor enabled variables prevent including additional editor header files
// if editor is enabled for the current game mode
extern bool editor_enabled;
// if level editor is active
extern bool editor_level_enabled;
// if world editor is active
extern bool editor_world_enabled;

// Active camera class
extern cCamera *pActive_Camera;
// Active player
extern cSprite *pActive_Player;


/* *** *** *** *** *** *** *** Functions *** *** *** *** *** *** *** *** *** *** */

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
*/
std::string float_to_string( const float number, int prec = 6 );
// Return the string as a number
int string_to_int( const std::string &str );
Uint64 string_to_int64( const std::string &str );
long string_to_long( const std::string &str );
// Return the string as a float
float string_to_float( const std::string &str );
// Return the string as a double
double string_to_double( const std::string &str );
// Return the real string
std::string xml_string_to_string( std::string str );

// Handle game events
void Handle_Game_Events( void );
// Handle generic game events
void Handle_Generic_Game_Events( const CEGUI::XMLAttributes &Current_Game_Action_Data );
/* Leave the current game mode state
 * mostly to prepare for a new game mode
*/
void Leave_Game_Mode( const GameMode next_mode );

/* Enter the given game mode
 * handles/changes game mode specific managers and objects
*/
void Enter_Game_Mode( const GameMode new_mode );

// Return the given time as string
std::string Time_to_String( time_t t, const char *format );

// Clear the complete input event queue
void Clear_Input_Events( void ); 

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

/* Preload the common images into the image manager
 * draw_gui : if set use the loading screen gui for drawing
 */
void Preload_Images( bool draw_gui = 0 );

/* Preload the common sounds into the sound manager
 * draw_gui : if set use the loading screen gui for drawing
 */
void Preload_Sounds( bool draw_gui = 0 );

// Write a property line to the serializer
void Write_Property( CEGUI::XMLSerializer &stream, const CEGUI::String &name, CEGUI::String val );
inline void Write_Property( CEGUI::XMLSerializer &stream, const CEGUI::String &name, int val )
{
	Write_Property( stream, name, CEGUI::PropertyHelper::intToString( val ) );
};
inline void Write_Property( CEGUI::XMLSerializer &stream, const CEGUI::String &name, unsigned int val )
{
	Write_Property( stream, name, CEGUI::PropertyHelper::uintToString( val ) );
};
inline void Write_Property( CEGUI::XMLSerializer &stream, const CEGUI::String &name, Uint64 val )
{
	Write_Property( stream, name, int64_to_string( val ) );
};
inline void Write_Property( CEGUI::XMLSerializer &stream, const CEGUI::String &name, long val )
{
	Write_Property( stream, name, long_to_string( val ) );
};
inline void Write_Property( CEGUI::XMLSerializer &stream, const CEGUI::String &name, float val )
{
	Write_Property( stream, name, CEGUI::PropertyHelper::floatToString( val ) );
};

// Changes the image path in the given xml attributes to the new one
void Relocate_Image( CEGUI::XMLAttributes &xml_attributes, const std::string &filename_old, const std::string &filename_new, const CEGUI::String &attribute_name = "image" );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
