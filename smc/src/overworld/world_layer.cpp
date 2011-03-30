/***************************************************************************
 * layer.cpp  -  Overworld Layer class
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
 
#include "../overworld/world_layer.h"
#include "../video/renderer.h"
#include "../core/game_core.h"
#include "../overworld/overworld.h"
#include "../core/i18n.h"
#include "../overworld/world_editor.h"
// CEGUI
#include "CEGUIXMLParser.h"
#include "CEGUIWindowManager.h"
#include "CEGUIExceptions.h"
#include "elements/CEGUIEditbox.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** cLayer_Line_Point *** *** *** *** *** *** *** *** *** */

cLayer_Line_Point :: cLayer_Line_Point( cSprite_Manager *sprite_manager, cOverworld *overworld, SpriteType new_type )
: cSprite( sprite_manager )
{
	m_sprite_array = ARRAY_PASSIVE;
	m_type = new_type;
	m_massive_type = MASS_PASSIVE;
	m_pos_z = 0.087f;

	m_overworld = overworld;
	m_linked_point = NULL;

	if( m_type == TYPE_OW_LINE_START )
	{
		m_pos_z += 0.001f;
		m_color = orange;
		m_name = _("Line Start Point");
	}
	else
	{
		m_color = red;
		m_name = _("Line End Point");
	}

	m_rect.m_w = 4;
	m_rect.m_h = 4;
	m_col_rect.m_w = m_rect.m_w;
	m_col_rect.m_h = m_rect.m_h;
	m_start_rect.m_w = m_rect.m_w;
	m_start_rect.m_h = m_rect.m_h;

	Update_Position_Rect();

	m_camera_range = 0;
}

cLayer_Line_Point :: ~cLayer_Line_Point( void )
{
	Destroy();

	// remove linked point
	if( m_linked_point )
	{
		m_linked_point->m_linked_point = NULL;
	}
}

void cLayer_Line_Point :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( m_auto_destroy || !pOverworld_Manager->m_draw_layer )
	{
		return;
	}

	// point rect
	pVideo->Draw_Rect( m_col_rect.m_x - pActive_Camera->m_x, m_col_rect.m_y - pActive_Camera->m_y, m_col_rect.m_w, m_col_rect.m_h, m_pos_z, &m_color );
}

void cLayer_Line_Point :: Destroy( void )
{
	if( m_auto_destroy )
	{
		return;
	}

	if( m_type == TYPE_OW_LINE_START )
	{
		m_overworld->m_layer->Delete( static_cast<cLayer_Line_Point_Start *>(this), 0 );
	}

	cSprite::Destroy();

	// destroy linked point
	if( m_linked_point && !m_linked_point->m_auto_destroy )
	{
		m_linked_point->Destroy();
	}
}

float cLayer_Line_Point :: Get_Line_Pos_X( void ) const
{
	return m_pos_x + ( m_col_rect.m_w * 0.5f );
}

float cLayer_Line_Point :: Get_Line_Pos_Y( void ) const
{
	return m_pos_y + ( m_col_rect.m_h * 0.5f );
}


/* *** *** *** *** *** *** *** *** cLayer_Line_Point_Start *** *** *** *** *** *** *** *** *** */

cLayer_Line_Point_Start :: cLayer_Line_Point_Start( cSprite_Manager *sprite_manager, cOverworld *overworld )
: cLayer_Line_Point( sprite_manager, overworld, TYPE_OW_LINE_START )
{
	cLayer_Line_Point_Start::Init();
}

cLayer_Line_Point_Start :: cLayer_Line_Point_Start( CEGUI::XMLAttributes &attributes, cSprite_Manager *sprite_manager, cOverworld *overworld )
: cLayer_Line_Point( sprite_manager, overworld, TYPE_OW_LINE_START )
{
	cLayer_Line_Point_Start::Init();
	cLayer_Line_Point_Start::Load_From_XML( attributes );
}

cLayer_Line_Point_Start :: ~cLayer_Line_Point_Start( void )
{
	Destroy();

	// remove linked point
	if( m_linked_point )
	{
		m_linked_point->m_linked_point = NULL;
	}
}

void cLayer_Line_Point_Start :: Init( void )
{
	m_anim_type = 0;
	m_origin = 0;

	// create end point
	m_linked_point = new cLayer_Line_Point( m_sprite_manager, m_overworld, TYPE_OW_LINE_END );
	m_linked_point->m_linked_point = this;
}

cLayer_Line_Point_Start *cLayer_Line_Point_Start :: Copy( void ) const
{
	// create layer line
	// hack : assume it's copied with the editor
	cLayer_Line_Point_Start *layer_line = new cLayer_Line_Point_Start( m_sprite_manager, pWorld_Editor->m_overworld );
	// start position
	layer_line->Set_Pos( m_pos_x, m_pos_y, 1 );
	// end position
	layer_line->m_linked_point->Set_Pos( m_pos_x + 20, m_pos_y, 1 );
	// origin
	layer_line->m_origin = m_origin;
	return layer_line;
}

void cLayer_Line_Point_Start :: Load_From_XML( CEGUI::XMLAttributes &attributes )
{
	// Start
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "X1" )) - 2, static_cast<float>(attributes.getValueAsInteger( "Y1" )) - 2, 1 );
	// End
	m_linked_point->Set_Pos( static_cast<float>(attributes.getValueAsInteger( "X2" )) - 2, static_cast<float>(attributes.getValueAsInteger( "Y2" )) - 2, 1 );
	// origin
	m_origin = attributes.getValueAsInteger( "origin" );
}

void cLayer_Line_Point_Start :: Set_Sprite_Manager( cSprite_Manager *sprite_manager )
{
	cSprite::Set_Sprite_Manager( sprite_manager );

	if( m_linked_point )
	{
		m_linked_point->Set_Sprite_Manager( sprite_manager );
	}
}

void cLayer_Line_Point_Start :: Draw( cSurface_Request *request /* = NULL */ )
{
	// not a valid draw
	if( m_auto_destroy || m_linked_point->m_auto_destroy || !pOverworld_Manager->m_draw_layer )
	{
		return;
	}

	// create request
	cLine_Request *line_request = new cLine_Request();

	// drawing color
	Color color = darkgreen;

	// if active
	if( pOverworld_Player->m_current_line >= 0 && m_overworld->m_layer->objects[pOverworld_Player->m_current_line] == this )
	{
		color = lightblue;
	}

	pVideo->Draw_Line( Get_Line_Pos_X() - pActive_Camera->m_x, Get_Line_Pos_Y() - pActive_Camera->m_y, m_linked_point->Get_Line_Pos_X() - pActive_Camera->m_x, m_linked_point->Get_Line_Pos_Y() - pActive_Camera->m_y, 0.085f, &color, line_request );
	line_request->m_line_width = 6;

	// add request
	pRenderer->Add( line_request );

	// draw point rect
	cLayer_Line_Point::Draw( request );
}

GL_line cLayer_Line_Point_Start :: Get_Line( void ) const
{
	return GL_line( m_pos_x + ( m_col_rect.m_w * 0.5f ), m_pos_y + ( m_col_rect.m_h * 0.5f ), m_linked_point->m_pos_x + ( m_linked_point->m_col_rect.m_w * 0.5f ), m_linked_point->m_pos_y + ( m_linked_point->m_col_rect.m_h * 0.5f ) );
}

cWaypoint *cLayer_Line_Point_Start :: Get_End_Waypoint( void ) const
{
	// get waypoint number
	int wp_num = m_overworld->Get_Waypoint_Collision( m_linked_point->m_col_rect );

	// no waypoint collision
	if( wp_num < 0 )
	{
		cLayer_Line_Point_Start *line_col = m_overworld->m_layer->Get_Line_Collision_Start( m_linked_point->m_col_rect );

		// line collision
		if( line_col )
		{
			// follow line
			return line_col->Get_End_Waypoint();
		}
	}

	// return Waypoint
	return m_overworld->Get_Waypoint( wp_num );
}

void cLayer_Line_Point_Start :: Editor_Activate( void )
{
	// get window manager
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// origin
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "layer_line_origin" ));
	Editor_Add( UTF8_("Waypoint origin"), UTF8_("Waypoint origin"), editbox, 100 );

	editbox->setValidationString( "^[+]?\\d*$" );
	editbox->setText( int_to_string( m_origin ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cLayer_Line_Point_Start::Editor_Origin_Text_Changed, this ) );

	// init
	Editor_Init();
}

bool cLayer_Line_Point_Start :: Editor_Origin_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	m_origin = string_to_int( str_text );

	return 1;
}

/* *** *** *** *** *** *** *** *** Line Collision *** *** *** *** *** *** *** *** *** */

cLine_collision :: cLine_collision( void )
{
	m_line = NULL;
	m_line_number = -2;
	m_difference = 0;
}

/* *** *** *** *** *** *** *** *** Layer *** *** *** *** *** *** *** *** *** */

cLayer :: cLayer( cOverworld *origin )
{
	m_overworld = origin;
}

cLayer :: ~cLayer( void )
{
	Delete_All();
}

void cLayer :: Add( cLayer_Line_Point_Start *line_point )
{
	for( LayerLineList::iterator itr = objects.begin(); itr != objects.end(); ++itr )
	{
		// get pointer
		cLayer_Line_Point_Start *layer_line = (*itr);

		// if already in layer
		if( layer_line == line_point )
		{
			return;
		}
	}

	cObject_Manager<cLayer_Line_Point_Start>::Add( line_point );

	// check if in sprite manager
	if( m_overworld->m_sprite_manager->Get_Array_Num( line_point ) == -1 )
	{
		// add start point
		m_overworld->m_sprite_manager->Add( line_point );
	}
	// check if in sprite manager
	if( m_overworld->m_sprite_manager->Get_Array_Num( line_point->m_linked_point ) == -1 )
	{
		// add end point
		m_overworld->m_sprite_manager->Add( line_point->m_linked_point );
	}
}

void cLayer :: Load( const std::string &filename )
{
	Delete_All();

	try
	{
		// parse layer
	// fixme : Workaround for std::string to CEGUI::String utf8 conversion. Check again if CEGUI 0.8 works with std::string utf8
	#ifdef _WIN32
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, (const CEGUI::utf8*)filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/World/Lines.xsd", "" );
	#else
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/World/Lines.xsd", "" );
	#endif
	}
	// catch CEGUI Exceptions
	catch( CEGUI::Exception &ex )
	{
		printf( "Loading Line Layer %s CEGUI Exception %s\n", filename.c_str(), ex.getMessage().c_str() );
		pHud_Debug->Set_Text( _("Line Layer Loading failed : ") + (const std::string)ex.getMessage().c_str() );
	}
}

bool cLayer :: Save( const std::string &filename )
{
// fixme : Check if there is a more portable way f.e. with imbue()
#ifdef _WIN32
	ofstream file( utf8_to_ucs2( filename ).c_str(), ios::out | ios::trunc );
#else
	ofstream file( filename.c_str(), ios::out | ios::trunc );
#endif


	if( !file )
	{
		pHud_Debug->Set_Text( _("Couldn't save world layer ") + filename );
		return 0;
	}

	CEGUI::XMLSerializer stream( file );

	// begin layer
	stream.openTag( "layer" );

	// lines
	for( LayerLineList::iterator itr = objects.begin(); itr != objects.end(); ++itr )
	{
		cLayer_Line_Point_Start *line = (*itr);

		// begin
		stream.openTag( "line" );
			// start
			Write_Property( stream, "X1", static_cast<int>(line->Get_Line_Pos_X()) );
			Write_Property( stream, "Y1", static_cast<int>(line->Get_Line_Pos_Y()) );
			// end
			Write_Property( stream, "X2", static_cast<int>(line->m_linked_point->Get_Line_Pos_X()) );
			Write_Property( stream, "Y2", static_cast<int>(line->m_linked_point->Get_Line_Pos_Y()) );
			// origin
			Write_Property( stream, "origin", line->m_origin );
		// end line
		stream.closeTag();
	}

	// end layer
	stream.closeTag();

	file.close();

	return 1;
}

void cLayer :: Delete_All( void )
{
	// only clear array
	objects.clear();
}

cLayer_Line_Point_Start *cLayer :: Get_Line_Collision_Start( const GL_rect &line_rect )
{
	for( LayerLineList::iterator itr = objects.begin(); itr != objects.end(); ++itr )
	{
		// get pointer
		cLayer_Line_Point_Start *layer_line = (*itr);

		// check line 1
		if( line_rect.Intersects( layer_line->m_col_rect ) )
		{
			return layer_line;
		}
	}

	return NULL;
}

cLine_collision cLayer :: Get_Line_Collision_Direction( float x, float y, ObjectDirection dir, float dir_size /* = 10 */, unsigned int check_size /* = 10 */ ) const
{
	if( dir == DIR_UP )
	{
		y -= dir_size;
	}
	else if( dir == DIR_DOWN )
	{
		y += dir_size;
	}
	else if( dir == DIR_RIGHT )
	{
		x += dir_size;
	}
	else if( dir == DIR_LEFT )
	{
		x -= dir_size;
	}

	if( dir == DIR_LEFT || dir == DIR_RIGHT )
	{
		return m_overworld->m_layer->Get_Nearest( x, y, DIR_VERTICAL, check_size );
	}
	else if( dir == DIR_UP || dir == DIR_DOWN )
	{
		return m_overworld->m_layer->Get_Nearest( x, y, DIR_HORIZONTAL, check_size );
	}

	// invalid direction
	return cLine_collision();
}

cLine_collision cLayer :: Get_Nearest( float x, float y, ObjectDirection dir /* = DIR_HORIZONTAL */, unsigned int check_size /* = 15 */, int only_origin_id /* = -1 */ ) const
{
	for( LayerLineList::const_iterator itr = objects.begin(); itr != objects.end(); ++itr )
	{
		// get pointer
		cLayer_Line_Point_Start *layer_line = (*itr);

		// line is not from waypoint
		if( only_origin_id >= 0 && only_origin_id != layer_line->m_origin )
		{
			continue;
		}

		cLine_collision col = Get_Nearest_Line( layer_line, x, y, dir, check_size );

		// found
		if( col.m_line )
		{
			return col;
		}
	}

	// none found
	return cLine_collision();
}

cLine_collision cLayer :: Get_Nearest_Line( cLayer_Line_Point_Start *map_layer_line, float x, float y, ObjectDirection dir /* = DIR_HORIZONTAL */, unsigned int check_size /* = 15  */ ) const
{
	GL_line line_1, line_2;

	// create map line
	GL_line map_line = map_layer_line->Get_Line();

	// check into both directions from inside
	for( float csize = 0; csize < check_size; csize++ )
	{
		line_1.m_x1 = x;
		line_1.m_y1 = y;
		line_1.m_x2 = x;
		line_1.m_y2 = y;
		line_2 = line_1;

		// set line size
		if( dir == DIR_HORIZONTAL )
		{
			line_1.m_x1 += csize;
			line_2.m_x2 -= csize;
		}
		else // vertical
		{
			line_1.m_y1 += csize;
			line_2.m_y2 -= csize;
		}

		// debug drawing
		if( pOverworld_Manager->m_debug_mode && pOverworld_Manager->m_draw_layer )
		{
			// create request
			cLine_Request *line_request = new cLine_Request();
			pVideo->Draw_Line( line_1.m_x1 - pActive_Camera->m_x, line_1.m_y1 - pActive_Camera->m_y, line_1.m_x2 - pActive_Camera->m_x, line_1.m_y2 - pActive_Camera->m_y, map_layer_line->m_pos_z + 0.001f, &white, line_request );
			line_request->m_line_width = 2;
			line_request->m_render_count = 50;
			// add request
			pRenderer->Add( line_request );

			// create request
			line_request = new cLine_Request();
			pVideo->Draw_Line( line_2.m_x1 - pActive_Camera->m_x, line_2.m_y1 - pActive_Camera->m_y, line_2.m_x2 - pActive_Camera->m_x, line_2.m_y2 - pActive_Camera->m_y, map_layer_line->m_pos_z + 0.001f, &black, line_request );
			line_request->m_line_width = 2;
			line_request->m_render_count = 50;
			// add request
			pRenderer->Add( line_request );
		}

		// check direction line 1
		if( line_1.Intersects( &map_line ) )
		{
			cLine_collision col = cLine_collision();

			col.m_line = map_layer_line;
			col.m_line_number = Get_Array_Num( map_layer_line );
			col.m_difference = csize;

			// found
			return col;
		}

		// check direction line 2
		if( line_2.Intersects( &map_line ) )
		{
			cLine_collision col = cLine_collision();

			col.m_line = map_layer_line;
			col.m_line_number = Get_Array_Num( map_layer_line );
			col.m_difference = -csize;

			// found
			return col;
		}
	}

	// not found
	return cLine_collision();
}

void cLayer :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
{
	if( element == "property" || element == "Property" )
	{
		m_xml_attributes.add( attributes.getValueAsString( "name" ), attributes.getValueAsString( "value" ) );
	}
}

void cLayer :: elementEnd( const CEGUI::String &element )
{
	if( element == "property" || element == "Property" )
	{
		return;
	}

	if( element == "line" )
	{
		// if V.1.9 and lower : move y coordinate bottom to 0
		if( m_overworld->m_engine_version < 2 )
		{
			if( m_xml_attributes.exists( "Y1" ) )
			{
				m_xml_attributes.add( "Y1", CEGUI::PropertyHelper::floatToString( m_xml_attributes.getValueAsFloat( "Y1" ) - 600.0f ) );
			}
			if( m_xml_attributes.exists( "Y2" ) )
			{
				m_xml_attributes.add( "Y2", CEGUI::PropertyHelper::floatToString( m_xml_attributes.getValueAsFloat( "Y2" ) - 600.0f ) );
			}
		}

		// add layer line
		Add( new cLayer_Line_Point_Start( m_xml_attributes, m_overworld->m_sprite_manager, m_overworld ) );
	}
	else if( element == "layer" )
	{
		// ignore
	}
	else if( element.length() )
	{
		printf( "Warning : Overworld Layer unknown element : %s\n", element.c_str() );
	}

	// clear
	m_xml_attributes = CEGUI::XMLAttributes();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
