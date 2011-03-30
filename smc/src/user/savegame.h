/***************************************************************************
 * savegame.h
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

#ifndef SMC_SAVEGAME_H
#define SMC_SAVEGAME_H

#include "../objects/sprite.h"
// CEGUI
#include "CEGUIXMLHandler.h"
#include "CEGUIXMLAttributes.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define SAVEGAME_VERSION 12
#define SAVEGAME_VERSION_UNSUPPORTED 5

/* *** *** *** *** *** *** *** cSave_Overworld_Waypoint *** *** *** *** *** *** *** *** *** *** */
// Overworld Waypoint save data
class cSave_Overworld_Waypoint
{
public:
	cSave_Overworld_Waypoint( void );
	~cSave_Overworld_Waypoint( void );

	std::string m_destination;
	bool m_access;
};

typedef vector<cSave_Overworld_Waypoint *> Save_Overworld_WaypointList;


/* *** *** *** *** *** *** *** cSave_Overworld *** *** *** *** *** *** *** *** *** *** */
// Overworld save data
class cSave_Overworld
{
public:
	cSave_Overworld( void );
	~cSave_Overworld( void );

	std::string m_name;

	// waypoint data
	Save_Overworld_WaypointList m_waypoints;
};

typedef vector<cSave_Overworld *> Save_OverworldList;

/* *** *** *** *** *** *** *** cSave_Level_Object_Property *** *** *** *** *** *** *** *** *** *** */
// Level object string property
class cSave_Level_Object_Property
{
public:
	cSave_Level_Object_Property( const std::string &new_name = "", const std::string &new_value = "" );

	std::string m_name;
	std::string m_value;
};

typedef vector<cSave_Level_Object_Property> Save_Level_Object_ProprtyList;

/* *** *** *** *** *** *** *** cSave_Level_Object *** *** *** *** *** *** *** *** *** *** */
// Level object save data
class cSave_Level_Object
{
public:
	cSave_Level_Object( void );
	~cSave_Level_Object( void );

	// Check if property exists
	bool exists( const std::string &val_name );

	// Returns the value
	std::string Get_Value( const std::string &val_name );

	SpriteType m_type;

	// object properties
	Save_Level_Object_ProprtyList m_properties;
};

typedef vector<cSave_Level_Object *> Save_Level_ObjectList;

/* *** *** *** *** *** *** *** cSave_Level *** *** *** *** *** *** *** *** *** *** */
// Level save data
class cSave_Level
{
public:
	cSave_Level( void );
	~cSave_Level( void );

	std::string m_name;
	// player position is only set if level is the active one
	float m_level_pos_x;
	float m_level_pos_y;

	// objects data
	Save_Level_ObjectList m_level_objects;
	// spawned objects
	cSprite_List m_spawned_objects;
};

typedef vector<cSave_Level *> Save_LevelList;

/* *** *** *** *** *** *** *** cSave *** *** *** *** *** *** *** *** *** *** */
class cSave
{
public:
	cSave( void );
	~cSave( void );

	// Initialize data to empty values
	void Init( void );

	// return the active level if available
	std::string Get_Active_Level( void );

	// savegame version
	int m_version;
	// time ( seconds since 1970 )
	time_t m_save_time;
	// description
	std::string m_description;

	// lives
	unsigned int m_lives;
	// points
	long m_points;
	// goldpieces
	unsigned int m_goldpieces;
	// player type
	unsigned int m_player_type;
	// player moving state
	unsigned int m_player_state;
	// item in the itembox
	unsigned int m_itembox_item;

	// level data
	Save_LevelList m_levels;
	// level time
	Uint32 m_level_time;
	// level engine version
	int m_level_engine_version;

	/* active overworld
	 * if not set game mode is custom level
	*/
	std::string m_overworld_active;
	// current waypoint
	unsigned int m_overworld_current_waypoint;

	// overworld data
	Save_OverworldList m_overworlds;
};

/* *** *** *** *** *** *** *** cSavegame *** *** *** *** *** *** *** *** *** *** */

class cSavegame
{
public:
	cSavegame( void );
	virtual ~cSavegame( void );

	/* Load a save
	* Previous progress should be reset before.
	* Returns:
	* 0 if failed
	* 1 if level save
	* 2 if overworld save
	*/
	int Load_Game( unsigned int save_slot );
	// Save the game with the given description
	bool Save_Game( unsigned int save_slot, std::string description );

	/* Load a Save
	* The returned object should be deleted if not used anymore
	*/
	cSave *Load( unsigned int save_slot );
	// Save a Save
	int Save( unsigned int save_slot, cSave *savegame );

	// Returns only the Savegame description
	std::string Get_Description( unsigned int save_slot, bool only_description = 0 );

	// Returns true if the Savegame is valid
	bool Is_Valid( unsigned int save_slot ) const;

	// savegame directory
	std::string m_savegame_dir;
};

/* *** *** *** *** *** *** *** cSavegame_XML_Handler *** *** *** *** *** *** *** *** *** *** */

class cSavegame_XML_Handler : public CEGUI::XMLHandler
{
public:
	cSavegame_XML_Handler( const std::string &filename );
	virtual ~cSavegame_XML_Handler( void );

	/* Returns the savegame data
	 * The returned savegame should be deleted if not used anymore
	*/
	cSave *Acquire_Savegame( void );

	// XML element start
	virtual void elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes );
	// XML element end
	virtual void elementEnd( const CEGUI::String &element );

	void Handle_Level( const CEGUI::XMLAttributes &attributes );
	void Handle_Level_Object( const CEGUI::XMLAttributes &attributes );
	void Handle_Level_Spawned_Object( const CEGUI::String &element, CEGUI::XMLAttributes &attributes );
	void Handle_Player( const CEGUI::XMLAttributes &attributes );
	void Handle_Overworld_Data( const CEGUI::XMLAttributes &attributes );
	void Handle_Overworld( const CEGUI::XMLAttributes &attributes );
	void Handle_Overworld_Waypoint( const CEGUI::XMLAttributes &attributes );

	// if old format savegame
	bool m_old_format;

	// XML attributes list
	CEGUI::XMLAttributes m_xml_attributes;

	// overworld waypoints for parsing
	Save_Overworld_WaypointList m_active_waypoints;
	// level objects for parsing
	Save_Level_ObjectList m_level_objects;
	// level spawned objects for parsing
	cSprite_List m_level_spawned_objects;

	// object we are constructing
	cSave *m_savegame;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Savegame Handler
extern cSavegame *pSavegame;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
