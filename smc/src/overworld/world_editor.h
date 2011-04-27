/***************************************************************************
 * world_editor.h
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

#ifndef SMC_WORLD_EDITOR_H
#define SMC_WORLD_EDITOR_H

#include "../core/editor.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cEditor_World *** *** *** *** *** *** *** *** *** *** */

class cEditor_World : public cEditor
{
public:
	cEditor_World( cSprite_Manager *sprite_manager, cOverworld *overworld );
	virtual ~cEditor_World( void );

	// Initialize
	virtual void Init( void );

	// Enable
	virtual void Enable( void );
	/* Disable
	 * native_mode : if unset the current game mode isn't altered
 	*/
	virtual void Disable( bool native_mode = 0 );

	/* handle key down event
	 * returns true if the key was processed
	*/
	virtual bool Key_Down( SDLKey key );

	// Set the parent overworld
	void Set_Overworld( cOverworld *overworld );
	// Set Active Menu Entry
	virtual void Activate_Menu_Item( cEditor_Menu_Object *entry );
	// return the sprite object
	virtual cSprite *Get_Object( const CEGUI::String &element, CEGUI::XMLAttributes &attributes, int engine_version );

	// Menu functions
	virtual bool Function_New( void );
	virtual void Function_Load( void );
	virtual void Function_Save( bool with_dialog = 0 );
	//virtual void Function_Save_as( void );
	virtual void Function_Reload( void );
	//void Function_Settings( void );

	// parent overworld
	cOverworld *m_overworld;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The World Editor
extern cEditor_World *pWorld_Editor;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
