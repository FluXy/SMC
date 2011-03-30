/***************************************************************************
 * overworld.h
 *
 * Copyright (C) 2003 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_OVERWORLD_H
#define SMC_OVERWORLD_H

#include "../overworld/world_manager.h"
#include "../overworld/world_waypoint.h"
#include "../overworld/world_layer.h"
#include "../overworld/world_player.h"
#include "../overworld/world_sprite_manager.h"
#include "../gui/hud.h"
#include "../audio/random_sound.h"
// CEGUI
#include "CEGUIXMLHandler.h"
#include "CEGUIXMLAttributes.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** cOverworld_description *** *** *** *** *** *** *** *** *** */

class cOverworld_description : public CEGUI::XMLHandler
{
public:
	cOverworld_description( void );
	virtual ~cOverworld_description( void );

	// Load
	void Load( void );

	// Save
	void Save( void );

	// returns the full path
	std::string Get_Full_Path( void ) const;

	// directory name
	std::string m_path;
	// world name
	std::string m_name;
	// is visible on selection
	bool m_visible;
	/* 0 if only in game directory
	 * 1 if only in user directory
	 * 2 if in both
	*/
	int m_user;

	// world comment
	std::string m_comment;
private:
	// XML element start
	virtual void elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes );
	// XML element end
	virtual void elementEnd( const CEGUI::String &element );
	// handle world description
	void handle_world( const CEGUI::XMLAttributes& attributes );

	// XML element property list
	CEGUI::XMLAttributes m_xml_attributes;
};

/* *** *** *** *** *** *** *** *** cOverworld *** *** *** *** *** *** *** *** *** */

// forward declare
class cAnimation_Manager;

typedef vector<cWaypoint *> WaypointList;

class cOverworld : public CEGUI::XMLHandler
{
public:
	cOverworld( void );
	virtual ~cOverworld( void );

	// New
	bool New( std::string name );
	// Load
	bool Load( void );
	// Unload
	void Unload( void );
	// Save
	void Save( void );
	// Enter
	void Enter( const GameMode old_mode = MODE_NOTHING );
	// Leave
	void Leave( const GameMode next_mode = MODE_NOTHING );

	// Draw
	void Draw( void );
	// Draw Layer 1 ( Backgrounds, World objects and Animations )
	void Draw_Layer_1( void );
	// Draws the HUD
	void Draw_HUD( void );

	// Update
	void Update( void );
	// Updates the Camera
	void Update_Camera( void );

	/* handle key down event
	 * returns true if processed
	*/
	bool Key_Down( SDLKey key );
	/* handle key up event
	 * returns true if processed
	*/
	bool Key_Up( SDLKey key );
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

	/* Sets the current Waypoint progress
	 * if force is set already accessible waypoints will be unset
	*/
	void Set_Progress( unsigned int normal_level, bool force = 1 );

	/* Returns a pointer to the Waypoint
	 * if not found returns NULL
	*/
	cWaypoint *Get_Waypoint( const std::string &name );
	cWaypoint *Get_Waypoint( unsigned int num );
	/* Returns the Waypoint array number if the level name matches
	 * if not found returns -1
	*/
	int Get_Level_Waypoint_Num( std::string level_name );
	/* Returns the Waypoint array number if the destination name matches
	 * if not found returns -1
	*/
	int Get_Waypoint_Num( const std::string &world_name );

	/* Check if the rect collides with a Waypoint
	 * if no collision found returns -1
	*/
	int Get_Waypoint_Collision( const GL_rect &rect_2 );
	// returns the last accessible Waypoint
	int Get_Last_Valid_Waypoint( void );
	// update the Waypoint text
	void Update_Waypoint_text( void );

	// Enable the next Level and walk into the forward direction
	bool Goto_Next_Level( void );
	// Resets the Waypoint access to the default
	void Reset_Waypoints( void );

	// Return true if a world is loaded
	bool Is_Loaded( void ) const;

	// map objects
	cWorld_Sprite_Manager *m_sprite_manager;
	// animation manager
	cAnimation_Manager *m_animation_manager;
	// waypoints
	WaypointList m_waypoints;
	// description
	cOverworld_description *m_description;
	// current Layer for collision checking
	cLayer *m_layer;

	/* *** *** *** Settings *** *** *** *** */

	// world engine version
	int m_engine_version;
	// last save time
	time_t m_last_saved;

	// background color
	Color m_background_color;
	// music filename
	std::string m_musicfile;

	// settings
	unsigned int m_player_start_waypoint;
	Moving_state m_player_moving_state;

	/* *** *** ***  *** *** *** *** */

	// goto next level on overworld enter
	bool m_next_level;

	// HUD world name
	cHudSprite *m_hud_world_name;
	// HUD current level name
	cHudSprite *m_hud_level_name;

private:
	// XML element start
	virtual void elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes );
	// XML element end
	virtual void elementEnd( const CEGUI::String &element );
 
	// XML element property list
	CEGUI::XMLAttributes m_xml_attributes;
};

/* Returns a World Object if element name is available else NULL
 * engine_version : engine version of the data and if it's below the current version it converts it
 * sprite_manager : needed if the engine version is below the current version and data conversion creates multiple objects
 * overworld : the parent overworld
*/
cSprite *Create_World_Object_From_XML( const CEGUI::String &element, CEGUI::XMLAttributes &attributes, int engine_version, cSprite_Manager *sprite_manager, cOverworld *overworld );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Overworld
extern cOverworld *pActive_Overworld;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
