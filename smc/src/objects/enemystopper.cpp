/***************************************************************************
 * enemystopper.cpp  -  enemystopper class
 *
 * Copyright (C) 2003 - 2010 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../objects/enemystopper.h"
#include "../level/level_editor.h"
#include "../core/game_core.h"
#include "../core/i18n.h"

namespace SMC
{

/* *** *** *** *** *** cEnemyStopper *** *** *** *** *** *** *** *** *** *** *** *** */

cEnemyStopper :: cEnemyStopper( cSprite_Manager *sprite_manager )
: cAnimated_Sprite( sprite_manager )
{
	cEnemyStopper::Init();
}

cEnemyStopper :: cEnemyStopper( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager )
: cAnimated_Sprite( sprite_manager )
{
	cEnemyStopper::Init();
	cEnemyStopper::Create_From_Stream( attributes );
}

cEnemyStopper :: ~cEnemyStopper( void )
{

}

cEnemyStopper *cEnemyStopper :: Copy( void ) const
{
	cEnemyStopper *enemystopper = new cEnemyStopper( m_sprite_manager );
	enemystopper->Set_Pos( m_start_pos_x, m_start_pos_y, 1 );
	return enemystopper;
}

void cEnemyStopper :: Init( void )
{
	m_sprite_array = ARRAY_ACTIVE;
	m_type = TYPE_ENEMY_STOPPER;
	m_massive_type = MASS_PASSIVE;
	m_editor_pos_z = 0.11f;

	m_name = _("Enemystopper");

	// size
	m_rect.m_w = 15.0f;
	m_rect.m_h = 15.0f;
	m_col_rect.m_w = m_rect.m_w;
	m_col_rect.m_h = m_rect.m_h;
	m_start_rect.m_w = m_rect.m_w;
	m_start_rect.m_h = m_rect.m_h;

	m_editor_color = Color( static_cast<Uint8>(0), 0, 255, 128 );
}

void cEnemyStopper :: Create_From_Stream( CEGUI::XMLAttributes &attributes )
{
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
}

void cEnemyStopper :: Save_To_Stream( ofstream &file )
{
	// begin enemystopper
	file << "\t<enemystopper>" << std::endl;

	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(m_start_pos_x) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(m_start_pos_y) << "\" />" << std::endl;

	// end enemystopper
	file << "\t</enemystopper>" << std::endl;
}

void cEnemyStopper :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( !m_valid_draw )
	{
		return;
	}

	pVideo->Draw_Rect( m_col_rect.m_x - pActive_Camera->m_x, m_col_rect.m_y - pActive_Camera->m_y, m_col_rect.m_w, m_col_rect.m_h, m_editor_pos_z, &m_editor_color );
}

bool cEnemyStopper :: Is_Draw_Valid( void )
{
	// if editor not enabled
	if( !editor_enabled )
	{
		return 0;
	}

	// if not active or not on the screen
	if( !m_active || !Is_Visible_On_Screen() )
	{
		return 0;
	}

	return 1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
