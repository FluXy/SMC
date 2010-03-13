/***************************************************************************
 * level.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2003 - 2009 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_LEVEL_H
#define SMC_LEVEL_H

#include "../core/global_basic.h"
#include "../level/global_effect.h"
#include "../level/level_background.h"
#include "../core/file_parser.h"
#include "../level/level_manager.h"
#include "../objects/level_entry.h"
#include "../audio/random_sound.h"
// CEGUI
#include "CEGUIXMLHandler.h"
#include "CEGUIXMLAttributes.h"

namespace SMC
{

/* *** *** *** *** *** cLevel *** *** *** *** *** *** *** *** *** *** *** *** */

class cLevel : public CEGUI::XMLHandler
{
public:
	cLevel( void );
	virtual ~cLevel( void );

	/* Create a new level
	 * returns true if successful
	*/
	bool New( std::string filename );
	// Load
	bool Load( std::string filename );
	/* Unload the current Level
	 * if delayed is given unloads the on the next update
	*/
	void Unload( bool delayed = 0 );
	// Save the Level
	void Save( void );
	// Delete and Unload Level
	void Delete( void );
	// reset settings data
	void Reset_Settings( void );

	// Init
	void Init( void );
	// Set this sprite manager active
	void Set_Sprite_Manager( void );
	// Enter
	void Enter( const GameMode old_mode = MODE_NOTHING );
	// Leave
	void Leave( const GameMode next_mode = MODE_NOTHING );

	// update level
	void Update( void );
	/* late level update
	 * needed for objects that need other objects to be already updated
	*/
	void Update_Late( void );

	// Draw Layer 1 ( Backgrounds, Level objects and Animations )
	void Draw_Layer_1( LevelDrawType type = LVL_DRAW );
	// Draw Layer 2 ( Global Effects )
	void Draw_Layer_2( LevelDrawType type = LVL_DRAW );

	// Function : Process_Input
	// static input handler
	void Process_Input( void );
	/* handle key down event
	 * returns true processed
	*/
	bool Key_Down( const SDLKey key );
	/* handle key up event
	 * returns true if processed
	*/
	bool Key_Up( const SDLKey key );
	/* handle mouse button down event
	 * returns true if processed
	*/
	bool Mouse_Down( Uint8 button );
	/* handle mouse button up event
	 * returns true if processed
	*/
	bool Mouse_Up( Uint8 button );
	/* handle joystick button down event
	 * returns true if processed
	*/
	bool Joy_Button_Down( Uint8 button );
	/* handle joystick button up event
	 * returns true if processed
	*/
	bool Joy_Button_Up( Uint8 button );

	/* Return the current Music filename with the given options
	 * if with_dir is set to 0 the whole directory is cut
	 * if set to 1 the music directory is cut out
	 * if set to 1 the full directory will be returned
	*/
	std::string Get_Musicfile( int with_dir = 2, bool with_end = 1 ) const;
	// Set the Music filename
	void Set_Musicfile( std::string filename );
	/* Set a new Level filename name and automatically re-save the level
	 * delete_old : if set delete the old level name
	*/
	void Set_Levelfile( std::string filename, bool delete_old = 1 );
	// Set the Level Author
	void Set_Author( const std::string &name );
	// Set the Level Version
	void Set_Version( const std::string &level_version );

	// Get entry with the given name
	cLevel_Entry *Get_Entry( const std::string &name );

	// Return true if a level is loaded
	bool Is_Loaded( void ) const;

	// level filename
	std::string m_level_filename;
	// if a new level should be loaded this is the next level filename
	std::string m_next_level_filename;

	// unload the level on the next update
	bool m_delayed_unload;

	// background manager
	cBackground_Manager *m_background_manager;
	// global effect
	cGlobal_effect *m_global_effect;
	// animation manager
	cAnimation_Manager *m_animation_manager;
	// sprite manager
	cSprite_Manager *m_sprite_manager;

	/* *** *** *** Settings *** *** *** *** */

	// level engine version
	int m_engine_version;
	// last save time
	time_t m_last_saved;
	// level author
	std::string m_author;
	// level version
	std::string m_version;
	// music filename
	std::string m_musicfile;
	// valid music to play
	bool m_valid_music;

	// player
	float m_player_start_pos_x;
	float m_player_start_pos_y;
	ObjectDirection m_player_start_direction;
	// camera 
	GL_rect m_camera_limits;
	float m_fixed_camera_hor_vel;

private:
	// XML element start
	virtual void elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes );
	// XML element end
	virtual void elementEnd( const CEGUI::String &element );

	// XML element Item Tag list
	CEGUI::XMLAttributes m_xml_attributes;
};

/* Return the Level Object if element name is available else NULL
 * engine_version : engine version of the data and if it's below the current version it converts it
 * sprite_manager : needed if the engine version is below the current version and data conversion creates multiple objects
*/
cSprite *Get_Level_Object( const CEGUI::String &xml_element, CEGUI::XMLAttributes &attributes, int engine_version, cSprite_Manager *sprite_manager );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Level
extern cLevel *pActive_Level;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
