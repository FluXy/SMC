/***************************************************************************
 * waypoint.h
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

#ifndef SMC_WORLD_WAYPOINT_H
#define SMC_WORLD_WAYPOINT_H

#include "../core/global_basic.h"
#include "../video/video.h"
#include "../objects/movingsprite.h"

namespace SMC
{

/* *** *** *** *** *** Waypoint types *** *** *** *** *** *** *** *** *** *** *** *** */

enum Waypoint_type
{
	WAYPOINT_NORMAL = 1,
	WAYPOINT_WORLD_LINK = 2 // Enters another World
};

/* *** *** *** *** *** *** cWaypoint *** *** *** *** *** *** *** *** *** *** *** */

class cWaypoint : public cSprite
{
public:
	// constructor
	cWaypoint( cSprite_Manager *sprite_manager );
	// create from stream
	cWaypoint( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager );
	// destructor
	virtual ~cWaypoint( void );
	
	// Init defaults
	void Init( void );

	// copy this object
	virtual cWaypoint *Copy( void ) const;

	// load from stream
	virtual void Load_From_XML( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_XML( CEGUI::XMLSerializer &stream );

	// Update
	virtual void Update( void );
	// Draw
	virtual void Draw( cSurface_Request *request = NULL );

	// Set direction forward
	void Set_Direction_Forward( ObjectDirection direction );
	// Set direction backward
	void Set_Direction_Backward( ObjectDirection direction );
	// Set Access
	void Set_Access( bool enabled, bool new_start_access = 0 );

	// Set the Destination
	void Set_Destination( std::string str );
	// Returns the Destination
	std::string Get_Destination( bool with_dir = 0, bool with_end = 0 ) const;

	// editor activation
	virtual void Editor_Activate( void );
	// editor type option selected event
	bool Editor_Type_Select( const CEGUI::EventArgs &event );
	// editor destination text changed event
	bool Editor_Destination_Text_Changed( const CEGUI::EventArgs &event );
	// editor access option selected event
	bool Editor_Access_Select( const CEGUI::EventArgs &event );
	// editor direction backward option selected event
	bool Editor_Backward_Direction_Select( const CEGUI::EventArgs &event );
	// editor direction forward option selected event
	bool Editor_Forward_Direction_Select( const CEGUI::EventArgs &event );

	// forward direction
	ObjectDirection m_direction_forward;
	// backward direction
	ObjectDirection m_direction_backward;

	/* The Waypoint type
	 * see the definitions
	*/
	Waypoint_type m_waypoint_type;
	// destination
	std::string m_destination;

	// if this waypoint is accessible
	bool m_access;
	// the default access defined in the definition
	bool m_access_default;

	// color for the glim effect
	float m_glim_color;
	// glim effect type switch
	bool m_glim_mod;

	// arrow forward
	cGL_Surface *m_arrow_forward;
	// arrow backward
	cGL_Surface *m_arrow_backward;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
