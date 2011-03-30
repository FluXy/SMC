/***************************************************************************
 * turtle_boss.h
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

#ifndef SMC_TURTLEBOSS_H
#define SMC_TURTLEBOSS_H

#include "../../enemies/enemy.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** Turtle Boss state *** *** *** *** *** *** *** *** *** */

enum TurtleBoss_state
{
	TURTLEBOSS_DEAD		= 0,
	TURTLEBOSS_WALK		= 1,
	TURTLEBOSS_SHELL_STAND	= 2,
	TURTLEBOSS_SHELL_RUN	= 3,
	TURTLEBOSS_STAND_ANGRY = 4
};

/* *** *** *** *** *** *** *** *** Turtle Boss *** *** *** *** *** *** *** *** *** */
/* The uber-evolved walking Turtle
 * Likes to play roller coaster with maryo and bursts out in fire if he gets mad.
*/
class cTurtleBoss : public cEnemy
{
public:
	// constructor
	cTurtleBoss( cSprite_Manager *sprite_manager );
	// create from stream
	cTurtleBoss( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager );
	// destructor
	virtual ~cTurtleBoss( void );

	// init defaults
	void Init( void );

	// copy
	virtual cTurtleBoss *Copy( void ) const;

	// load from stream
	virtual void Load_From_XML( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_XML( CEGUI::XMLSerializer &stream );

	// maximum hits until downgrade
	void Set_Max_Hits( int nmax_hits );
	// maximum downgrades until death
	void Set_Max_Downgrade_Counts( int nmax_downgrade_count );
	// time running as shell until staying up
	void Set_Shell_Time( float nmax_downgrade_time );

	// Set Direction
	void Set_Direction( const ObjectDirection dir, bool new_start_direction = 0 );
	// set color
	void Set_Color( DefaultColor col );
	// Set if the level ends when killed
	void Set_Level_Ends_If_Killed( bool level_ends_if_killed );

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
	void Set_Turtle_Moving_State( TurtleBoss_state new_state );

	// update
	virtual void Update( void );

	// Change state to walking if it is shell
	void Stand_Up( void );
	/* Hit the given enemy
	 * returns true if enemy could get hit
	*/
	bool Hit_Enemy( cEnemy *enemy ) const;
	// Throw Fireballs upwards
	void Throw_Fireballs( unsigned int amount = 6 );
	// Generates Star Particles
	void Generate_Stars( unsigned int amount = 1, float particle_scale = 0.4f ) const;

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
	// editor max hits text changed event
	bool Editor_Max_Hits_Text_Changed( const CEGUI::EventArgs &event );
	// editor max downgrades text changed event
	bool Editor_Max_Downgrade_Counts_Text_Changed( const CEGUI::EventArgs &event );
	// editor max downgrades time text changed event
	bool Editor_Shell_Time_Text_Changed( const CEGUI::EventArgs &event );
	// editor level ends if killed option selected event
	bool Editor_Level_Ends_If_Killed( const CEGUI::EventArgs &event );

	// internal turtle state
	TurtleBoss_state m_turtle_state;

	// times hit since the start or downgrade
	int m_hits;
	// maximum hits until downgrade
	int m_max_hits;

	/* If the player kicked the shell this counter starts.
	 * if this counter is higher than 0
	 * maryo cannot get killed by the shell
	 */
	float m_player_counter;

	// Color
	DefaultColor m_color_type;

protected:
	// Create the Name from the current settings
	void Create_Name( void );

	// times downgraded
	int m_downgrade_count;
	// maximum downgrades until death
	int m_max_downgrade_count;
	// time running as shell until staying up
	float m_shell_time;
	// time running as shell counter
	float m_run_time_counter;
	// if the level ends when killed
	bool m_level_ends_if_killed;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
