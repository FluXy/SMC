/***************************************************************************
 * global_effect.h  -  header for the corresponding cpp file
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

#ifndef SMC_GLOBAL_EFFECT_H
#define SMC_GLOBAL_EFFECT_H

#include "../core/global_basic.h"
#include "../core/math/rect.h"
#include "../video/animation.h"

namespace SMC
{

/* *** *** *** *** *** *** *** Global Effect Type *** *** *** *** *** *** *** *** *** *** */

enum  GlobalEffectType
{
	GL_EFF_NONE = 0,	// disabled
	GL_EFF_FALLING = 1
};

/* *** *** *** *** *** *** *** Global effect class *** *** *** *** *** *** *** *** *** *** */

class cGlobal_effect : public cParticle_Emitter
{
public:
	// default constructor
	cGlobal_effect( cSprite_Manager *sprite_manager );
	// create from stream
	cGlobal_effect( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager );
	// destructor
	virtual ~cGlobal_effect( void );

	// initialise
	virtual void Init_Anim( void );
	// clear
	virtual void Clear( void );

	// create from stream
	virtual void Create_From_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_Stream( ofstream &file );

	// update
	virtual void Update( void );
	// find out of camera particles and move them to the opposite screen side
	void Update_Particles( void );
	// draw
	virtual void Draw( void );

	// Sets global effect type
	void Set_Type( const GlobalEffectType type );
	void Set_Type( const std::string &str_type );
	// Set image
	void Set_Image( const std::string &img_file );

	// Return the type name
	std::string Get_Type_Name( void ) const;

	// type
	GlobalEffectType m_global_effect_type;

	// image filename
	std::string m_image_filename;

	// valid global effect
	bool m_valid;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
