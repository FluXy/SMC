/***************************************************************************
 * overworld.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2004 - 2010 Florian Richter
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

class cOverworld_Manager : public CEGUI::XMLHandler, public cObject_Manager<cOverworld>
{
public:
	cOverworld_Manager( cSprite_Manager *sprite_manager );
	virtual ~cOverworld_Manager( void );

	/* Create a new world
	 * returns true if successful
	*/
	bool New( const std::string &name );

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

	// Get Overworld pointer
	cOverworld *Get( const std::string &str );
	// Get Overworld pointer from path
	cOverworld *Get_from_Path( const std::string &path );
	// Get Overworld pointer from name
	cOverworld *Get_from_Name( const std::string &name );

	// Return Overworld array number
	int Get_Array_Num( const std::string &path ) const;

	// worlds descriptor filename
	std::string m_worlds_filename;

	// shows additional information
	bool m_debug_mode;
	// draw the layer
	bool m_draw_layer;
	// map scrolling with the arrow keys
	bool m_camera_mode;

	// world camera
	cCamera *m_camera;
private:
	// XML element start
	virtual void elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes );
	// XML element end
	virtual void elementEnd( const CEGUI::String &element );
	// handles a world
	void handle_world( const CEGUI::XMLAttributes &attributes );

	// XML element Property list
	CEGUI::XMLAttributes m_xml_attributes;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Overworld information handler
extern cOverworld_Manager *pOverworld_Manager;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
