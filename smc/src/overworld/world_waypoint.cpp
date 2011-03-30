/***************************************************************************
 * waypoint.cpp  -  waypoint class for the Overworld
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
 
#include "../overworld/world_waypoint.h"
#include "../overworld/overworld.h"
#include "../core/game_core.h"
#include "../core/framerate.h"
#include "../user/preferences.h"
#include "../video/renderer.h"
#include "../level/level.h"
#include "../core/math/utilities.h"
#include "../core/i18n.h"
#include "../video/gl_surface.h"
#include "../core/filesystem/filesystem.h"
// CEGUI
#include "CEGUIWindowManager.h"
#include "elements/CEGUICombobox.h"
#include "elements/CEGUIEditbox.h"
#include "elements/CEGUIListboxTextItem.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** cWaypoint *** *** *** *** *** *** *** *** *** */

cWaypoint :: cWaypoint( cSprite_Manager *sprite_manager )
: cSprite( sprite_manager, "waypoint" )
{
	cWaypoint::Init();
}

cWaypoint :: cWaypoint( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager )
: cSprite( sprite_manager, "waypoint" )
{
	cWaypoint::Init();
	cWaypoint::Load_From_XML( attributes );
}


cWaypoint :: ~cWaypoint( void )
{
	//
}

void cWaypoint :: Init( void )
{
	m_sprite_array = ARRAY_PASSIVE;
	m_type = TYPE_OW_WAYPOINT;
	m_massive_type = MASS_PASSIVE;
	m_pos_z = cSprite::m_pos_z_massive_start;
	m_camera_range = 0;
	
	m_waypoint_type = WAYPOINT_NORMAL;
	m_name = _("Waypoint");
	
	m_access = 0;
	m_access_default = 0;
	m_direction_forward = DIR_UNDEFINED;
	m_direction_backward = DIR_UNDEFINED;

	m_glim_color = Get_Random_Float( 0, 100 );
	m_glim_mod = 1;

	m_arrow_forward = NULL;
	m_arrow_backward = NULL;

	Set_Image( pVideo->Get_Surface( "world/waypoint/default_1.png" ) );
}

cWaypoint *cWaypoint :: Copy( void ) const
{
	cWaypoint *waypoint = new cWaypoint( m_sprite_manager );
	waypoint->Set_Pos( m_start_pos_x, m_start_pos_y, 1 );
	waypoint->Set_Image( m_start_image, 1 );
	waypoint->m_waypoint_type = m_waypoint_type;
	waypoint->Set_Destination( m_destination );
	waypoint->Set_Direction_Forward( m_direction_forward );
	waypoint->Set_Direction_Backward( m_direction_backward );
	waypoint->Set_Access( m_access_default, 1 );
	return waypoint;
}

void cWaypoint :: Load_From_XML( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "x" )), static_cast<float>(attributes.getValueAsInteger( "y" )), 1 );
	// image
	/*if( attributes.exists( "image" ) )
	{
		Set_Image( pVideo->Get_Surface( attributes.getValueAsString( "image" ).c_str() ), 1 ) ;
	}*/
	// type
	m_waypoint_type = static_cast<Waypoint_type>(attributes.getValueAsInteger( "type", WAYPOINT_NORMAL ));
	// destination
	// pre 0.99.6 : world
	if( attributes.exists( "world" ) )
	{
		Set_Destination( attributes.getValueAsString( "world" ).c_str() );
	}
	// pre 0.99.6 : level
	else if( attributes.exists( "level" ) )
	{
		Set_Destination( attributes.getValueAsString( "level" ).c_str() );
	}
	// default : destination
	else
	{
		Set_Destination( attributes.getValueAsString( "destination" ).c_str() );
	}
	// backward direction
	Set_Direction_Backward( Get_Direction_Id( attributes.getValueAsString( "direction_backward", "left" ).c_str() ) );
	// forward direction
	Set_Direction_Forward( Get_Direction_Id( attributes.getValueAsString( "direction_forward", "right" ).c_str() ) );

	// access
	Set_Access( attributes.getValueAsBool( "access", 1 ), 1 );
}

void cWaypoint :: Save_To_XML( CEGUI::XMLSerializer &stream )
{
	// begin
	stream.openTag( m_type_name );

	// position
	Write_Property( stream, "x", static_cast<int>(m_start_pos_x) );
	Write_Property( stream, "y", static_cast<int>(m_start_pos_y) );
	// image
	/*if( start_image )
	{
		string img_filename = start_image->filename;

		// remove pixmaps directory from string
		if( img_filename.find( PIXMAPS_DIR ) == 0 )
		{
			img_filename.erase( 0, strlen( PIXMAPS_DIR ) + 1 );
		}

		Write_Property( stream, "image", img_filename );
	}*/
	// type
	Write_Property( stream, "type", static_cast<int>(m_waypoint_type) );
	// destination
	Write_Property( stream, "destination", m_destination );
	// direction backward
	Write_Property( stream, "direction_backward", Get_Direction_Name( m_direction_backward ) );
	// direction forward
	Write_Property( stream, "direction_forward", Get_Direction_Name( m_direction_forward ) );
	// access
	Write_Property( stream, "access", m_access_default );

	// end
	stream.closeTag();
}

void cWaypoint :: Update( void )
{
	if( m_auto_destroy )
	{
		return;
	}

	cSprite::Update();

	if( m_glim_mod )
	{
		m_glim_color += pFramerate->m_speed_factor * 3.0f;
	}
	else
	{
		m_glim_color -= pFramerate->m_speed_factor * 3.0f;
	}

	if( m_glim_color > 120.0f )
	{
		m_glim_mod = 0;
	}
	else if( m_glim_color < 7.0f )
	{
		m_glim_mod = 1;
	}
}

void cWaypoint :: Draw( cSurface_Request *request /* = NULL  */ )
{
	if( m_auto_destroy )
	{
		return;
	}

	if( pOverworld_Manager->m_debug_mode || editor_world_enabled )
	{
		float x;
		float y;

		// direction back arrow
		if( m_direction_backward == DIR_RIGHT || m_direction_backward == DIR_LEFT || m_direction_backward == DIR_UP || m_direction_backward == DIR_DOWN )
		{
			x = m_rect.m_x - pActive_Camera->m_x;
			y = m_rect.m_y - pActive_Camera->m_y;

			// create request
			cSurface_Request *surface_request = new cSurface_Request();

			if( m_direction_backward == DIR_RIGHT )
			{
				x += m_rect.m_w;
				y += (m_rect.m_h * 0.5f) - (m_arrow_backward->m_w * 0.5f);
			}
			else if( m_direction_backward == DIR_LEFT )
			{
				x -= m_arrow_backward->m_w;
				y += (m_rect.m_h * 0.5f) - (m_arrow_backward->m_w * 0.5f);
			}
			else if( m_direction_backward == DIR_UP )
			{
				y -= m_arrow_backward->m_h;
				x += (m_rect.m_w * 0.5f) - (m_arrow_backward->m_h * 0.5f);
			}
			// down
			else
			{
				y += m_rect.m_h;
				x += (m_rect.m_w * 0.5f) - (m_arrow_backward->m_h * 0.5f);
			}
			
			m_arrow_backward->Blit( x, y, 0.089f, surface_request );
			surface_request->m_shadow_pos = 2;
			surface_request->m_shadow_color = lightgreyalpha64;
			// add request
			pRenderer->Add( surface_request );
		}

		// direction forward arrow
		if( m_direction_forward == DIR_RIGHT || m_direction_forward == DIR_LEFT || m_direction_forward == DIR_UP || m_direction_forward == DIR_DOWN )
		{
			x = m_rect.m_x - pActive_Camera->m_x;
			y = m_rect.m_y - pActive_Camera->m_y;

			// create request
			cSurface_Request *surface_request = new cSurface_Request();

			if( m_direction_forward == DIR_RIGHT )
			{
				x += m_rect.m_w;
				y += (m_rect.m_h * 0.5f) - (m_arrow_forward->m_w * 0.5f);
			}
			else if( m_direction_forward == DIR_LEFT )
			{
				x -= m_arrow_forward->m_w;
				y += (m_rect.m_h * 0.5f) - (m_arrow_forward->m_w * 0.5f);
			}
			else if( m_direction_forward == DIR_UP )
			{
				y -= m_arrow_forward->m_h;
				x += (m_rect.m_w * 0.5f) - (m_arrow_forward->m_h * 0.5f);
			}
			// down
			else
			{
				y += m_rect.m_h;
				x += (m_rect.m_w * 0.5f) - (m_arrow_forward->m_h * 0.5f);
			}
			
			m_arrow_forward->Blit( x, y, 0.089f, surface_request );
			surface_request->m_shadow_pos = 2;
			surface_request->m_shadow_color = lightgreyalpha64;
			// add request
			pRenderer->Add( surface_request );
		}
	}

	// draw waypoint
	if( ( m_access && ( m_waypoint_type == 1 || m_waypoint_type == 2 ) ) || pOverworld_Manager->m_debug_mode || editor_world_enabled )
	{
		bool create_request = 0;

		if( !request )
		{
			create_request = 1;
			// create request
			request = new cSurface_Request();
		}

		// draw
		cSprite::Draw( request );

		// default color
		if( !pOverworld_Manager->m_debug_mode )
		{
			request->m_color = Color( static_cast<Uint8>(255), 100 + static_cast<Uint8>(m_glim_color), 10 );
		}
		// change to debug color
		else
		{
			if( m_access )
			{
				request->m_color = Color( static_cast<Uint8>(255), 100 + static_cast<Uint8>(m_glim_color), 200 );
			}
			else
			{
				request->m_color =  Color( static_cast<Uint8>(20), 100 + static_cast<Uint8>(m_glim_color), 10 );
			}
		}

		if( create_request )
		{
			// add request
			pRenderer->Add( request );
		}
	}
}

void cWaypoint :: Set_Direction_Forward( ObjectDirection direction )
{
	m_direction_forward = direction;

	if( direction == DIR_LEFT )
	{
		m_arrow_forward = pVideo->Get_Surface( "game/arrow/small/white/left.png" );
	}
	else if( direction == DIR_RIGHT )
	{
		m_arrow_forward = pVideo->Get_Surface( "game/arrow/small/white/right.png" );
	}
	else if( direction == DIR_UP )
	{
		m_arrow_forward = pVideo->Get_Surface( "game/arrow/small/white/up.png" );
	}
	else if( direction == DIR_DOWN )
	{
		m_arrow_forward = pVideo->Get_Surface( "game/arrow/small/white/down.png" );
	}
}

void cWaypoint :: Set_Direction_Backward( ObjectDirection direction )
{
	m_direction_backward = direction;

	if( direction == DIR_LEFT )
	{
		m_arrow_backward = pVideo->Get_Surface( "game/arrow/small/blue/left.png" );
	}
	else if( direction == DIR_RIGHT )
	{
		m_arrow_backward = pVideo->Get_Surface( "game/arrow/small/blue/right.png" );
	}
	else if( direction == DIR_UP )
	{
		m_arrow_backward = pVideo->Get_Surface( "game/arrow/small/blue/up.png" );
	}
	else if( direction == DIR_DOWN )
	{
		m_arrow_backward = pVideo->Get_Surface( "game/arrow/small/blue/down.png" );
	}
}

void cWaypoint :: Set_Access( bool enabled, bool new_start_access /* = 0 */ )
{
	m_access = enabled;
	
	if( new_start_access )
	{
		m_access_default = m_access;
	}
}

void cWaypoint :: Set_Destination( std::string str )
{
	// normal waypoint
	if( m_waypoint_type == WAYPOINT_NORMAL )
	{
		// erase file type and directory  if set
		str = Trim_Filename( str, 0, 0 );
	}
	// world waypoint
	else if( m_waypoint_type == WAYPOINT_WORLD_LINK && str.find( DATA_DIR "/" GAME_OVERWORLD_DIR "/" ) != std::string::npos )
	{
		str.erase( 0, strlen( DATA_DIR "/" GAME_OVERWORLD_DIR "/" ) );
	}

	m_destination = str;
}

std::string cWaypoint :: Get_Destination( bool with_dir /* = 0 */, bool with_end /* = 0 */ ) const
{
	std::string name = m_destination;

	if( m_waypoint_type == WAYPOINT_NORMAL )
	{
		pLevel_Manager->Get_Path( name );
		name = Trim_Filename( name, with_dir, with_end );
	}
	else if( m_waypoint_type == WAYPOINT_WORLD_LINK )
	{
		if( with_dir && name.find( DATA_DIR "/" GAME_OVERWORLD_DIR "/" ) == std::string::npos )
		{
			name.insert( 0, DATA_DIR "/" GAME_OVERWORLD_DIR "/" );
		}
	}

	return name;
}

void cWaypoint :: Editor_Activate( void )
{
	// get window manager
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// Type
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "waypoint_type" ));
	Editor_Add( UTF8_("Type"), UTF8_("Destination type"), combobox, 120, 80 );

	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Level") ) );
	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("World") ) );

	if( m_waypoint_type == WAYPOINT_NORMAL )
	{
		combobox->setText( UTF8_("Level") );
	}
	else
	{
		combobox->setText( UTF8_("World") );
	}

	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cWaypoint::Editor_Type_Select, this ) );


	// destination
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "waypoint_destination" ));
	Editor_Add( UTF8_("Destination"), UTF8_("Destination level or world"), editbox, 150 );

	editbox->setText( Get_Destination() );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cWaypoint::Editor_Destination_Text_Changed, this ) );

	// backward direction
	combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "waypoint_backward_direction" ));
	Editor_Add( UTF8_("Backward Direction"), UTF8_("Backward Direction"), combobox, 100, 105 );

	combobox->addItem( new CEGUI::ListboxTextItem( "up" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "down" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "right" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "left" ) );
	combobox->setText( Get_Direction_Name( m_direction_backward ) );

	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cWaypoint::Editor_Backward_Direction_Select, this ) );

	// forward direction
	combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "waypoint_forward_direction" ));
	Editor_Add( UTF8_("Forward Direction"), UTF8_("Forward Direction"), combobox, 100, 105 );

	combobox->addItem( new CEGUI::ListboxTextItem( "up" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "down" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "right" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "left" ) );
	combobox->setText( Get_Direction_Name( m_direction_forward ) );

	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cWaypoint::Editor_Forward_Direction_Select, this ) );

	// Access
	combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "waypoint_access" ));
	Editor_Add( UTF8_("Default Access"), UTF8_("Enable if the Waypoint should be always accessible."), combobox, 120, 80 );

	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Enabled") ) );
	combobox->addItem( new CEGUI::ListboxTextItem( UTF8_("Disabled") ) );

	if( m_access_default )
	{
		combobox->setText( UTF8_("Enabled") );
	}
	else
	{
		combobox->setText( UTF8_("Disabled") );
	}

	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cWaypoint::Editor_Access_Select, this ) );

	// init
	Editor_Init();
}

bool cWaypoint :: Editor_Type_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	if( item->getText().compare( UTF8_("Level") ) == 0 )
	{
		m_waypoint_type = WAYPOINT_NORMAL;
	}
	else
	{
		m_waypoint_type = WAYPOINT_WORLD_LINK;
	}

	return 1;

}
bool cWaypoint :: Editor_Destination_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Destination( str_text );

	return 1;
}

bool cWaypoint :: Editor_Backward_Direction_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Direction_Backward( Get_Direction_Id( item->getText().c_str() ) );

	return 1;
}

bool cWaypoint :: Editor_Forward_Direction_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Direction_Forward( Get_Direction_Id( item->getText().c_str() ) );

	return 1;
}

bool cWaypoint :: Editor_Access_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	if( item->getText().compare( UTF8_("Enabled") ) == 0 )
	{
		Set_Access( 1, 1 );
	}
	else
	{
		Set_Access( 0, 1 );
	}

	return 1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
