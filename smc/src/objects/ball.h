/***************************************************************************
 * ball.h
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

#ifndef SMC_BALL_H
#define SMC_BALL_H

#include "../video/video.h"
#include "../objects/animated_sprite.h"

namespace SMC
{

/* *** *** *** *** *** *** *** Ball class *** *** *** *** *** *** *** *** *** *** */

class cBall : public cAnimated_Sprite
{
public:
	// constructor
	cBall( cSprite_Manager *sprite_manager );
	// create from stream
	cBall( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager );
	// destructor
	virtual ~cBall( void );

	// init defaults
	void Init( void );
	// copy
	virtual cBall *Copy( void ) const;
	// load from stream
	virtual void Load_From_XML( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_XML( CEGUI::XMLSerializer &stream );

	// load from savegame
	virtual void Load_From_Savegame( cSave_Level_Object *save_object );
	// save to savegame
	virtual cSave_Level_Object *Save_To_Savegame( void );

	// set type
	void Set_Ball_Type( ball_effect type );
	// set origin
	void Set_Origin( ArrayType origin_array, SpriteType origin_type );
	// like Destroy but with sound option
	void Destroy_Ball( bool with_sound = 0 );

	/* set this sprite to destroyed and completely disable it
	 * sprite is still in the sprite manager but only to get possibly replaced
	*/
	virtual void Destroy( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurface_Request *request = NULL );

	// Generate the default animation Particles
	void Generate_Particles( cParticle_Emitter *anim = NULL ) const;

	/* Validate the given collision object
	 * returns 0 if not valid
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
	virtual Col_Valid_Type Validate_Collision( cSprite *obj );
	// default collision handler
	virtual void Handle_Collision( cObjectCollision *collision );
	// collision from player
	virtual void Handle_Collision_Player( cObjectCollision *collision );
	// collision from an enemy
	virtual void Handle_Collision_Enemy( cObjectCollision *collision );
	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );
	// handle moved out of Level event
	virtual void Handle_out_of_Level( ObjectDirection dir );

	// origin
	ArrayType m_origin_array;
	SpriteType m_origin_type;
	// ball type
	ball_effect m_ball_type;

	// glim animation modifier
	float m_glim_mod;
	// glim animation counter
	float m_glim_counter;
	// fire particle counter
	float m_fire_counter;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
