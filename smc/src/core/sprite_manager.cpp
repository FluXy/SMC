/***************************************************************************
 * sprite_manager.cpp  -  Sprite Manager
 *
 * Copyright (C) 2005 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/sprite_manager.h"
#include "../core/game_core.h"
#include "../level/level_player.h"
#include "../input/mouse.h"
#include "../overworld/world_player.h"
#include <algorithm>

namespace SMC
{

/* *** *** *** *** *** *** cSprite_Manager *** *** *** *** *** *** *** *** *** *** *** */

cSprite_Manager :: cSprite_Manager( unsigned int reserve_items /* = 2000 */, unsigned int zpos_items /* = 100 */ )
: cObject_Manager<cSprite>()
{
	objects.reserve( reserve_items );

	m_z_pos_data.assign( zpos_items, 0.0f );
	m_z_pos_data_editor.assign( zpos_items,0.0f );
}

cSprite_Manager :: ~cSprite_Manager( void )
{
	Delete_All();
}

void cSprite_Manager :: Add( cSprite *sprite )
{
	// empty object
	if( !sprite )
	{
		return;
	}

	Set_Pos_Z( sprite );

	// Check if an destroyed object can be replaced
	for( cSprite_List::iterator itr = objects.begin(); itr != objects.end(); ++itr )
	{
		// get object pointer
		cSprite *obj = (*itr);

		// if destroy is set
		if( obj->m_auto_destroy )
		{
			// set new object
			*itr = sprite;
			// delete old
			delete obj;

			return;
		}
	}

	cObject_Manager<cSprite>::Add( sprite );
}

cSprite *cSprite_Manager :: Copy( unsigned int identifier )
{
	if( identifier >= objects.size() )
	{
		return NULL;
	}

	return objects[identifier]->Copy();
}

void cSprite_Manager :: Set_Pos_Z( cSprite *sprite )
{
	// don't set particle effect z position
	if( sprite->m_type == TYPE_ANIMATION || sprite->m_type == TYPE_PARTICLE_EMITTER )
	{
		return;
	}

	// set new z position if unset
	if( sprite->m_pos_z <= m_z_pos_data[sprite->m_type] )
	{
		sprite->m_pos_z = m_z_pos_data[sprite->m_type] + 0.000001f;
	}
	// if editor z position is given
	if( sprite->m_editor_pos_z > 0.0f )
	{
		if( sprite->m_editor_pos_z <= m_z_pos_data_editor[sprite->m_type] )
		{
			sprite->m_editor_pos_z = m_z_pos_data_editor[sprite->m_type] + 0.000001f;
		}
	}


	// update z position
	if( sprite->m_pos_z > m_z_pos_data[sprite->m_type] )
	{
		m_z_pos_data[sprite->m_type] = sprite->m_pos_z;
	}
	// if editor z position is given
	if( sprite->m_editor_pos_z > 0.0f )
	{
		if( sprite->m_editor_pos_z > m_z_pos_data_editor[sprite->m_type] )
		{
			m_z_pos_data_editor[sprite->m_type] = sprite->m_editor_pos_z;
		}
	}
}

void cSprite_Manager :: Move_To_Front( cSprite *sprite )
{
	// not needed
	if( objects.size() <= 1 )
	{
		return;
	}

	cSprite *first = objects.front();

	// if already in front
	if( sprite == first )
	{
		return;
	}

	// get iterator
	cSprite_List::iterator itr = std::find( objects.begin(), objects.end(), sprite );

	// not available
	if( itr == objects.end() )
	{
		// fixme : should not happen but it does
		return;
	}

	objects.erase( itr );
	objects.front() = sprite;
	objects.insert( objects.begin() + 1, first );

	// make it the first z position
	sprite->m_pos_z = Get_First( sprite->m_type )->m_pos_z - 0.000001f;
}

void cSprite_Manager :: Move_To_Back( cSprite *sprite )
{
	// not needed
	if( objects.size() <= 1 )
	{
		return;
	}
	
	cSprite *last = objects.back();

	// if already in back
	if( sprite == last )
	{
		return;
	}

	// get iterator
	cSprite_List::iterator itr = std::find( objects.begin(), objects.end(), sprite );

	// not available
	if( itr == objects.end() )
	{
		// fixme : should not happen but it does
		return;
	}

	objects.erase( itr );
	objects.back() = sprite;
	objects.insert( objects.end() - 1, last );

	// make it the last z position
	sprite->m_pos_z = Get_Last( sprite->m_type )->m_pos_z + 0.000001f;
}

void cSprite_Manager :: Delete_All( bool delayed /* = 0 */ )
{
	// delayed
	if( delayed )
	{
		for( cSprite_List::iterator itr = objects.begin(); itr != objects.end(); ++itr )
		{
			// get object pointer
			cSprite *obj = (*itr);

			obj->Destroy();
		}
	}
	// instant
	else
	{
		// remove objects that can not be auto-deleted
		for( cSprite_List::iterator itr = objects.begin(); itr != objects.end(); )
		{
			// get object pointer
			cSprite *obj = (*itr);

			if( obj->m_disallow_managed_delete )
			{
				itr = objects.erase( itr );
			}
			// increment
			else
			{
				++itr;
			}
		}

		cObject_Manager<cSprite>::Delete_All();
	}

	// clear z position data
	std::fill( m_z_pos_data.begin(), m_z_pos_data.end(), 0.0f );
	std::fill( m_z_pos_data_editor.begin(), m_z_pos_data_editor.end(), 0.0f );
}

cSprite *cSprite_Manager :: Get_First( const SpriteType type ) const
{
	cSprite *first = NULL;

	for( cSprite_List::const_iterator itr = objects.begin(); itr != objects.end(); ++itr )
	{
		// get object pointer
		cSprite *obj = (*itr);

		if( obj->m_type == type && ( !first || obj->m_pos_z < first->m_pos_z ) )
		{
			first = obj;
		}
	}

	// return result
	return first;
}

cSprite *cSprite_Manager :: Get_Last( const SpriteType type ) const
{
	cSprite *last = NULL;

	for( cSprite_List::const_iterator itr = objects.begin(); itr != objects.end(); ++itr )
	{
		// get object pointer
		cSprite *obj = (*itr);

		if( obj->m_type == type && ( !last || obj->m_pos_z > last->m_pos_z ) )
		{
			last = obj;
		}
	}

	// return result
	return last;
}

cSprite *cSprite_Manager :: Get_from_Position( int start_pos_x, int start_pos_y, const SpriteType type /* = TYPE_UNDEFINED */, int check_pos /* = 0 */ ) const
{
	for( cSprite_List::const_iterator itr = objects.begin(); itr != objects.end(); ++itr )
	{
		// get object pointer
		cSprite *obj = (*itr);

		if( static_cast<int>(obj->m_start_pos_x) != start_pos_x || static_cast<int>(obj->m_start_pos_y) != start_pos_y )
		{
			if( check_pos != 2 || ( static_cast<int>(obj->m_pos_x) != start_pos_x && static_cast<int>(obj->m_pos_y) != start_pos_y ) )
			{
				continue;
			}
		}

		if( check_pos == 1 && ( static_cast<int>(obj->m_pos_x) != start_pos_x || static_cast<int>(obj->m_pos_y) != start_pos_y ) )
		{
			continue;
		}
		

		// if type is given
		if( type != TYPE_UNDEFINED )
		{
			// skip invalid type
			if( obj->m_type != type )
			{
				continue;
			}
		}

		// found
		return obj;
	}

	return NULL;
}

void cSprite_Manager :: Get_Objects_sorted( cSprite_List &new_objects, bool editor_sort /* = 0 */, bool with_player /* = 0 */ ) const
{
	new_objects = objects;

	if( with_player )
	{
		new_objects.push_back( pActive_Player );
	}

	// z position sort
	if( !editor_sort )
	{
		// default
		std::sort( new_objects.begin(), new_objects.end(), zpos_sort() );
	}
	else
	{
		// editor
		std::sort( new_objects.begin(), new_objects.end(), editor_zpos_sort() );
	}
}

void cSprite_Manager :: Get_Colliding_Objects( cSprite_List &col_objects, const GL_rect &rect, bool with_player /* = 0 */, const cSprite *exclude_sprite /* = NULL */ ) const
{
	// Check objects
	for( cSprite_List::const_iterator itr = objects.begin(); itr != objects.end(); ++itr )
	{
		// get object pointer
		cSprite *obj = (*itr);

		// if destroyed object
		if( obj == exclude_sprite || obj->m_auto_destroy )
		{
			continue;
		}

		// if rects don't touch
		if( !rect.Intersects( obj->m_col_rect ) )
		{
			continue;
		}

		col_objects.push_back( obj );
	}

	if( with_player && pActive_Player != exclude_sprite )
	{
		if( rect.Intersects( pActive_Player->m_col_rect ) )
		{
			col_objects.push_back( pActive_Player );
		}
	}
}

void cSprite_Manager :: Handle_Collision_Items( void )
{
	for( cSprite_List::iterator itr = objects.begin(); itr != objects.end(); ++itr )
	{
		cSprite *obj = (*itr);

		// invalid
		if( obj->m_auto_destroy )
		{
			if( obj->m_collisions.size() )
			{
				debug_print( "Collision with a destroyed object (%s)\n", obj->m_name.c_str() );
				obj->Clear_Collisions();
			}

			continue;
		}

		// collision and movement handling
		obj->Collide_Move();
		// handle found collisions
		obj->Handle_Collisions();
	}
}

unsigned int cSprite_Manager :: Get_Size_Array( const ArrayType sprite_array )
{
	unsigned int count = 0;

	for( cSprite_List::const_iterator itr = objects.begin(); itr != objects.end(); ++itr )
	{
		if( (*itr)->m_sprite_array == sprite_array )
		{
			count++;
		}
	}

	return count;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
