/***************************************************************************
 * rokko.h
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

#ifndef SMC_ROKKO_H
#define SMC_ROKKO_H

#include "../enemies/enemy.h"

namespace SMC
{

/* *** *** *** *** *** *** cRokko *** *** *** *** *** *** *** *** *** *** *** */
/*	A giant, slow-moving bullet
 */
class cRokko : public cEnemy 
{
public:
	// constructor
	cRokko( cSprite_Manager *sprite_manager );
	// create from stream
	cRokko( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager );
	// destructor
	virtual ~cRokko( void );

	// init defaults
	void Init( void );
	// copy
	virtual cRokko *Copy( void ) const;

	// load from stream
	virtual void Load_From_XML( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_XML( CEGUI::XMLSerializer &stream );

	// load from savegame
	virtual void Load_From_Savegame( cSave_Level_Object *save_object );

	// Set Direction
	void Set_Direction( const ObjectDirection dir );
	// set flying speed
	void Set_Speed( float nspeed );
	// Set max detection distance for the front
	void Set_Max_Distance_Front( float distance );
	// Set max detection distance for the sides
	void Set_Max_Distance_Sides( float distance );
	// activate
	void Activate( bool with_sound = 1 );

	/* downgrade state ( if already weakest state : dies )
	 * force : usually dies or a complete downgrade
	*/
	virtual void DownGrade( bool force = 0 );
	// dying animation update
	virtual void Update_Dying( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurface_Request *request = NULL );

	// update the distance rect
	void Update_Distance_rect( void );
	// Get the final distance rect
	GL_rect Get_Final_Distance_Rect( void ) const;

	// Generates Smoke Particles
	void Generate_Smoke( unsigned int amount = 10 ) const;
	// Generates Spark Particles
	void Generate_Sparks( unsigned int amount = 5 ) const;

	// if update is valid for the current state
	virtual bool Is_Update_Valid( void );
	// if draw is valid for the current state and position
	virtual bool Is_Draw_Valid( void );

	/* Validate the given collision object
	 * returns 0 if not valid
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
	virtual Col_Valid_Type Validate_Collision( cSprite *obj );
	// collision from player
	virtual void Handle_Collision_Player( cObjectCollision *collision );
	// handle moved out of Level event
	virtual void Handle_out_of_Level( ObjectDirection dir );

	// editor activation
	virtual void Editor_Activate( void );
	// editor direction option selected event
	bool Editor_Direction_Select( const CEGUI::EventArgs &event );
	// editor speed text changed event
	bool Editor_Speed_Text_Changed( const CEGUI::EventArgs &event );

	// smoke particle counter
	float m_smoke_counter;
	// flying speed
	float m_speed;
	// maximum and minimum detection distance for the front
	float m_max_distance_front;
	float m_min_distance_front;
	// maximum detection distance for the sides
	float m_max_distance_sides;
	// detection distance rect
	GL_rect m_distance_rect;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
