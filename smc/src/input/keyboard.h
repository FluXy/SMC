/***************************************************************************
 * keyboard.h
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

#ifndef SMC_KEYBOARD_H
#define SMC_KEYBOARD_H

#include "../core/global_basic.h"
#include "../core/global_game.h"
// SDL
#include "SDL.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** cKeyboard *** *** *** *** *** *** *** *** *** */

class cKeyboard
{
public:
	cKeyboard( void );
	~cKeyboard( void );

	// Reset all keys
	void Reset_Keys( void );

	/* CEGUI Key Up handler
	 * returns true if CEGUI processed the given key up event
	*/
	bool CEGUI_Handle_Key_Up( SDLKey key ) const;

	/* Key Up Handler
	 * returns true if the event was processed
	*/
	bool Key_Up( SDLKey key );

	/* CEGUI Key Down handler
	 * returns true if CEGUI processed the given key down event
	*/
	bool CEGUI_Handle_Key_Down( SDLKey key ) const;

	/* Key Down handler
	 * returns true if the event was processed
	*/
	bool Key_Down( SDLKey key );

	// Is the CTRL key pressed
	inline bool Is_Ctrl_Down( void ) const
	{
		return m_keys[SDLK_RCTRL] || m_keys[SDLK_LCTRL];
	};
	// Is the SHIFT key pressed
	inline bool Is_Shift_Down( void ) const
	{
		return m_keys[SDLK_RSHIFT] || m_keys[SDLK_LSHIFT];
	};
	// Is the ALT key pressed
	inline bool Is_Alt_Down( void ) const
	{
		return m_keys[SDLK_RALT] || m_keys[SDLK_LALT];
	};

	// Translate a SDLKey to the proper CEGUI::Key
	unsigned int SDLKey_to_CEGUIKey( const SDLKey key ) const;

	// Pressed keys
	Uint8 m_keys[SDLK_LAST];
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// global Keyboard pointer
extern cKeyboard *pKeyboard;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
