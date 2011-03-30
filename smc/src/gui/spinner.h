/***************************************************************************
 * spinner.h
 *
 * Copyright (C) 2010 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_SPINNER_H
#define SMC_SPINNER_H

#include "../core/global_basic.h"
// CEGUI
#include "elements/CEGUISpinner.h"
#include "CEGUIWindowFactory.h"

namespace CEGUI
{

/* *** *** *** *** *** *** *** *** SMC_Spinner *** *** *** *** *** *** *** *** *** */

class SMC_Spinner : public Spinner
{
public:
	SMC_Spinner( const String& type, const String& name );
	virtual ~SMC_Spinner( void );

    /*!
    \brief
        Returns the textual representation of the current spinner value.

    \return
        String object that is equivalent to the the numerical value of the spinner.
    */
    virtual String getTextFromValue( void ) const;

	// Events
	//bool Mouse_Wheel( const CEGUI::EventArgs &event );

	static const String WidgetTypeName;
};

CEGUI_DECLARE_WINDOW_FACTORY(SMC_Spinner)

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace CEGUI

#endif
