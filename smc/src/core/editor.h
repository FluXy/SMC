/***************************************************************************
 * editor.h  -  header for the corresponding cpp file
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

#ifndef SMC_EDITOR_H
#define SMC_EDITOR_H

#include "../core/global_basic.h"
#include "../objects/sprite.h"
#include "../gui/hud.h"
#include "../video/img_settings.h"
// CEGUI
#include "CEGUIXMLHandler.h"
#include "CEGUIXMLAttributes.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cEditor_Object_Settings_Item *** *** *** *** *** *** *** *** *** *** */

class cEditor_Object_Settings_Item
{
public:
	cEditor_Object_Settings_Item( void );
	~cEditor_Object_Settings_Item( void );

	// name
	CEGUI::Window *window_name;
	// settings
	CEGUI::Window *window_setting;
	// if set start new row
	bool advance_row;
};

/* *** *** *** *** *** *** *** *** cEditor_Item_Object *** *** *** *** *** *** *** *** *** */

class cEditor_Item_Object : public CEGUI::ListboxItem
{
public:
	cEditor_Item_Object( const std::string &text );
	virtual ~cEditor_Item_Object( void );

	// Initialize
	void Init( void );

	/*!
	\brief
		Return the rendered pixel size of this list box item.
	\return
		Size object describing the size of the list box item in pixels.
	*/
	virtual	CEGUI::Size getPixelSize( void ) const;
	// draw
    void draw(CEGUI::GeometryBuffer& buffer, const CEGUI::Rect& targetRect, float alpha, const CEGUI::Rect* clipper) const;
	// draw image
	void Draw_Image( void );

	// text
	CEGUI::ListboxTextItem *list_text;
	// sprite
	cSprite *sprite_obj;
	// preview image scale
	float preview_scale;
};

/* *** *** *** *** *** *** *** *** cEditor_Menu_Object *** *** *** *** *** *** *** *** *** */

class cEditor_Menu_Object : public CEGUI::ListboxTextItem
{
public:
	cEditor_Menu_Object( const std::string &text );
	virtual ~cEditor_Menu_Object( void );

	// Initialize
	void Init( void );

	// name
	std::string name;
	// tags or function name if function
	std::string tags;

	// if type is a function
	bool bfunction;
	// if this is a header
	bool header;
};

/* *** *** *** *** *** *** *** cEditor *** *** *** *** *** *** *** *** *** *** */

class cEditor : public CEGUI::XMLHandler
{
public:
	cEditor( cSprite_Manager *sprite_manager );
	virtual ~cEditor( void );

	// Initialize Editor
	virtual void Init( void );
	// Unload Editor
	virtual void Unload( void );

	// Toggle
	void Toggle( void );
	// Enable
	virtual void Enable( void );
	/* Disable
	 * native_mode : if unset the current game mode isn't altered
 	*/
	virtual void Disable( bool native_mode = 1 );

	// Update Editor
	virtual void Update( void );
	// Draw the Editor Menus
	virtual void Draw( void );

	// Function : Process_Input
	// static input handler
	void Process_Input( void );
	// Handle Input event
	virtual bool Handle_Event( SDL_Event *ev );
	/* handle key down event
	 * returns true if processed
	*/
	virtual bool Key_Down( SDLKey key );
	/* handle mouse button down event
	 * returns true if processed
	*/
	virtual bool Mouse_Down( Uint8 button );
	/* handle mouse button up event
	 * returns true if processed
	*/
	virtual bool Mouse_Up( Uint8 button );

	// Set the parent sprite manager
	virtual void Set_Sprite_Manager( cSprite_Manager *sprite_manager );

	// ##### Main Menu

	// Add Menu Entry
	void Add_Menu_Object( const std::string &name, std::string tags, CEGUI::colour normal_color = CEGUI::colour( 1, 1, 1 ) );
	// Set Active Menu Entry
	virtual void Activate_Menu_Item( cEditor_Menu_Object *entry );

	// ##### Item Menu
	// Load an defined Menu
	virtual bool Load_Item_Menu( std::string item_tag );
	// Unload the Menu
	void Unload_Item_Menu( void );
	/* Add an Object to the Item list
	 * if nName is set it will not use the object name
	 * if image is set the default object image is not used
	 */
	void Add_Item_Object( cSprite *sprite, std::string new_name = "", cGL_Surface *image = NULL );
	// Loads all Image Items
	void Load_Image_Items( std::string dir );
	// Active Item Entry
	virtual void Activate_Item( cEditor_Item_Object *entry );
	// return the sprite object
	virtual cSprite *Get_Object( const CEGUI::String &element, CEGUI::XMLAttributes &attributes, int engine_version, cSprite_Manager *sprite_manager );

	// #### Editor Functions
	// move the draw position of object to the front or back
	void Change_Draw_Position( cSprite *obj, bool move_back ) const;
	/* copy the given object(s) next to itself into the given direction
	 * if offset is given it will be used instead of the auto calculated direction size
	 * returns the new object(s)
	*/
	cSprite_List Copy_Direction( const cSprite_List &objects, const ObjectDirection dir ) const;
	cSprite *Copy_Direction( const cSprite *obj, const ObjectDirection dir, int offset = 0 ) const;
	/* Select same obect types
	 * if type is basic sprite the image filename and massivetype is also compared
	*/
	void Select_Same_Object_Types( const cSprite *obj );
	// Replace the selected basic sprites
	void Replace_Sprites( void );

	// CEGUI events
	bool Editor_Mouse_Enter( const CEGUI::EventArgs &event ); // Mouse entered Window
	bool Menu_Select( const CEGUI::EventArgs &event ); // Menu selected item
	bool Item_Select( const CEGUI::EventArgs &event ); // Item selected item

	// Menu functions
	void Function_Exit( void );
	virtual bool Function_New( void ) { return 0; };
	virtual void Function_Load( void ) {};
	virtual void Function_Save( bool with_dialog = 0 ) {};
	virtual void Function_Save_as( void ) {};
	virtual void Function_Delete( void ) {};
	virtual void Function_Reload( void ) {};
	virtual void Function_Settings( void ) {};

	// the parent sprite manager
	cSprite_Manager *m_sprite_manager;
	// true if editor is active
	bool m_enabled;

	// Editor filenames
	std::string m_menu_filename;
	std::string m_items_filename;

	// Required item tag
	std::string m_editor_item_tag;
	// editor camera speed
	float m_camera_speed;

	// Timer until the Menu will be minimized
	float m_menu_timer;

	// Objects with tags
	typedef vector<cImage_settings_data *> TaggedItemImageSettingsList;
	TaggedItemImageSettingsList m_tagged_item_images;
	typedef vector<cSprite *> TaggedItemObjectsList;
	TaggedItemObjectsList m_tagged_item_objects;

	// CEGUI window
	CEGUI::Window *m_editor_window;
	CEGUI::Listbox *m_listbox_menu;
	CEGUI::Listbox *m_listbox_items;
	CEGUI::TabControl *m_tabcontrol_menu;

protected:
	// Check if the given tag is available in the string
	bool Is_Tag_Available( const std::string &str, const std::string &tag, unsigned int search_pos = 0 );

	// Draw Editor Help Box
	void Draw_Editor_Help( void );
	/* Add a text line to the Help Box
	 * spacing : can be used f.e. a header
	 * pos_x : the line start position x
	*/
	void Add_Help_Line( std::string key_text, std::string description = "", float spacing = 0, float pos_x = 60 );

	// if true show the editor help
	bool m_show_editor_help;
	// help sprites
	typedef vector<cHudSprite *> HudSpriteList;
	HudSpriteList m_help_sprites;
private:
	// XML element start
	virtual void elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes );
	// XML element end
	virtual void elementEnd( const CEGUI::String &element );

	void Handle_Item( const CEGUI::XMLAttributes &attributes );
	void Handle_Menu( const CEGUI::XMLAttributes &attributes );

	// XML element Item Tag list
	CEGUI::XMLAttributes m_xml_attributes;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
