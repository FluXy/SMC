/***************************************************************************
 * level_entry.h
 *
 * Copyright (C) 2007 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_LEVEL_ENTRY_H
#define SMC_LEVEL_ENTRY_H

#include "../core/global_basic.h"
#include "../objects/animated_sprite.h"

namespace SMC
{

/* *** *** *** *** *** *** *** Level Entry types *** *** *** *** *** *** *** *** *** *** */

enum Level_Entry_type
{
	LEVEL_ENTRY_BEAM		= 0,	// no animation ( f.e. a door or hole )
	LEVEL_ENTRY_WARP		= 1		// rotated player moves slowly into the destination direction
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Level Entry
 * 
*/
class cLevel_Entry : public cAnimated_Sprite
{
public:
	// constructor
	cLevel_Entry( cSprite_Manager *sprite_manager );
	// create from stream
	cLevel_Entry( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager );
	// destructor
	virtual ~cLevel_Entry( void );

	// init defaults
	void Init( void );
	// copy this sprite
	virtual cLevel_Entry *Copy( void ) const;

	// load from stream
	virtual void Load_From_XML( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_XML( CEGUI::XMLSerializer &stream );
	// Set direction
	void Set_Direction( const ObjectDirection dir );

	// draw
	virtual void Draw( cSurface_Request *request = NULL );

	// Get centered player position y
	float Get_Player_Pos_X( void ) const;
	// Get centered player position y
	float Get_Player_Pos_Y( void ) const;

	// Activate
	void Activate( void );

	// Set the type
	void Set_Type( Level_Entry_type new_type );
	// Set the name
	void Set_Name( const std::string &str_name );

	// if draw is valid for the current state and position
	virtual bool Is_Draw_Valid( void );

	// editor activation
	virtual void Editor_Activate( void );
	// editor direction option selected event
	bool Editor_Direction_Select( const CEGUI::EventArgs &event );
	// editor name text changed event
	bool Editor_Name_Text_Changed( const CEGUI::EventArgs &event );

	// level entry type
	Level_Entry_type m_entry_type;
	// identification name
	std::string m_entry_name;

	// editor type color
	Color m_editor_color;
	// editor entry name text
	cGL_Surface *m_editor_entry_name;

private:
	void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
