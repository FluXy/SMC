/***************************************************************************
 * turtle.h
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

#ifndef SMC_TURTLE_H
#define SMC_TURTLE_H

#include "../enemies/enemy.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

enum Turtle_state
{
	TURTLE_DEAD		= 0,
	TURTLE_WALK		= 1,
	TURTLE_SHELL_STAND	= 2,
	TURTLE_SHELL_RUN	= 3,
	TURTLE_FLY		= 4 // todo
};

/* *** *** *** *** *** *** cTurtle *** *** *** *** *** *** *** *** *** *** *** */
/* The evolved walking Turtle
 * Likes to play roller coaster with maryo
*/
class cTurtle : public cEnemy
{
public:
	// constructor
	cTurtle( cSprite_Manager *sprite_manager );
	// create from stream
	cTurtle( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager );
	// destructor
	virtual ~cTurtle( void );

	// init defaults
	void Init( void );
	// copy
	virtual cTurtle *Copy( void ) const;

	// load from stream
	virtual void Load_From_XML( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_XML( CEGUI::XMLSerializer &stream );

	// load from savegame
	virtual void Load_From_Savegame( cSave_Level_Object *save_object );
	// save to savegame
	virtual cSave_Level_Object *Save_To_Savegame( void );

	// Set Direction
	virtual void Set_Direction( const ObjectDirection dir, bool new_start_direction = 0 );
	// set color
	void Set_Color( DefaultColor col );

	/* Move into the opposite Direction
	 * if col_dir is given only turns around if the collision direction is in front
	 */
	virtual void Turn_Around( ObjectDirection col_dir = DIR_UNDEFINED );

	/* downgrade state ( if already weakest state : dies )
	 * force : usually dies or a complete downgrade
	*/
	virtual void DownGrade( bool force = 0 );
	// dying animation update
	virtual void Update_Dying( void );

	// set the turtle moving state
	void Set_Turtle_Moving_State( Turtle_state new_state );

	// update
	virtual void Update( void );

	// Change state to walking if it is shell
	void Stand_Up( void );
	/* Hit the given enemy
	 * returns true if enemy could get hit
	*/
	bool Hit_Enemy( cEnemy *enemy );

	// update maximum velocity values
	void Update_Velocity_Max( void );

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
	// collision from an enemy
	virtual void Handle_Collision_Enemy( cObjectCollision *collision );
	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );

	// editor activation
	virtual void Editor_Activate( void );
	// editor direction option selected event
	bool Editor_Direction_Select( const CEGUI::EventArgs &event );

	// internal turtle state
	Turtle_state m_turtle_state;

	/* If the player kicked the shell this counter is set.
	 * if this counter is higher than 0
	 * maryo cannot get killed by the shell
	 */
	float m_player_counter;

	// Color
	DefaultColor m_color_type;

private:
	// Create the Name from the current settings
	void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
