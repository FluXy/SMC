/***************************************************************************
 * overworld.h
 *
 * Copyright (C) 2004 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_WORLD_MANAGER_H
#define SMC_WORLD_MANAGER_H

#include "../core/global_basic.h"
#include "../core/obj_manager.h"
#include "../core/camera.h"
// CEGUI
#include "CEGUIXMLHandler.h"
#include "CEGUIXMLAttributes.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** cOverworld_Manager *** *** *** *** *** *** *** *** *** */

class cOverworld;

class cOverworld_Manager : public cObject_Manager<cOverworld>
{
public:
	cOverworld_Manager( cSprite_Manager *sprite_manager );
	virtual ~cOverworld_Manager( void );

	/* Create a new overworld
	 * returns true if successful
	*/
	bool New( std::string name );

	// Load all overworlds
	// todo : should only load overworld info
	void Init( void );
	/* Load overworlds from the given directory
	 * user_dir : if set overrides game worlds
	*/
	void Load_Dir( const std::string &dir, bool user_dir = 0 );

	// Set active Overworld from name or path
	bool Set_Active( const std::string &str );
	// Set active Overworld
	bool Set_Active( cOverworld *world );

	// Reset to default world first Waypoint
	void Reset( void );

	// Get overworld pointer
	cOverworld *Get( const std::string &str );
	// Get overworld from path
	cOverworld *Get_from_Path( const std::string &path );
	// Get overworld from name
	cOverworld *Get_from_Name( const std::string &name );

	// Return overworld array number
	int Get_Array_Num( const std::string &path ) const;

	// shows additional information
	bool m_debug_mode;
	// draw the layer
	bool m_draw_layer;
	// map scrolling with the arrow keys
	bool m_camera_mode;

	// world camera
	cCamera *m_camera;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Overworld Manager
extern cOverworld_Manager *pOverworld_Manager;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
