/***************************************************************************
 * enemystopper.h
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

#ifndef SMC_ENEMYSTOPPER_H
#define SMC_ENEMYSTOPPER_H

#include "../core/global_basic.h"
#include "../objects/animated_sprite.h"

namespace SMC
{

/* *** *** *** *** *** cEnemyStopper *** *** *** *** *** *** *** *** *** *** *** *** */

class cEnemyStopper : public cAnimated_Sprite
{
public:
	// constructor
	cEnemyStopper( cSprite_Manager *sprite_manager );
	// create from stream
	cEnemyStopper( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager );
	// destructor
	virtual ~cEnemyStopper( void );
	
	// init defaults
	void Init( void );
	// copy
	virtual cEnemyStopper *Copy( void ) const;

	// load from stream
	virtual void Load_From_XML( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_XML( CEGUI::XMLSerializer &stream );

	// draw
	virtual void Draw( cSurface_Request *request = NULL );

	// if draw is valid for the current state and position
	virtual bool Is_Draw_Valid( void );

	// editor color
	Color m_editor_color;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
