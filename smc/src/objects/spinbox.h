/***************************************************************************
 * spinbox.h
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

#ifndef SMC_SPINBOX_H
#define SMC_SPINBOX_H

#include "../core/global_basic.h"
#include "../objects/box.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** cSpinBox *** *** *** *** *** *** *** *** *** */

class cSpinBox : public cBaseBox
{
public:
	// constructor
	cSpinBox( cSprite_Manager *sprite_manager );
	// create from stream
	cSpinBox( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager );
	// destructor
	virtual ~cSpinBox( void );

	// init defaults
	void Init( void );

	// copy
	virtual cSpinBox *Copy( void ) const;

	// load from stream
	virtual void Load_From_XML( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_XML( CEGUI::XMLSerializer &stream );

	// Activate the Spinning
	virtual void Activate( void );
	// Stop the Spinning
	void Stop( void );
	// update
	virtual void Update( void );

	// if update is valid for the current state
	virtual bool Is_Update_Valid( void );

	// spin counter
	float m_spin_counter;

	// if spinning
	bool m_spin;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
