/***************************************************************************
 * sprite_manager.h
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

#ifndef SMC_SPRITE_MANAGER_H
#define SMC_SPRITE_MANAGER_H

#include "../core/global_game.h"
#include "../core/obj_manager.h"
#include "../objects/movingsprite.h"

namespace SMC
{

/* *** *** *** *** *** cSprite_Manager *** *** *** *** *** *** *** *** *** *** *** *** */

class cSprite_Manager : public cObject_Manager<cSprite>
{
public:
	cSprite_Manager( unsigned int reserve_items = 2000, unsigned int zpos_items = 100 );
	virtual ~cSprite_Manager( void );

	/* Add a sprite
	 */
	virtual void Add( cSprite *sprite );

	// Return a sprite copy
	cSprite *Copy( unsigned int identifier );

	// Set the sprite z position and update the z pos list
	void Set_Pos_Z( cSprite *sprite );

	/* Move the sprite to the front of the array
	 * the sprite is then behind other sprites on the screen
	 * this also sets the z position
	*/
	void Move_To_Front( cSprite *sprite );
	/* Move the sprite to the back of the array
	 * the sprite is then in front of other sprites on the screen
	 * this also sets the z position
	*/
	void Move_To_Back( cSprite *sprite );

	/* Delete all objects
	 * if delayed is set deletion will only occur if replaced
	 */
	virtual void Delete_All( bool delayed = 0 );

	// Return the first z position object from the given type
	cSprite *Get_First( const SpriteType type ) const;
	// Return the last z position object from the given type
	cSprite *Get_Last( const SpriteType type ) const;
	/* Return the matching object from the given start position
	 * type : if not set to zero only returns the object with the given type
	 * check_pos : if set to 1 the normal position must be the same
	 * if 2 return object also if found on the normal position
	*/
	cSprite *Get_from_Position( int start_pos_x, int start_pos_y, const SpriteType type = TYPE_UNDEFINED, int check_pos = 0 ) const;

	/* Get a sorted Objects Array
	 * editor_sort : if set sorts from editor z pos
	 * with_player : include player
	*/
	void Get_Objects_sorted( cSprite_List &new_objects, bool editor_sort = 0, bool with_player = 0 ) const;
	/* Get objects colliding with the given rectangle
	 * with_player : include player in check
	 * exclude_sprite : exclude the given sprite from check
	*/
	void Get_Colliding_Objects( cSprite_List &col_objects, const GL_rect &rect, bool with_player = 0, const cSprite *exclude_sprite = NULL ) const;

	// Update items drawing validation
	inline void Update_Items_Valid_Draw( void )
	{
		for( cSprite_List::iterator itr = objects.begin(); itr != objects.end(); ++itr )
		{
			(*itr)->Update_Valid_Draw();
		}
	}
	// Update items
	inline void Update_Items( void )
	{
		for( cSprite_List::iterator itr = objects.begin(); itr != objects.end(); ++itr )
		{
			(*itr)->Update();
		}
	}
	// Update_Late items
	inline void Update_Items_Late( void )
	{
		for( cSprite_List::iterator itr = objects.begin(); itr != objects.end(); ++itr )
		{
			(*itr)->Update_Late();
		}
	}
	// Draw items
	inline void Draw_Items( void )
	{
		for( cSprite_List::iterator itr = objects.begin(); itr != objects.end(); ++itr )
		{
			(*itr)->Draw();
		}
	}

	// Create Collision data and Handle the collisions
	void Handle_Collision_Items( void );


	/* Return the current size
	 * of the specified sprite array
	 */
	unsigned int Get_Size_Array( const ArrayType sprite_array );

	// Return object pointer if found
	cSprite *operator [] ( unsigned int identifier )
	{
		return Get_Pointer( identifier );
	}

	typedef vector<float> ZposList;
	// biggest type z position
	ZposList m_z_pos_data;
	// biggest editor type z position
	ZposList m_z_pos_data_editor;

	// Z position sort
	struct zpos_sort
	{
		bool operator()( const cSprite *a, const cSprite *b ) const
		{
			return a->m_pos_z > b->m_pos_z;
		}
	};

	// Editor Z position sort
	struct editor_zpos_sort
	{
		bool operator()( const cSprite *a, const cSprite *b ) const
		{
			// check if the editor zpos is available
			if( !a->m_editor_pos_z )
			{
				if( !b->m_editor_pos_z )
				{
					return a->m_pos_z < b->m_pos_z;
				}

				return a->m_pos_z < b->m_editor_pos_z;
			}
			if( !b->m_editor_pos_z )
			{
				if( !a->m_editor_pos_z )
				{
					return a->m_pos_z < b->m_pos_z;
				}

				return a->m_editor_pos_z < b->m_pos_z;
			}

			// both objects have an editor z pos
			return a->m_editor_pos_z < b->m_editor_pos_z;
		}
	};
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
