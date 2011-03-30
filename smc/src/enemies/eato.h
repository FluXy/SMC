/***************************************************************************
 * eato.h
 *
 * Copyright (C) 2006 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_EATO_H
#define SMC_EATO_H

#include "../enemies/enemy.h"

namespace SMC
{

/* *** *** *** *** *** cEato *** *** *** *** *** *** *** *** *** *** *** *** */
/* Eats your Butt !
 * Secret attack: Merges visually with the background and becomes an anti-maryo mine.
*/
class cEato : public cEnemy
{
public:
	// constructor
	cEato( cSprite_Manager *sprite_manager );
	// create from stream
	cEato( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager );
	// destructor
	virtual ~cEato( void );

	// init defaults
	void Init( void );
	// copy
	virtual cEato *Copy( void ) const;

	// load from stream
	virtual void Load_From_XML( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_XML( CEGUI::XMLSerializer &stream );

	// Set the image directory
	void Set_Image_Dir( std::string dir );
	// Set direction
	void Set_Direction( const ObjectDirection dir );

	/* downgrade state ( if already weakest state : dies )
	 * force : usually dies or a complete downgrade
	*/
	virtual void DownGrade( bool force = 0 );
	// dying animation update
	virtual void Update_Dying( void );

	// update
	virtual void Update( void );

	// if update is valid for the current state
	virtual bool Is_Update_Valid( void );

	/* Validate the given collision object
	 * returns 0 if not valid
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
	virtual Col_Valid_Type Validate_Collision( cSprite *obj );
	// collision from player
	virtual void Handle_Collision_Player( cObjectCollision *collision );

	// editor activation
	virtual void Editor_Activate( void );
	// editor direction option selected event
	bool Editor_Direction_Select( const CEGUI::EventArgs &event );
	// editor image dir text changed event
	bool Editor_Image_Dir_Text_Changed( const CEGUI::EventArgs &event );

	// image directory
	std::string m_img_dir;

private:
	// Create the Name from the current settings
	void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
