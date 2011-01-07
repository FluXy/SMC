/***************************************************************************
 * game_core.cpp  -  globally used variables and functions
 *
 * Copyright (C) 2003 - 2010 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/game_core.h"
#include "../audio/audio.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "../input/joystick.h"
#include "../level/level_editor.h"
#include "../level/level_player.h"
#include "../video/renderer.h"
#include "../level/level.h"
#include "../core/sprite_manager.h"
#include "../overworld/overworld.h"
#include "../gui/menu.h"
#include "../core/i18n.h"
#include "../core/filesystem/filesystem.h"
#include "../objects/level_exit.h"
#include "../gui/menu_data.h"
#include "../user/savegame.h"
#include "../overworld/world_editor.h"
// boost filesystem
#include "boost/filesystem/convenience.hpp"
namespace fs = boost::filesystem;
// CEGUI
#include "CEGUIWindowManager.h"
#include "elements/CEGUIProgressBar.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** Variables *** *** *** *** *** *** *** *** *** */

bool game_exit = 0;
GameMode Game_Mode = MODE_NOTHING;
GameModeType Game_Mode_Type = MODE_TYPE_DEFAULT;
GameAction Game_Action = GA_NONE;
CEGUI::XMLAttributes Game_Action_Data_Start;
CEGUI::XMLAttributes Game_Action_Data_Middle;
CEGUI::XMLAttributes Game_Action_Data_End;
void *Game_Action_ptr = NULL;

int game_res_w = 800;
int game_res_h = 600;

bool game_debug = 0;
bool game_debug_performance = 0;

SDL_Event input_event;

float global_upscalex = 1.0f;
float global_upscaley = 1.0f;
float global_downscalex = 1.0f;
float global_downscaley = 1.0f;

bool editor_enabled = 0;
bool editor_level_enabled = 0;
bool editor_world_enabled = 0;

cCamera *pActive_Camera = NULL;
cSprite *pActive_Player = NULL;


/* *** *** *** *** *** *** *** Functions *** *** *** *** *** *** *** *** *** *** */

void string_replace_all( std::string &str, const std::string &search, const std::string &format )
{
	size_t pos = 0;

	while( (pos = str.find(search, pos)) != std::string::npos )
	{
		str.replace( pos, search.length(), format );
		pos += format.length();
	}
}

void cegui_string_replace_all( CEGUI::String &str, const CEGUI::String &search, const CEGUI::String &format )
{
	size_t pos = 0;

	while( (pos = str.find(search, pos)) != CEGUI::String::npos )
	{
		str.replace( pos, search.length(), format );
		pos += format.length();
	}
}

std::string string_trim_from_end( std::string str, const char search )
{
	// find last position from end which is not the given character
	size_t pos = str.find_last_not_of( search );

	// if all match or empty
	if( pos == std::string::npos )
	{
		return std::string();
	}
	else
	{
		return str.substr( 0, pos + 1 );
	}
}

std::string int_to_string( const int number )
{
	std::ostringstream os;
	os << number;
	return os.str();
}

std::string int64_to_string( const Uint64 number )
{
	std::ostringstream os;
	os << number;
	return os.str();
}

std::string long_to_string( const long number )
{
	std::ostringstream os;
	os << number;
	return os.str();
}

// from stringencoders for float_to_string
/**
 * Powers of 10
 * 10^0 to 10^9
 */
static const double pow_of_10[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

// function from stringencoders for float_to_string
static void strreverse(char* begin, char* end)
{
	char aux;
	while (end > begin)
		aux = *end, *end-- = *begin, *begin++ = aux;
}

/* function from stringencoders 3.10.3 with modifications. Copyright (C) 2007 Nick Galbreath -- nickg [at] modp [dot] com
 * BSD License - http://www.opensource.org/licenses/bsd-license.php
 */
std::string float_to_string( double value, int prec /* = 6 */, bool keep_zeros /* = 1 */ )
{
	/* Hacky test for NaN
	 * under -fast-math this won't work, but then you also won't
	 * have correct nan values anyways.  The alternative is
	 * to link with libmath (bad) or hack IEEE double bits (bad)
	 */
	if( !(value == value) )
	{
		return "nan";
	}

	/* if input is larger than thres_max, revert to native */
	const double thres_max = static_cast<double>(0x7FFFFFFF);

	double diff = 0.0;
	char str[64];
	char* wstr = str;

	if(prec < 0)
	{
		prec = 0;
	}
	else if(prec > 6)
	{
		/* precision of >= 7 for float can lead to overflow errors */
		prec = 6;
	}

	/* we'll work in positive values and deal with the
	   negative sign issue later */
	int neg = 0;
	if(value < 0)
	{
		neg = 1;
		value = -value;
	}

	int whole = static_cast<int>(value);
	double tmp = (value - whole) * pow_of_10[prec];
	uint32_t frac = static_cast<uint32_t>(tmp);
	diff = tmp - frac;

	if(diff > 0.5)
	{
		++frac;
		/* handle rollover, e.g.  case 0.99 with prec 1 is 1.0  */
		if(frac >= pow_of_10[prec])
		{
			frac = 0;
			++whole;
		}
	}
	else if(diff == 0.5 && ((frac == 0) || (frac & 1)))
	{
		/* if halfway, round up if odd, OR
		   if last digit is 0.  That last part is strange */
		++frac;
	}

	/* for very large numbers switch back to native for exponentials. */
	/*
		normal printf behavior is to print EVERY whole number digit
		which can be 100s of characters overflowing your buffers == bad
	*/
	if(value > thres_max)
	{
		std::ostringstream temp;
		temp.setf( std::ios_base::fixed );
		temp << value;

		return temp.str();
	}

	if(prec == 0)
	{
		diff = value - whole;
		if(diff > 0.5)
		{
			/* greater than 0.5, round up, e.g. 1.6 -> 2 */
			++whole;
		}
		else if(diff == 0.5 && (whole & 1))
		{
			/* exactly 0.5 and ODD, then round up */
			/* 1.5 -> 2, but 2.5 -> 2 */
			++whole;
		}
	}
	else
	{
		int count = prec;

		if(!keep_zeros)
		{
			if(frac)
			{
				// now do fractional part, as an unsigned number
				// we know it is not 0 but we can have leading zeros, these
				// should be removed
				while(!(frac % 10))
				{
					--count;
					frac /= 10;
				}

				// now do fractional part, as an unsigned number
				do
				{
					--count;
					*wstr++ = (char)(48 + (frac % 10));
				}
				while(frac /= 10);
				// add extra 0s
				while(count-- > 0) *wstr++ = '0';
				// add decimal
				*wstr++ = '.';
			}
		}
		else
		{
			// now do fractional part, as an unsigned number
			do
			{
				--count;
				*wstr++ = (char)(48 + (frac % 10));
			}
			while(frac /= 10);
			// add extra 0s
			while(count-- > 0) *wstr++ = '0';
			// add decimal
			*wstr++ = '.';
		}
	}

	// do whole part
	// Take care of sign
	// Conversion. Number is reversed.
	do *wstr++ = (char)(48 + (whole % 10)); while (whole /= 10);
	if(neg)
	{
		*wstr++ = '-';
	}

	*wstr='\0';
	strreverse(str, wstr - 1);

	return str;
}

// string conversion helper
template <class T> bool from_string( T &t, const std::string &s, std::ios_base &(*f)(std::ios_base&) )
{
	std::istringstream iss( s );
	return !(iss >> f >> t).fail();
}

int string_to_int( const std::string &str )
{
	int num = 0;
	// use helper
	from_string<int>( num, str, std::dec );
	return num;
}

Uint64 string_to_int64( const std::string &str )
{
	Uint64 num = 0;
	// use helper
	from_string<Uint64>( num, str, std::dec );
	return num;
}

long string_to_long( const std::string &str )
{
	long num = 0;
	// use helper
	from_string<long>( num, str, std::dec );
	return num;
}

float string_to_float( const std::string &str )
{
	float num = 0.0f;
	// use helper
	from_string<float>( num, str, std::dec );
	return num;
}

double string_to_double( const std::string &str )
{
	double num = 0.0;
	// use helper
	from_string<double>( num, str, std::dec );
	return num;
}

unsigned int string_to_version_number( std::string str )
{
	if( str.empty() )
	{
		return 0;
	}

	std::string::size_type pos = str.find( '.' );

	// only major version
	if( pos == std::string::npos )
	{
		return string_to_int( str ) * 10000;
	}

	unsigned int ver = 0;

	if( pos != 0 )
	{
		// number before first '.' is major
		ver = string_to_int( str.substr( 0, pos ) ) * 10000;
	}

	str.erase( 0, pos + 1 );
	pos = str.find( '.' );

	if( pos != 0 )
	{
		// number after first '.' is minor
		ver += string_to_int( str.substr( 0, pos ) ) * 100;
	}
	
	if( pos == std::string::npos )
	{
		return ver;
	}
	
	str.erase( 0, pos + 1 );

	if( !str.empty() )
	{
		// number after second '.' is patch
		ver += string_to_int( str );
	}

	return ver;
}

std::string xml_string_to_string( std::string str )
{
	while( 1 )
	{
		std::string::size_type pos = str.find( "<br/>" );

		if( pos == std::string::npos )
		{
			break;
		}

		str.replace( pos, 5, "\n" );
	}

	return str;
}

void Handle_Game_Events( void )
{
	// if game action is set
	while( Game_Action != GA_NONE )
	{
		// get current data
		const GameMode current_game_mode = Game_Mode;
		const GameModeType current_game_mode_type = Game_Mode_Type;
		const GameAction current_game_action = Game_Action;
		const CEGUI::XMLAttributes current_game_action_data_start = Game_Action_Data_Start;
		const CEGUI::XMLAttributes current_game_action_data_middle = Game_Action_Data_Middle;
		const CEGUI::XMLAttributes current_game_action_data_end = Game_Action_Data_End;
		void *current_game_action_ptr = Game_Action_ptr;
		// clear
		Game_Action = GA_NONE;
		Game_Action_Data_Start = CEGUI::XMLAttributes();
		Game_Action_Data_Middle = CEGUI::XMLAttributes();
		Game_Action_Data_End = CEGUI::XMLAttributes();
		Game_Action_ptr = NULL;

		// handle player downgrade
		if( current_game_action == GA_DOWNGRADE_PLAYER )
		{
			Handle_Generic_Game_Events( current_game_action_data_start );
			pLevel_Player->DownGrade_Player( 0, current_game_action_data_middle.getValueAsBool( "downgrade_force" ) );
			Handle_Generic_Game_Events( current_game_action_data_middle );
			Handle_Generic_Game_Events( current_game_action_data_end );
		}
		// activate level exit
		else if( current_game_action == GA_ACTIVATE_LEVEL_EXIT )
		{
			Handle_Generic_Game_Events( current_game_action_data_start );
			cLevel_Exit *level_exit = static_cast<cLevel_Exit *>(current_game_action_ptr);
			level_exit->Activate();
			Handle_Generic_Game_Events( current_game_action_data_middle );
			Handle_Generic_Game_Events( current_game_action_data_end );
		}
		// full events
		else
		{
			GameMode new_mode = MODE_NOTHING;

			if( current_game_action == GA_ENTER_LEVEL )
			{
				new_mode = MODE_LEVEL;
			}
			else if( current_game_action == GA_ENTER_WORLD )
			{
				new_mode = MODE_OVERWORLD;
			}
			else if( current_game_action == GA_ENTER_MENU )
			{
				new_mode = MODE_MENU;
			}
			else if( current_game_action == GA_ENTER_LEVEL_SETTINGS )
			{
				new_mode = MODE_LEVEL_SETTINGS;
			}

			Handle_Generic_Game_Events( current_game_action_data_start );
			Leave_Game_Mode( new_mode );
			Handle_Generic_Game_Events( current_game_action_data_middle );
			Enter_Game_Mode( new_mode );
			Handle_Generic_Game_Events( current_game_action_data_end );
		}

	}
}

void Handle_Generic_Game_Events( const CEGUI::XMLAttributes &action_data )
{
	if( action_data.exists( "music_fadeout" ) )
	{
		pAudio->Fadeout_Music( action_data.getValueAsInteger( "music_fadeout" ) );
	}
	if( action_data.getValueAsBool( "reset_save" ) )
	{
		pLevel_Player->Reset_Save();
	}
	if( action_data.getValueAsBool( "unload_levels" ) )
	{
		pLevel_Manager->Unload();
	}
	if( action_data.getValueAsBool( "unload_menu" ) )
	{
		pMenuCore->Unload();
	}
	if( action_data.exists( "load_menu" ) )
	{
		MenuID menu = static_cast<MenuID>(action_data.getValueAsInteger( "load_menu" ));
		pMenuCore->Load( menu, static_cast<GameMode>(action_data.getValueAsInteger( "menu_exit_back_to" )) );

		if( menu == MENU_START && action_data.exists( "menu_start_current_level" ) )
		{
			cMenu_Start *menu_start = static_cast<cMenu_Start *>(pMenuCore->m_menu_data);
			menu_start->Highlight_Level( action_data.getValueAsString( "menu_start_current_level" ).c_str() );
		}
	}
	// set active world
	if( action_data.exists( "enter_world" ) )
	{
		pOverworld_Manager->Set_Active( action_data.getValueAsString( "enter_world" ).c_str() );
	}
	// set player waypoint
	if( action_data.exists( "world_player_waypoint" ) )
	{
		// get world waypoint
		int waypoint_num = pActive_Overworld->Get_Waypoint_Num( action_data.getValueAsString( "world_player_waypoint" ).c_str() );

		// waypoint available
		if( waypoint_num >= 0 )
		{
			// set the previous waypoints accessible
			pActive_Overworld->Set_Progress( waypoint_num, 0 );
			pOverworld_Player->Set_Waypoint( waypoint_num );
		}
	}
	if( action_data.exists( "new_level" ) )
	{
		std::string str_level = action_data.getValueAsString( "new_level" ).c_str();
		// new level
		cLevel *level = pLevel_Manager->New( str_level );

		if( level )
		{
			pLevel_Manager->Set_Active( level );
			level->Init();
		}
	}
	if( action_data.exists( "load_level" ) )
	{
		std::string str_level = action_data.getValueAsString( "load_level" ).c_str();
		// load the level
		cLevel *level = pLevel_Manager->Load( str_level );

		if( level )
		{
			pLevel_Manager->Set_Active( level );
			level->Init();

			if( action_data.exists( "load_level_entry" ) )
			{
				std::string str_entry = action_data.getValueAsString( "load_level_entry" ).c_str();
				cLevel_Entry *entry = level->Get_Entry( str_entry );

				// set camera position to show the entry
				if( entry )
				{
					// set position
					pLevel_Player->Set_Pos( entry->Get_Player_Pos_X(), entry->Get_Player_Pos_Y() );
					// center camera position
					pActive_Camera->Center();
					// set invisible for warp animation
					pLevel_Player->Set_Active( 0 );
				}
				else
				{
					printf( "Warning : Level entry %s not found\n", str_entry.c_str() );
				}
			}
		}
		// loading failed
		else
		{
			printf( "Error : Level not found %s\n", str_level.c_str() );
			pHud_Debug->Set_Text( _("Loading Level failed : ") + str_level );

			pLevel_Manager->Finish_Level();
		}
	}
	if( action_data.exists( "load_savegame" ) )
	{
		pSavegame->Load_Game( action_data.getValueAsInteger( "load_savegame" ) );
	}
	if( action_data.exists( "play_music" ) )
	{
		pAudio->Play_Music( action_data.getValueAsString( "play_music" ).c_str(), action_data.getValueAsInteger( "music_loops" ), action_data.getValueAsBool( "music_force", 1 ), action_data.getValueAsInteger( "music_fadein" ) );
	}
	if( action_data.exists( "screen_fadeout" ) )
	{
		Draw_Effect_Out( static_cast<Effect_Fadeout>(action_data.getValueAsInteger( "screen_fadeout" )), action_data.getValueAsFloat( "screen_fadeout_speed", 1 ) );
	}
	if( action_data.exists( "screen_fadein" ) )
	{
		Draw_Effect_In( static_cast<Effect_Fadein>(action_data.getValueAsInteger( "screen_fadein" )), action_data.getValueAsFloat( "screen_fadein_speed", 1 ) );
	}
	if( action_data.exists( "activate_level_entry" ) )
	{
		std::string str_entry = action_data.getValueAsString( "activate_level_entry" ).c_str();
		cLevel_Entry *entry = pActive_Level->Get_Entry( str_entry );

		if( entry )
		{
			pLevel_Manager->Goto_Sub_Level( "", entry->m_entry_name, CAMERA_MOVE_NONE );
		}
	}
	if( action_data.getValueAsBool( "activate_editor" ) )
	{
		if( Game_Mode == MODE_LEVEL )
		{
			pLevel_Editor->Enable();
		}
		else if( Game_Mode == MODE_OVERWORLD )
		{
			pWorld_Editor->Enable();
		}
	}
}

void Leave_Game_Mode( const GameMode next_mode )
{
	if( Game_Mode == MODE_OVERWORLD )
	{
		pActive_Overworld->Leave( next_mode );
	}
	else if( Game_Mode == MODE_LEVEL )
	{
		pActive_Level->Leave( next_mode );
	}
	else if( Game_Mode == MODE_MENU )
	{
		pMenuCore->Leave( next_mode );
	}
	else if( Game_Mode == MODE_LEVEL_SETTINGS )
	{
		pLevel_Editor->m_settings_screen->Leave();
	}
}

void Enter_Game_Mode( const GameMode new_mode )
{
	// remember old mode
	GameMode old_mode = Game_Mode;
	// set new mode
	Game_Mode = new_mode;

	// mode gets level
	if( new_mode == MODE_LEVEL )
	{
		pActive_Level->Enter( old_mode );
	}
	// mode gets overworld
	else if( new_mode == MODE_OVERWORLD )
	{
		pActive_Overworld->Enter( old_mode );
	}
	// mode gets menu
	else if( new_mode == MODE_MENU )
	{
		pMenuCore->Enter( old_mode );
	}
	// mode gets settings
	else if( new_mode == MODE_LEVEL_SETTINGS )
	{
		pLevel_Editor->m_settings_screen->Enter();
	}
}

std::string Time_to_String( time_t t, const char *format )
{
	char str_time[60];
	strftime( str_time, 60, format, localtime( &t ) );

	return str_time;
}

void Clear_Input_Events( void )
{
	while( SDL_PollEvent( &input_event ) )
	{
		// todo : keep Windowmanager quit events ?
		// ignore all events
	}

	// Reset keys
	pKeyboard->Reset_Keys();
	pMouseCursor->Reset_Keys();
	pJoystick->Reset_keys();
}

ObjectDirection Get_Opposite_Direction( const ObjectDirection direction )
{
	switch( direction )
	{
		case DIR_UP:			return DIR_DOWN;
		case DIR_DOWN:			return DIR_UP;
		case DIR_LEFT:			return DIR_RIGHT;
		case DIR_RIGHT:			return DIR_LEFT;
		case DIR_HORIZONTAL:	return DIR_VERTICAL;
		case DIR_VERTICAL:		return DIR_HORIZONTAL;
		default:				break;
	}

	return DIR_UNDEFINED;
}

std::string Get_Direction_Name( const ObjectDirection dir )
{
	switch( dir )
	{
		case DIR_UNDEFINED:		return N_("undefined");
		case DIR_LEFT:			return N_("left");
		case DIR_RIGHT:			return N_("right");
		case DIR_UP:			return N_("up");
		case DIR_DOWN:			return N_("down");
		case DIR_TOP_LEFT:		return N_("top_left");
		case DIR_TOP_RIGHT:		return N_("top_right");
		case DIR_BOTTOM_LEFT:	return N_("bottom_left");
		case DIR_BOTTOM_RIGHT:	return N_("bottom_right");
		case DIR_LEFT_TOP:		return N_("left_top");
		case DIR_LEFT_BOTTOM:	return N_("left_bottom");
		case DIR_RIGHT_TOP:		return N_("right_top");
		case DIR_RIGHT_BOTTOM:	return N_("right_bottom");
		case DIR_HORIZONTAL:	return N_("horizontal");
		case DIR_VERTICAL:		return N_("vertical");
		case DIR_ALL:			return N_("all");
		case DIR_FIRST:			return N_("first");
		case DIR_LAST:			return N_("last");
		default:				break;
	}

	return "";
}

ObjectDirection Get_Direction_Id( const std::string &str_direction )
{
	if( str_direction.compare( "undefined" ) == 0 )
	{
		return DIR_UNDEFINED;
	}
	else if( str_direction.compare( "left" ) == 0 )
	{
		return DIR_LEFT;
	}
	else if( str_direction.compare( "right" ) == 0 )
	{
		return DIR_RIGHT;
	}
	else if( str_direction.compare( "up" ) == 0 )
	{
		return DIR_UP;
	}
	else if( str_direction.compare( "down" ) == 0 )
	{
		return DIR_DOWN;
	}
	else if( str_direction.compare( "top_left" ) == 0 )
	{
		return DIR_TOP_LEFT;
	}
	else if( str_direction.compare( "top_right" ) == 0 )
	{
		return DIR_TOP_RIGHT;
	}
	else if( str_direction.compare( "bottom_left" ) == 0 )
	{
		return DIR_BOTTOM_LEFT;
	}
	else if( str_direction.compare( "bottom_right" ) == 0 )
	{
		return DIR_BOTTOM_RIGHT;
	}
	else if( str_direction.compare( "left_top" ) == 0 )
	{
		return DIR_LEFT_TOP;
	}
	else if( str_direction.compare( "left_bottom" ) == 0 )
	{
		return DIR_LEFT_BOTTOM;
	}
	else if( str_direction.compare( "right_top" ) == 0 )
	{
		return DIR_RIGHT_TOP;
	}
	else if( str_direction.compare( "right_bottom" ) == 0 )
	{
		return DIR_RIGHT_BOTTOM;
	}
	else if( str_direction.compare( "horizontal" ) == 0 )
	{
		return DIR_HORIZONTAL;
	}
	else if( str_direction.compare( "vertical" ) == 0 )
	{
		return DIR_VERTICAL;
	}
	else if( str_direction.compare( "all" ) == 0 )
	{
		return DIR_ALL;
	}
	else if( str_direction.compare( "first" ) == 0 )
	{
		return DIR_FIRST;
	}
	else if( str_direction.compare( "last" ) == 0 )
	{
		return DIR_LAST;
	}

	return DIR_UNDEFINED;
}

SpriteType Get_Sprite_Type_Id( const std::string &str_type )
{
	if( str_type.compare( "massive" ) == 0 )
	{
		return TYPE_MASSIVE;
	}
	else if( str_type.compare( "passive" ) == 0 )
	{
		return TYPE_PASSIVE;
	}
	else if( str_type.compare( "front_passive" ) == 0 )
	{
		return TYPE_FRONT_PASSIVE;
	}
	else if( str_type.compare( "halfmassive" ) == 0 )
	{
		return TYPE_HALFMASSIVE;
	}
	else if( str_type.compare( "climbable" ) == 0 )
	{
		return TYPE_CLIMBABLE;
	}
	else
	{
		printf( "Warning : Unknown Sprite Type String %s\n", str_type.c_str() );
	}
	
	return TYPE_UNDEFINED;
}

Color Get_Sprite_Color( const cSprite *sprite )
{
	switch( sprite->m_sprite_array )
	{
		case ARRAY_ENEMY:
		{
			return red;
		}
		case ARRAY_ACTIVE:
		{
			return blue;
		}
		case ARRAY_MASSIVE:
		{
			if( sprite->m_type == TYPE_PLAYER )
			{
				return lila;
			}

			return orange;
		}
		case ARRAY_PASSIVE:
		{
			if( sprite->m_type == TYPE_FRONT_PASSIVE )
			{
				return greenyellow;
			}

			return green;
		}
		case ARRAY_HUD:
		{
			return blackalpha128;
		}
		default:
		{
			break;
		}
	}

	return lightgreyalpha64;
}

std::string Get_Massive_Type_Name( const MassiveType mtype )
{
	switch( mtype )
	{
		case MASS_PASSIVE:		return "passive";
		case MASS_MASSIVE:		return "massive";
		case MASS_HALFMASSIVE:	return "halfmassive";
		case MASS_CLIMBABLE:	return "climbable";
		default:				break;
	}

	return "";
}

MassiveType Get_Massive_Type_Id( const std::string &str_massivetype )
{
	if( str_massivetype.compare( "passive" ) == 0 )
	{
		return MASS_PASSIVE;
	}
	else if( str_massivetype.compare( "massive" ) == 0 )
	{
		return MASS_MASSIVE;
	}
	else if( str_massivetype.compare( "halfmassive" ) == 0 )
	{
		return MASS_HALFMASSIVE;
	}
	else if( str_massivetype.compare( "climbable" ) == 0 )
	{
		return MASS_CLIMBABLE;
	}

	return MASS_PASSIVE;
}

Color Get_Massive_Type_Color( MassiveType mtype )
{
	switch( mtype )
	{
		case MASS_MASSIVE:		return lightred;
		case MASS_HALFMASSIVE:	return orange;
		case MASS_PASSIVE:		return lightgreen;
		case MASS_CLIMBABLE:	return lila;
		default:				break;
	}

	return white;
}

std::string Get_Ground_Type_Name( const GroundType gtype )
{
	switch( gtype )
	{
		case GROUND_NORMAL:		return "normal";
		case GROUND_EARTH:		return "earth";
		case GROUND_ICE:		return "ice";
		case GROUND_SAND:		return "sand";
		case GROUND_STONE:		return "stone";
		case GROUND_PLASTIC:	return "plastic";
		default:				break;
	}

	return "";
}

GroundType Get_Ground_Type_Id( const std::string &str_groundtype )
{
	if( str_groundtype.compare( "normal" ) == 0 )
	{
		return GROUND_NORMAL;
	}
	else if( str_groundtype.compare( "earth" ) == 0 )
	{
		return GROUND_EARTH;
	}
	else if( str_groundtype.compare( "ice" ) == 0 )
	{
		return GROUND_ICE;
	}
	else if( str_groundtype.compare( "sand" ) == 0 )
	{
		return GROUND_SAND;
	}
	else if( str_groundtype.compare( "stone" ) == 0 )
	{
		return GROUND_STONE;
	}
	else if( str_groundtype.compare( "plastic" ) == 0 )
	{
		return GROUND_PLASTIC;
	}

	return GROUND_NORMAL;
}

std::string Get_Level_Land_Type_Name( const LevelLandType land_type )
{
	switch( land_type )
	{
		case LLT_UNDEFINED:		return N_("undefined");
		case LLT_GREEN:			return N_("green");
		case LLT_JUNGLE:		return N_("jungle");
		case LLT_ICE:			return N_("ice");
		case LLT_SNOW:			return N_("snow");
		case LLT_WATER:			return N_("water");
		case LLT_CANDY:			return N_("candy");
		case LLT_DESERT:		return N_("desert");
		case LLT_SAND:			return N_("sand");
		case LLT_CASTLE:		return N_("castle");
		case LLT_UNDERGROUND:	return N_("underground");
		case LLT_CRYSTAL:		return N_("crystal");
		case LLT_GHOST:			return N_("ghost");
		case LLT_MUSHROOM:		return N_("mushroom");
		case LLT_SKY:			return N_("sky");
		case LLT_PLASTIC:		return N_("plastic");
		default:				break;
	}

	return "";
}

LevelLandType Get_Level_Land_Type_Id( const std::string &str_type )
{
	if( str_type.compare( "undefined" ) == 0 )
	{
		return LLT_UNDEFINED;
	}
	else if( str_type.compare( "green" ) == 0 )
	{
		return LLT_GREEN;
	}
	else if( str_type.compare( "jungle" ) == 0 )
	{
		return LLT_JUNGLE;
	}
	else if( str_type.compare( "ice" ) == 0 )
	{
		return LLT_ICE;
	}
	else if( str_type.compare( "snow" ) == 0 )
	{
		return LLT_SNOW;
	}
	else if( str_type.compare( "water" ) == 0 )
	{
		return LLT_WATER;
	}
	else if( str_type.compare( "candy" ) == 0 )
	{
		return LLT_CANDY;
	}
	else if( str_type.compare( "desert" ) == 0 )
	{
		return LLT_DESERT;
	}
	else if( str_type.compare( "sand" ) == 0 )
	{
		return LLT_SAND;
	}
	else if( str_type.compare( "castle" ) == 0 )
	{
		return LLT_CASTLE;
	}
	else if( str_type.compare( "underground" ) == 0 )
	{
		return LLT_UNDERGROUND;
	}
	else if( str_type.compare( "crystal" ) == 0 )
	{
		return LLT_CRYSTAL;
	}
	else if( str_type.compare( "ghost" ) == 0 )
	{
		return LLT_GHOST;
	}
	else if( str_type.compare( "mushroom" ) == 0 )
	{
		return LLT_MUSHROOM;
	}
	else if( str_type.compare( "sky" ) == 0 )
	{
		return LLT_SKY;
	}
	else if( str_type.compare( "plastic" ) == 0 )
	{
		return LLT_PLASTIC;
	}

	return LLT_UNDEFINED;
}

std::string Get_Color_Name( const DefaultColor color )
{
	switch( color )
	{
		case COL_DEFAULT:	return N_("default");
		case COL_WHITE:		return N_("white");
		case COL_BLACK:		return N_("black");
		case COL_RED:		return N_("red");
		case COL_ORANGE:	return N_("orange");
		case COL_YELLOW:	return N_("yellow");
		case COL_GREEN:		return N_("green");
		case COL_BLUE:		return N_("blue");
		case COL_BROWN:		return N_("brown");
		case COL_GREY:		return N_("grey");
		default:			break;
	}

	return "";
}

DefaultColor Get_Color_Id( const std::string &str_color )
{
	if( str_color.compare( "white" ) == 0 )
	{
		return COL_WHITE;
	}
	else if( str_color.compare( "black" ) == 0 )
	{
		return COL_BLACK;
	}
	else if( str_color.compare( "red" ) == 0 )
	{
		return COL_RED;
	}
	else if( str_color.compare( "orange" ) == 0 )
	{
		return COL_ORANGE;
	}
	else if( str_color.compare( "yellow" ) == 0 )
	{
		return COL_YELLOW;
	}
	else if( str_color.compare( "green" ) == 0 )
	{
		return COL_GREEN;
	}
	else if( str_color.compare( "blue" ) == 0 )
	{
		return COL_BLUE;
	}
	else if( str_color.compare( "brown" ) == 0 )
	{
		return COL_BROWN;
	}
	else if( str_color.compare( "grey" ) == 0 )
	{
		return COL_GREY;
	}

	return COL_DEFAULT;
}

std::string Get_Difficulty_Name( Uint8 difficulty )
{
	if( difficulty == 0 )
	{
		return "Undefined";
	}
	else if( difficulty < 10 )
	{
		return "Easy-Peasy";
	}
	else if( difficulty < 20 )
	{
		return "Very Easy";
	}
	else if( difficulty < 30 )
	{
		return "Easy";
	}
	else if( difficulty < 40 )
	{
		return "Easy-Medium";
	}
	else if( difficulty < 50 )
	{
		return "Medium";
	}
	else if( difficulty < 60 )
	{
		return "Medium-Hard";
	}
	else if( difficulty < 70 )
	{
		return "Hard";
	}
	else if( difficulty < 80 )
	{
		return "Very Hard";
	}
	else if( difficulty < 90 )
	{
		return "Extreme";
	}
	else if( difficulty < 95 )
	{
		return "Nightmare";
	}

	return "Ultimate";
}

void Preload_Images( bool draw_gui /* = 0 */ )
{
	// progress bar
	CEGUI::ProgressBar *progress_bar = NULL;

	if( draw_gui )
	{
		// get progress bar
		progress_bar = static_cast<CEGUI::ProgressBar *>(CEGUI::WindowManager::getSingleton().getWindow( "progress_bar" ));
		progress_bar->setProgress( 0 );
		// set loading screen text
		Loading_Screen_Draw_Text( _("Loading Images") );
	}

	// image files
	vector<std::string> image_files;

	// player
	vector<std::string> player_small_images = Get_Directory_Files( DATA_DIR "/" GAME_PIXMAPS_DIR "/maryo/small", ".png", 0, 0 );
	vector<std::string> player_big_images = Get_Directory_Files( DATA_DIR "/" GAME_PIXMAPS_DIR "/maryo/big", ".png", 0, 0 );
	vector<std::string> player_fire_images = Get_Directory_Files( DATA_DIR "/" GAME_PIXMAPS_DIR "/maryo/fire", ".png", 0, 0 );
	vector<std::string> player_ice_images = Get_Directory_Files( DATA_DIR "/" GAME_PIXMAPS_DIR "/maryo/ice", ".png", 0, 0 );
	vector<std::string> player_ghost_images = Get_Directory_Files( DATA_DIR "/" GAME_PIXMAPS_DIR "/maryo/ghost", ".png", 0, 0 );

	image_files.insert( image_files.end(), player_small_images.begin(), player_small_images.end() );
	image_files.insert( image_files.end(), player_big_images.begin(), player_big_images.end() );
	image_files.insert( image_files.end(), player_fire_images.begin(), player_fire_images.end() );
	image_files.insert( image_files.end(), player_ice_images.begin(), player_ice_images.end() );
	image_files.insert( image_files.end(), player_ghost_images.begin(), player_ghost_images.end() );

	// Mushrooms
	image_files.push_back( "game/items/mushroom_red.png" );
	image_files.push_back( "game/items/mushroom_green.png" );
	image_files.push_back( "game/items/mushroom_blue.png" );
	image_files.push_back( "game/items/mushroom_ghost.png" );
	// Fireplant
	image_files.push_back( "game/items/fireplant.png" );
	image_files.push_back( "game/items/fireplant_left.png" );
	image_files.push_back( "game/items/fireplant_right.png" );
	// Star
	image_files.push_back( "game/items/star.png" );
	// Feather
	//image_files.push_back( "game/items/feather_1.png" );
	// Yellow Goldpiece
	image_files.push_back( "game/items/goldpiece/yellow/1.png" );
	image_files.push_back( "game/items/goldpiece/yellow/2.png" );
	image_files.push_back( "game/items/goldpiece/yellow/3.png" );
	image_files.push_back( "game/items/goldpiece/yellow/4.png" );
	image_files.push_back( "game/items/goldpiece/yellow/5.png" );
	image_files.push_back( "game/items/goldpiece/yellow/6.png" );
	image_files.push_back( "game/items/goldpiece/yellow/7.png" );
	image_files.push_back( "game/items/goldpiece/yellow/8.png" );
	image_files.push_back( "game/items/goldpiece/yellow/9.png" );
	image_files.push_back( "game/items/goldpiece/yellow/10.png" );
	image_files.push_back( "game/items/goldpiece/yellow/1_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/2_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/3_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/4_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/5_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/6_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/7_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/8_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/9_falling.png" );
	image_files.push_back( "game/items/goldpiece/yellow/10_falling.png" );
	// Red Goldpiece
	image_files.push_back( "game/items/goldpiece/red/1.png" );
	image_files.push_back( "game/items/goldpiece/red/2.png" );
	image_files.push_back( "game/items/goldpiece/red/3.png" );
	image_files.push_back( "game/items/goldpiece/red/4.png" );
	image_files.push_back( "game/items/goldpiece/red/5.png" );
	image_files.push_back( "game/items/goldpiece/red/6.png" );
	image_files.push_back( "game/items/goldpiece/red/7.png" );
	image_files.push_back( "game/items/goldpiece/red/8.png" );
	image_files.push_back( "game/items/goldpiece/red/9.png" );
	image_files.push_back( "game/items/goldpiece/red/10.png" );
	image_files.push_back( "game/items/goldpiece/red/1_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/2_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/3_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/4_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/5_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/6_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/7_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/8_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/9_falling.png" );
	image_files.push_back( "game/items/goldpiece/red/10_falling.png" );

	// Brown Box
	image_files.push_back( "game/box/brown1_1.png" );	

	// Light animation
	image_files.push_back( "animation/light_1/1.png" );
	image_files.push_back( "animation/light_1/2.png" );
	image_files.push_back( "animation/light_1/3.png" );
	// Particle animations
	image_files.push_back( "animation/particles/fire_1.png" );
	image_files.push_back( "animation/particles/fire_2.png" );
	image_files.push_back( "animation/particles/fire_3.png" );
	image_files.push_back( "animation/particles/fire_4.png" );
	image_files.push_back( "animation/particles/smoke.png" );
	image_files.push_back( "animation/particles/smoke_black.png" );
	image_files.push_back( "animation/particles/light.png" );
	image_files.push_back( "animation/particles/dirt.png" );
	image_files.push_back( "animation/particles/ice_1.png" );
	image_files.push_back( "animation/particles/cloud.png" );
	image_files.push_back( "animation/particles/axis.png" );

	// Ball
	image_files.push_back( "animation/fireball/1.png" );
	image_files.push_back( "animation/iceball/1.png" );

	// HUD
	image_files.push_back( "game/maryo_l.png" );
	image_files.push_back( "game/gold_m.png" );
	image_files.push_back( "game/itembox.png" );

	unsigned int loaded_files = 0;
	unsigned int file_count = image_files.size();

	// load images
	for( vector<std::string>::iterator itr = image_files.begin(); itr != image_files.end(); ++itr )
	{
		// get filename
		std::string filename = (*itr);

		// preload image
		pVideo->Get_Surface( filename );

		// count files
		loaded_files++;

		if( draw_gui )
		{
			// update progress
			progress_bar->setProgress( static_cast<float>(loaded_files) / static_cast<float>(file_count) );

			Loading_Screen_Draw();
		}
	}
}

void Preload_Sounds( bool draw_gui /* = 0 */ )
{
	// skip caching if disabled
	if( !pAudio->m_sound_enabled )
	{
		return;
	}

	// progress bar
	CEGUI::ProgressBar *progress_bar = NULL;

	if( draw_gui )
	{
		// get progress bar
		progress_bar = static_cast<CEGUI::ProgressBar *>(CEGUI::WindowManager::getSingleton().getWindow( "progress_bar" ));
		progress_bar->setProgress( 0 );
		// set loading screen text
		Loading_Screen_Draw_Text( _("Loading Sounds") );
	}

	// sound files
	vector<std::string> sound_files;

	// player
	sound_files.push_back( "wall_hit.wav" );
	sound_files.push_back( "player/dead.ogg" );
	sound_files.push_back( "itembox_get.ogg" );
	sound_files.push_back( "itembox_set.ogg" );
	sound_files.push_back( "player/pickup_item.wav" );
	sound_files.push_back( "player/jump_small.ogg" );
	sound_files.push_back( "player/jump_small_power.ogg" );
	sound_files.push_back( "player/jump_big.ogg" );
	sound_files.push_back( "player/jump_big_power.ogg" );
	sound_files.push_back( "player/jump_ghost.ogg" );
	// todo : create again
	//sound_files.push_back( "player/maryo_au.ogg" );
	sound_files.push_back( "player/powerdown.ogg" );
	sound_files.push_back( "player/ghost_end.ogg" );
	sound_files.push_back( "player/run_stop.ogg" );
	sound_files.push_back( "enter_pipe.ogg" );
	sound_files.push_back( "leave_pipe.ogg" );

	// items
	sound_files.push_back( "item/star_kill.ogg" );
	sound_files.push_back( "item/fireball.ogg" );
	sound_files.push_back( "item/iceball.wav" );
	sound_files.push_back( "item/ice_kill.wav" );
	sound_files.push_back( "item/fireball_explode.wav" );
	sound_files.push_back( "item/fireball_repelled.wav" );
	sound_files.push_back( "item/fireball_explosion.wav" );
	sound_files.push_back( "item/iceball_explosion.wav" );
	sound_files.push_back( "item/fireplant.ogg" );
	sound_files.push_back( "item/goldpiece_1.ogg" );
	sound_files.push_back( "item/goldpiece_red.wav" );
	sound_files.push_back( "item/live_up.ogg" );
	sound_files.push_back( "item/live_up_2.ogg" );
	sound_files.push_back( "item/mushroom.ogg" );
	sound_files.push_back( "item/mushroom_ghost.ogg" );
	sound_files.push_back( "item/mushroom_blue.wav" );
	sound_files.push_back( "item/moon.ogg" );

	// box
	sound_files.push_back( "item/empty_box.wav" );

	// enemies
	// eato
	sound_files.push_back( "enemy/eato/die.ogg" );
	// gee
	sound_files.push_back( "enemy/gee/die.ogg" );
	// furball
	sound_files.push_back( "enemy/furball/die.ogg" );
	// furball boss
	sound_files.push_back( "enemy/boss/furball/hit.wav" );
	sound_files.push_back( "enemy/boss/furball/hit_failed.wav" );
	// flyon
	sound_files.push_back( "enemy/flyon/die.ogg" );
	// krush
	sound_files.push_back( "enemy/krush/die.ogg" );
	// rokko
	sound_files.push_back( "enemy/rokko/activate.wav" );
	sound_files.push_back( "enemy/rokko/hit.wav" );
	// spika
	sound_files.push_back( "enemy/spika/move.ogg" );
	// thromp
	sound_files.push_back( "enemy/thromp/hit.ogg" );
	sound_files.push_back( "enemy/thromp/die.ogg" );
	// turtle
	sound_files.push_back( "enemy/turtle/hit.ogg" );
	sound_files.push_back( "enemy/turtle/shell/hit.ogg" );
	sound_files.push_back( "enemy/turtle/stand_up.wav" );
	// turtle boss
	sound_files.push_back( "enemy/boss/turtle/big_hit.ogg" );
	sound_files.push_back( "enemy/boss/turtle/shell_attack.ogg" );
	sound_files.push_back( "enemy/boss/turtle/power_up.ogg" );

	// default
	sound_files.push_back( "sprout_1.ogg" );
	sound_files.push_back( "stomp_1.ogg" );
	sound_files.push_back( "stomp_4.ogg" );

	// savegame
	sound_files.push_back( "savegame_load.ogg" );
	sound_files.push_back( "savegame_save.ogg" );

	// overworld
	sound_files.push_back( "waypoint_reached.ogg" );

	unsigned int loaded_files = 0;
	unsigned int file_count = sound_files.size();

	// load images
	for( vector<std::string>::iterator itr = sound_files.begin(); itr != sound_files.end(); ++itr )
	{
		// get filename
		std::string filename = (*itr);

		// preload it
		pAudio->Get_Sound_File( filename );

		// count files
		loaded_files++;

		if( draw_gui )
		{
			// update progress
			progress_bar->setProgress( static_cast<float>(loaded_files) / static_cast<float>(file_count) );

			Loading_Screen_Draw();
		}
	}
}

void Write_Property( CEGUI::XMLSerializer &stream, const CEGUI::String &name, CEGUI::String val )
{
	// CEGUI doesn't handle line breaks
	cegui_string_replace_all( val, "\n", "<br/>" );

	stream.openTag( "property" )
		.attribute( "name", name )
		.attribute( "value", val )
		.closeTag();
}

void Relocate_Image( CEGUI::XMLAttributes &xml_attributes, const std::string &filename_old, const std::string &filename_new, const CEGUI::String &attribute_name /* = "image" */ )
{
	if( xml_attributes.getValueAsString( attribute_name ).compare( filename_old ) == 0 || xml_attributes.getValueAsString( attribute_name ).compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" + filename_old ) == 0 )
	{
		xml_attributes.remove( attribute_name );
		xml_attributes.add( attribute_name, filename_new );
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
