/***************************************************************************
 * i18n.h
 *
 * Copyright (C) 2008 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_I18N_H
#define SMC_I18N_H

#include "../core/global_game.h"
#include <libintl.h>

namespace SMC
{

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// translates the string with gettext
#define _(String) gettext(String)
// translates the utf8 string with gettext
#define UTF8_(String) reinterpret_cast<CEGUI::utf8*>(gettext(String))
// not translated and only for gettext detection
#define N_(String) String

// init internationalization
void I18N_Init( void );
// set language
void I18N_Set_Language( const std::string &default_language );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
