/***************************************************************************
 * level_settings.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2006 - 2010 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_LEVEL_SETTINGS_H
#define SMC_LEVEL_SETTINGS_H

#include "../core/global_basic.h"
#include "../gui/hud.h"

namespace SMC
{

/* *** *** *** *** *** *** *** Level Settings *** *** *** *** *** *** *** *** *** *** */
class cLevel_Settings
{
public:
	cLevel_Settings( cSprite_Manager *sprite_manager, cLevel *level );
	~cLevel_Settings( void );

	// Initializes Text Sprites and Graphics
	void Init( void );
	// Enter
	void Enter( void );
	// Leave (saves settings)
	void Leave( void );
	// Unload
	void Unload( void );

	// Update
	void Update( void );
	// Draw
	void Draw( void );

	/* handle key down event
	 * returns true if the key was processed
	*/
	bool Key_Down( SDLKey key );

	// Set the parent level
	void Set_Level( cLevel *level );
	// Set the parent sprite manager
	void Set_Sprite_Manager( cSprite_Manager *sprite_manager );

	// Adds a Background image
	bool Add_Background_Image( const CEGUI::EventArgs &event );
	// Delete the currently selected Background image
	bool Delete_Background_Image( const CEGUI::EventArgs &event );
	// Set the current Background image
	bool Set_Background_Image( const CEGUI::EventArgs &event );

	// Saves the Settings and exits
	bool Button_Save( const CEGUI::EventArgs &event );

	// Updates the Colors
	bool Update_BG_Colors( const CEGUI::EventArgs &event );
	// Loads the Background image List
	void Load_BG_Image_List( void );

	// Set the Background image preview
	void Set_Background_Image_Preview( std::string filename );
	// Updates the Background image
	bool Update_BG_Image( const CEGUI::EventArgs &event );
	// Clears the background Layer fields
	void Clear_Layer_Field( void );

	// Loads the global effect data
	void Load_Global_Effect( void );
	// Set the global effect image preview
	void Set_Global_Effect_Image_Preview( std::string filename );
	// Updates the global effect image
	bool Update_Global_Effect_Image( const CEGUI::EventArgs &event );

	// true if menu is active
	bool m_active;

	// parent level
	cLevel *m_level;
	// settings camera
	cCamera *m_camera;
	// GUI
	CEGUI::Window *m_gui_window;
	CEGUI::TabControl *m_tabcontrol;

	// background image preview
	cHudSprite *m_background_preview;

	// global effect image preview
	cHudSprite *m_global_effect_preview;

	// Background colors
	Color m_bg_color_1;
	Color m_bg_color_2;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
