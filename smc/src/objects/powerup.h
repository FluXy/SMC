/***************************************************************************
 * powerup.h
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

#ifndef SMC_POWERUP_H
#define SMC_POWERUP_H

#include "../core/global_basic.h"
#include "../objects/animated_sprite.h"

namespace SMC
{

/* *** *** *** *** *** cPowerUp *** *** *** *** *** *** *** *** *** *** *** *** */

class cPowerUp : public cAnimated_Sprite
{
public:
	// constructor
	cPowerUp( cSprite_Manager *sprite_manager );
	// destructor
	virtual ~cPowerUp( void );

	// load from savegame
	virtual void Load_From_Savegame( cSave_Level_Object *save_object );
	// save to savegame
	virtual cSave_Level_Object *Save_To_Savegame( void );

	/* draw
	 * a spawned powerup doesn't draw in editor mode
	*/
	virtual void Draw( cSurface_Request *request = NULL );

	// if update is valid for the current state
	virtual bool Is_Update_Valid( void );

	/* Validate the given collision object
	 * returns 0 if not valid
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
	virtual Col_Valid_Type Validate_Collision( cSprite *obj );
	// handle moved out of Level event
	virtual void Handle_out_of_Level( ObjectDirection dir );

	float m_counter;
};

/* *** *** *** *** *** cMushroom *** *** *** *** *** *** *** *** *** *** *** *** */

class cMushroom : public cPowerUp
{
public:
	// constructor
	cMushroom( cSprite_Manager *sprite_manager );
	// create from stream
	cMushroom( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager );
	// destructor
	virtual ~cMushroom( void );

	// init defaults
	void Init( void );
	// copy
	virtual cMushroom *Copy( void ) const;

	// load from stream
	virtual void Load_From_XML( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_XML( CEGUI::XMLSerializer &stream );

	// Set the Mushroom Type
	void Set_Type( SpriteType new_type );

	// Activates the item
	virtual void Activate( void );

	// update
	virtual void Update( void );

	// collision from player
	virtual void Handle_Collision_Player( cObjectCollision *collision );
	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );
	// collision from a box
	virtual void Handle_Collision_Box( ObjectDirection cdirection, GL_rect *r2 );

	// glim animation modifier
	bool m_glim_mod;
};

/* *** *** *** *** *** cFirePlant *** *** *** *** *** *** *** *** *** *** *** *** */

class cFirePlant : public cPowerUp
{
public:
	// constructor
	cFirePlant( cSprite_Manager *sprite_manager );
	// create from stream
	cFirePlant( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager );
	// destructor
	virtual ~cFirePlant( void );

	// init defaults
	void Init( void );
	// copy
	virtual cFirePlant *Copy( void ) const;

	// load from stream
	virtual void Load_From_XML( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_XML( CEGUI::XMLSerializer &stream );

	// Activates the item
	virtual void Activate( void );

	// update
	virtual void Update( void );

	// collision from player
	virtual void Handle_Collision_Player( cObjectCollision *collision );

	float m_particle_counter;
};

/* *** *** *** *** *** cMoon *** *** *** *** *** *** *** *** *** *** *** *** */

class cMoon : public cPowerUp
{
public:
	// constructor
	cMoon( cSprite_Manager *sprite_manager );
	// create from stream
	cMoon( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager );
	// destructor
	virtual ~cMoon( void );

	// init defaults
	void Init( void );
	// copy
	virtual cMoon *Copy( void ) const;

	// load from stream
	virtual void Load_From_XML( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_XML( CEGUI::XMLSerializer &stream );

	// Activates the item
	virtual void Activate( void );

	// update
	virtual void Update( void );

	// collision from player
	virtual void Handle_Collision_Player( cObjectCollision *collision );

	float m_particle_counter;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
