/***************************************************************************
 * animation.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2003 - 2009 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_ANIMATION_H
#define SMC_ANIMATION_H

#include "../objects/animated_sprite.h"
#include "../core/obj_manager.h"

namespace SMC
{

/* *** *** *** *** *** *** *** Animation definitions *** *** *** *** *** *** *** *** *** *** */

enum AnimationEffect
{
	ANIM_UNDEFINED,
	BLINKING_POINTS,
	FIRE_EXPLOSION,
	PARTICLE_EXPLOSION
};

/* *** *** *** *** *** *** *** Particle blending definitions *** *** *** *** *** *** *** *** *** *** */

enum BlendingMode
{
	BLEND_NONE,
	BLEND_ADD,
	BLEND_DRIVE
	// todo : more
};

/* *** *** *** *** *** *** *** Base Animation class *** *** *** *** *** *** *** *** *** *** */

class cAnimation : public cAnimated_Sprite
{
public:
	cAnimation( cSprite_Manager *sprite_manager );
	virtual ~cAnimation( void );

	// initialize animation
	virtual void Init_Anim( void );
	// update animation
	virtual void Update( void );
	// draw animation
	virtual void Draw( cSurface_Request *request = NULL );

	// Set time to live for Objects in seconds
	void Set_Time_to_Live( float time, float time_rand = 0.0f );
	/* Set speed of fading out ( 0.01 - 100 )
	* the lower the longer it takes
	* Note : only useful for non particle animation objects
	*/
	void Set_Fading_Speed( float speed );
	// set z position
	virtual void Set_Pos_Z( float pos, float pos_rand = 0.0f );

	// Z random position
	float posz_rand;
	// fading out speed
	float fading_speed;
	// object time to live
	float time_to_live, time_to_live_rand;
	// animation type
	AnimationEffect animtype;
};

/* *** *** *** *** *** *** *** *** Blinking points *** *** *** *** *** *** *** *** *** */

class cAnimation_Goldpiece : public cAnimation
{
public:
	cAnimation_Goldpiece( cSprite_Manager *sprite_manager, float posx, float posy, float height = 40.0f, float width = 20.0f );
	virtual ~cAnimation_Goldpiece( void );

	// initialize animation
	virtual void Init_Anim( void );
	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurface_Request *request = NULL );

	typedef vector<cSprite *> BlinkPointList;
	BlinkPointList objects;
	
};

/* *** *** *** *** *** *** *** Fireball Animation *** *** *** *** *** *** *** *** *** *** */

class cAnimation_Fireball_Item : public cAnimated_Sprite
{
public:
	cAnimation_Fireball_Item( cSprite_Manager *sprite_manager )
	: cAnimated_Sprite( sprite_manager )
	{
		counter = 0.0f;
	}

	virtual ~cAnimation_Fireball_Item( void ) {}

	// lifetime
	float counter;
};

class cAnimation_Fireball : public cAnimation
{
public:
	cAnimation_Fireball( cSprite_Manager *sprite_manager, float posx, float posy, unsigned int power = 5 );
	virtual ~cAnimation_Fireball( void );

	// initialize animation
	virtual void Init_Anim( void );
	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurface_Request *request = NULL );

	typedef vector<cAnimation_Fireball_Item *> FireAnimList;
	FireAnimList objects;
};

/* *** *** *** *** *** *** *** Particle Emitter item *** *** *** *** *** *** *** *** *** *** */

// pre declare
class cParticle_Emitter;

// Particle Item
class cParticle : public cMovingSprite
{
public:
	cParticle( cParticle_Emitter *parent );
	virtual ~cParticle( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurface_Request *request = NULL );

	// set gravity
	void Set_Gravity( float x, float y );

	// parent particle emitter
	cParticle_Emitter *m_parent;
	// time to live
	float m_time_to_live;
	// constant rotation
	float m_const_rot_x, m_const_rot_y, m_const_rot_z;
	// particle gravity
	float m_gravity_x, m_gravity_y;

	// fading position value
	float m_fade_pos;
};

/* *** *** *** *** *** *** *** Particle Emitter *** *** *** *** *** *** *** *** *** *** */

class cParticle_Emitter : public cAnimation
{
public:
	// constructor
	cParticle_Emitter( cSprite_Manager *sprite_manager );
	// create from stream
	cParticle_Emitter( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager );
	// destructor
	virtual ~cParticle_Emitter( void );

	// create from stream
	virtual void Create_From_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_Stream( ofstream &file );

	// Init
	virtual void Init( void );
	// copy
	virtual cParticle_Emitter *Copy( void ) const;
	// initialize animation
	virtual void Init_Anim( void );
	// Emit Particles
	virtual void Emit( void );
	// Clear Particles and Animation data
	virtual void Clear( void );

	// Update given settings
	virtual void Update( void );
	// Draw everything
	virtual void Draw( cSurface_Request *request = NULL );

	// if update is valid for the current state
	virtual bool Is_Update_Valid( void );
	// if draw is valid for the current state and position
	virtual bool Is_Draw_Valid( void );

	// Set the Emitter rect
	void Set_Emitter_Rect( float x, float y, float w = 0.0f, float h = 0.0f );
	void Set_Emitter_Rect( const GL_rect &r );
	/* Set time to live for the Emitter in seconds
	 * set -1 for infinite
	*/
	void Set_Emitter_Time_to_Live( float time );
	// Set time between Iterations
	void Set_Emitter_Iteration_Interval( float time );
	// Set Particle Count/Quota
	void Set_Quota( unsigned int size );
	// Set speed ( 0 - 100 )
	void Set_Speed( float vel_base, float vel_random = 2.0f );
	// Set start rotation z uses start direction
	void Set_Start_Rot_Z_Uses_Direction( bool enable );
	// Set x constant rotation
	void Set_Const_Rotation_X( float rot, float rot_random = 0.0f );
	// Set y constant rotation
	void Set_Const_Rotation_Y( float rot, float rot_random = 0.0f );
	// Set z constant rotation
	void Set_Const_Rotation_Z( float rot, float rot_random = 0.0f );
	/* Set direction range ( 0 - 360 )
	 * 0 : Right, 90 Down, 180 Left, 270 Up
	*/
	void Set_Direction_Range( float start, float range = 0.0f );
	// Set image scale ( 0.01 - 100 )
	virtual void Set_Scale( float nscale, float scale_random = 0.0f );
	// Set horizontal gravity
	void Set_Horizontal_Gravity( float start, float random = 0.0f );
	// Set vertical gravity
	void Set_Vertical_Gravity( float start, float random = 0.0f );
	// Set the Color
	virtual void Set_Color( const Color &col, const Color &col_rand = Color( static_cast<Uint8>(0), 0, 0, 0 ) );
	// Set fading type
	void Set_Fading_Size( bool enable );
	void Set_Fading_Alpha( bool enable );
	void Set_Fading_Color( bool enable );
	// Set blending mode
	virtual void Set_Blending( BlendingMode mode );
	// Set image
	virtual void Set_Image( cGL_Surface *img );
	// Set the file name
	virtual void Set_Filename( const std::string &str_filename );

	// editor activation
	virtual void Editor_Activate( void );
	// position z base text changed event
	bool Editor_Pos_Z_Base_Text_Changed( const CEGUI::EventArgs &event );
	// position z rand text changed event
	bool Editor_Pos_Z_Rand_Text_Changed( const CEGUI::EventArgs &event );
	// editor filename text changed event
	bool Editor_Filename_Text_Changed( const CEGUI::EventArgs &event );
	// emitter width text changed event
	bool Editor_Emitter_Width_Text_Changed( const CEGUI::EventArgs &event );
	// emitter height text changed event
	bool Editor_Emitter_Height_Text_Changed( const CEGUI::EventArgs &event );
	// emitter time to live text changed event
	bool Editor_Emitter_Time_To_Live_Text_Changed( const CEGUI::EventArgs &event );
	// emitter interval text changed event
	bool Editor_Emitter_Interval_Text_Changed( const CEGUI::EventArgs &event );
	// quota text changed event
	bool Editor_Quota_Text_Changed( const CEGUI::EventArgs &event );
	// ttl base text changed event
	bool Editor_Ttl_Base_Text_Changed( const CEGUI::EventArgs &event );
	// ttl rand text changed event
	bool Editor_Ttl_Rand_Text_Changed( const CEGUI::EventArgs &event );
	// velocity base text changed event
	bool Editor_Velocity_Base_Text_Changed( const CEGUI::EventArgs &event );
	// velocity rand text changed event
	bool Editor_Velocity_Rand_Text_Changed( const CEGUI::EventArgs &event );
	// start rotation x base text changed event
	bool Editor_Rotation_X_Base_Text_Changed( const CEGUI::EventArgs &event );
	// start rotation y base text changed event
	bool Editor_Rotation_Y_Base_Text_Changed( const CEGUI::EventArgs &event );
	// start rotation z base text changed event
	bool Editor_Rotation_Z_Base_Text_Changed( const CEGUI::EventArgs &event );
	// start rotation z uses start direction event
	bool Editor_Start_Rot_Z_Uses_Direction_Changed( const CEGUI::EventArgs &event );
	// constant rotation x base text changed event
	bool Editor_Const_Rotation_X_Base_Text_Changed( const CEGUI::EventArgs &event );
	// constant rotation x rand text changed event
	bool Editor_Const_Rotation_X_Rand_Text_Changed( const CEGUI::EventArgs &event );
	// constant rotation y base text changed event
	bool Editor_Const_Rotation_Y_Base_Text_Changed( const CEGUI::EventArgs &event );
	// constant rotation y rand text changed event
	bool Editor_Const_Rotation_Y_Rand_Text_Changed( const CEGUI::EventArgs &event );
	// constant rotation z base text changed event
	bool Editor_Const_Rotation_Z_Base_Text_Changed( const CEGUI::EventArgs &event );
	// constant rotation z rand text changed event
	bool Editor_Const_Rotation_Z_Rand_Text_Changed( const CEGUI::EventArgs &event );
	// direction base text changed event
	bool Editor_Direction_Base_Text_Changed( const CEGUI::EventArgs &event );
	// direction rand text changed event
	bool Editor_Direction_Rand_Text_Changed( const CEGUI::EventArgs &event );
	// scale base text changed event
	bool Editor_Scale_Base_Text_Changed( const CEGUI::EventArgs &event );
	// scale rand text changed event
	bool Editor_Scale_Rand_Text_Changed( const CEGUI::EventArgs &event );
	// horizontal gravity base text changed event
	bool Editor_Horizontal_Gravity_Base_Text_Changed( const CEGUI::EventArgs &event );
	// horizontal gravity rand text changed event
	bool Editor_Horizontal_Gravity_Rand_Text_Changed( const CEGUI::EventArgs &event );
	// vertical gravity base text changed event
	bool Editor_Vertical_Gravity_Base_Text_Changed( const CEGUI::EventArgs &event );
	// vertical gravity rand text changed event
	bool Editor_Vertical_Gravity_Rand_Text_Changed( const CEGUI::EventArgs &event );
	// todo : start rotation x/y/z rand, color, color_rand

	// Particle items
	typedef vector<cParticle *> ParticleList;
	ParticleList objects;

	// filename of the particle
	std::string filename;
	// emitter time to live
	float emitter_time_to_live;
	// emitter iteration interval
	float emitter_iteration_interval;
	// emitter object quota
	unsigned int emitter_quota;

	// velocity
	float vel, vel_rand;
	// start rotation z uses start direction
	bool start_rot_z_uses_direction;
	// constant rotation
	float const_rotx, const_roty, const_rotz;
	// constant rotation random modifier
	float const_rotx_rand, const_roty_rand, const_rotz_rand;
	// direction range
	float angle_start, angle_range;
	// size scaling
	float size_scale, size_scale_rand;
	// gravity
	float gravity_x, gravity_x_rand;
	float gravity_y, gravity_y_rand;
	// color random
	Color color_rand;
	// fading types (todo : dest-color)
	bool fade_size, fade_alpha, fade_color;
	// blending mode
	BlendingMode blending;

private:
	// time alive
	float emitter_living_time;
	// emit counter
	float emit_counter;
};

/* *** *** *** *** *** *** *** Animation Manager *** *** *** *** *** *** *** *** *** *** */

class cAnimation_Manager : public cObject_Manager<cAnimation>
{
public:
	cAnimation_Manager( void );
	virtual ~cAnimation_Manager( void );

	// Add an animation object with the given settings
	virtual void Add( cAnimation *animation );

	// Update the objects
	void Update( void );
	// Draw the objects
	void Draw( void );

	typedef vector<cAnimation *> cAnimation_List;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Animation Manager
extern cAnimation_Manager *pActive_Animation_Manager;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
