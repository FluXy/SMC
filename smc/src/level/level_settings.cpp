/***************************************************************************
 * level_settings.cpp  - level editor settings class
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

#include "../core/global_basic.h"
#include "../core/game_core.h"
#include "../level/level_settings.h"
#include "../input/mouse.h"
#include "../level/level.h"
#include "../video/font.h"
#include "../video/renderer.h"
#include "../core/filesystem/filesystem.h"
#include "../core/framerate.h"

namespace SMC
{

/* *** *** *** *** *** cLevel_Settings *** *** *** *** *** *** *** *** *** *** *** *** */

cLevel_Settings :: cLevel_Settings( cSprite_Manager *sprite_manager, cLevel *level )
{
	m_active = 0;

	// Main
	m_background_preview = new cHudSprite( sprite_manager );
	// Global Effects
	m_global_effect_preview = new cHudSprite( sprite_manager );
	m_global_effect_preview->Set_Pos( game_res_w * 0.75f, game_res_h * 0.2f );
	m_global_effect_preview->Set_Shadow( lightgrey, 2 );

	m_level = level;
	m_camera = new cCamera( sprite_manager );
	m_gui_window = NULL;
	m_tabcontrol = NULL;
}

cLevel_Settings :: ~cLevel_Settings( void )
{
	Unload();

	delete m_background_preview;
	delete m_global_effect_preview;
	delete m_camera;
}

void cLevel_Settings :: Init( void )
{
	if( m_gui_window )
	{
		Unload();
	}
	
	// GUI
	m_gui_window = CEGUI::WindowManager::getSingleton().loadWindowLayout( "level_settings/main.layout" );
	pGuiSystem->getGUISheet()->addChildWindow( m_gui_window );
	
	// Tab Control
	m_tabcontrol = static_cast<CEGUI::TabControl *>(CEGUI::WindowManager::getSingleton().getWindow( "tabcontrol_main" ));
	// tab main
	CEGUI::Window *tabwindow = CEGUI::WindowManager::getSingleton().loadWindowLayout( "level_settings/tab_main.layout" );
	m_tabcontrol->addTab( tabwindow );
	// Tab background
	tabwindow = CEGUI::WindowManager::getSingleton().loadWindowLayout( "level_settings/tab_background.layout" );
	m_tabcontrol->addTab( tabwindow );
	// tab global effects
	tabwindow = CEGUI::WindowManager::getSingleton().loadWindowLayout( "level_settings/tab_global_effect.layout" );
	m_tabcontrol->addTab( tabwindow );

	// Main
	// level filename
	CEGUI::Editbox *editbox_level_filename = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_level_filename" ));
	editbox_level_filename->setText( Trim_Filename( m_level->m_level_filename, 0, 0 ).c_str() );
	// music filename
	CEGUI::Editbox *editbox_music_filename = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_music_filename" ));
	editbox_music_filename->setText( m_level->Get_Musicfile( 1 ).c_str() );
	// author
	CEGUI::Editbox *editbox_author = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_author" ));
	editbox_author->setText( reinterpret_cast<const CEGUI::utf8*>(m_level->m_author.c_str()) );
	// version
	CEGUI::Editbox *editbox_version = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_version" ));
	editbox_version->setText( m_level->m_version.c_str() );
	// camera limits
	CEGUI::Spinner *spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_camera_limit_w" ));
	spinner->setCurrentValue( m_level->m_camera_limits.m_w );
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_camera_limit_h" ));
	spinner->setCurrentValue( m_level->m_camera_limits.m_h );
	// fixed camera horizontal velocity
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_camera_hor_vel" ));
	spinner->setCurrentValue( m_level->m_fixed_camera_hor_vel );
	// last save time
	CEGUI::Editbox *editbox_save_time = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_save_time" ));
	editbox_save_time->setText( Time_to_String( m_level->m_last_saved, "%Y-%m-%d  %H:%M:%S" ) );


	// add background image button
	CEGUI::PushButton *button_add_background_image = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_add_background_image" ));
	button_add_background_image->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cLevel_Settings::Add_Background_Image, this ) );
	// delete background image button
	CEGUI::PushButton *button_delete_background_image = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_delete_background_image" ));
	button_delete_background_image->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cLevel_Settings::Delete_Background_Image, this ) );
	// save and exit button
	CEGUI::PushButton *button_save = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_save" ));
	button_save->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &cLevel_Settings::Button_Save, this ) );

	// Background
	// listbox
	CEGUI::Listbox *listbox = static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_backgrounds" ));
	listbox->setSortingEnabled( 1 );
	listbox->subscribeEvent( CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber( &cLevel_Settings::Set_Background_Image, this ) );
	// type
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(CEGUI::WindowManager::getSingleton().getWindow( "combo_bg_image_type" ));
	combobox->addItem( new CEGUI::ListboxTextItem( cBackground::Get_Type_Name( BG_NONE ) ) );
	combobox->addItem( new CEGUI::ListboxTextItem( cBackground::Get_Type_Name( BG_IMG_TOP ) ) );
	combobox->addItem( new CEGUI::ListboxTextItem( cBackground::Get_Type_Name( BG_IMG_BOTTOM ) ) );
	combobox->addItem( new CEGUI::ListboxTextItem( cBackground::Get_Type_Name( BG_IMG_ALL ) ) );
	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cLevel_Settings::Update_BG_Image, this ) );
	// filename
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_image_name" ));
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cLevel_Settings::Update_BG_Image, this ) );
	// speed
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_bg_image_speed_x" ));
	spinner->subscribeEvent( CEGUI::Spinner::EventKeyUp, CEGUI::Event::Subscriber( &cLevel_Settings::Update_BG_Image, this ) );
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_bg_image_speed_y" ));
	spinner->subscribeEvent( CEGUI::Spinner::EventKeyUp, CEGUI::Event::Subscriber( &cLevel_Settings::Update_BG_Image, this ) );
	// position
	editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_image_posx" ));
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cLevel_Settings::Update_BG_Image, this ) );
	editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_image_posy" ));
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cLevel_Settings::Update_BG_Image, this ) );
	editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_image_posz" ));
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cLevel_Settings::Update_BG_Image, this ) );
	// constant velocity
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_bg_image_const_vel_x" ));
	spinner->subscribeEvent( CEGUI::Spinner::EventKeyUp, CEGUI::Event::Subscriber( &cLevel_Settings::Update_BG_Image, this ) );
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_bg_image_const_vel_y" ));
	spinner->subscribeEvent( CEGUI::Spinner::EventKeyUp, CEGUI::Event::Subscriber( &cLevel_Settings::Update_BG_Image, this ) );
	// Gradient colors
	m_bg_color_1 = Color( m_level->m_background_manager->Get_Pointer(0)->m_color_1.red, m_level->m_background_manager->Get_Pointer(0)->m_color_1.green, m_level->m_background_manager->Get_Pointer(0)->m_color_1.blue, 255 );
	m_bg_color_2 = Color( m_level->m_background_manager->Get_Pointer(0)->m_color_2.red, m_level->m_background_manager->Get_Pointer(0)->m_color_2.green, m_level->m_background_manager->Get_Pointer(0)->m_color_2.blue, 255 );

	// color 1
	editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_color_start_red" ));
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( m_bg_color_1.red ).c_str() );
	editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_color_start_green" ));
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( m_bg_color_1.green ).c_str() );
	editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_color_start_blue" ));
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( m_bg_color_1.blue ).c_str() );
	// color 2
	editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_color_end_red" ));
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( m_bg_color_2.red ).c_str() );
	editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_color_end_green" ));
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( m_bg_color_2.green ).c_str() );
	editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_color_end_blue" ));
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( m_bg_color_2.blue ).c_str() );
	// preview window
	CEGUI::Window *window_background_preview = CEGUI::WindowManager::getSingleton().getWindow( "window_background_preview" );
	m_background_preview->Set_Pos_X( window_background_preview->getUnclippedOuterRect().d_left * global_downscalex, 1 );
	m_background_preview->Set_Pos_Y( window_background_preview->getUnclippedOuterRect().d_top * global_downscaley, 1 );

	Update_BG_Colors( CEGUI::EventArgs() );

	// global effect
	// filename
	editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_global_effect_file" ));
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cLevel_Settings::Update_Global_Effect_Image, this ) );
	// type
	combobox = static_cast<CEGUI::Combobox *>(CEGUI::WindowManager::getSingleton().getWindow( "combo_global_effect_type" ));
	combobox->addItem( new CEGUI::ListboxTextItem( "Disabled" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "Default" ) );
	// Z Position
	editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_global_effect_pos_z" ));
	editbox->setText( float_to_string( m_level->m_global_effect->m_pos_z ) );
	editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_global_effect_pos_z_rand" ));
	editbox->setText( float_to_string( m_level->m_global_effect->posz_rand ) );
	// creation rect
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_rect_x" ));
	spinner->setCurrentValue( m_level->m_global_effect->m_start_pos_x );
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_rect_y" ));
	spinner->setCurrentValue( m_level->m_global_effect->m_start_pos_y );
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_rect_w" ));
	spinner->setCurrentValue( m_level->m_global_effect->m_rect.m_w );
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_rect_h" ));
	spinner->setCurrentValue( m_level->m_global_effect->m_rect.m_h );
	// time to live
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_time_to_live" ));
	spinner->setCurrentValue( m_level->m_global_effect->time_to_live );
	// scale
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_scale" ));
	spinner->setCurrentValue( m_level->m_global_effect->size_scale );
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_scale_rand" ));
	spinner->setCurrentValue( m_level->m_global_effect->size_scale_rand );
	// emitter iteration interval
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_emitter_iteration_interval" ));
	spinner->setCurrentValue( m_level->m_global_effect->emitter_iteration_interval );
	// velocity
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_vel" ));
	spinner->setCurrentValue( m_level->m_global_effect->vel );
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_vel_rand" ));
	spinner->setCurrentValue( m_level->m_global_effect->vel_rand );
	// angle
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_angle_start" ));
	spinner->setCurrentValue( m_level->m_global_effect->angle_start );
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_angle_range" ));
	spinner->setCurrentValue( m_level->m_global_effect->angle_range );
	// rotation z
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_rot_z" ));
	spinner->setCurrentValue( m_level->m_global_effect->m_rot_z );
	// constant rotation z
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_const_rot_z" ));
	spinner->setCurrentValue( m_level->m_global_effect->const_rotz );
	spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_const_rot_z_rand" ));
	spinner->setCurrentValue( m_level->m_global_effect->const_rotz_rand );

	Clear_Layer_Field();

	Load_BG_Image_List();
	Load_Global_Effect();

	Update_Global_Effect_Image( CEGUI::EventArgs() );
}

void cLevel_Settings :: Enter( void )
{
	// set active camera
	pActive_Camera = m_camera;

	editor_enabled = 0;

	if( pMouseCursor->m_active_object )
	{
		pMouseCursor->Clear_Active_Object();
	}

	// Initialize level data
	Init();
	// set active
	m_active = 1;

	// update camera
	m_camera->Update_Position();
}

void cLevel_Settings :: Leave( void )
{
	// # Main Tab
	// filename
	if( Trim_Filename( m_level->m_level_filename, 0, 0 ).compare( CEGUI::WindowManager::getSingleton().getWindow( "editbox_level_filename" )->getText().c_str() ) != 0 )
	{
		m_level->Set_Levelfile( CEGUI::WindowManager::getSingleton().getWindow( "editbox_level_filename" )->getText().c_str() );
		// show no level saved info text
		pHud_Debug->Set_Text( "", 0 );
	}
	// musicfile
	m_level->Set_Musicfile( CEGUI::WindowManager::getSingleton().getWindow( "editbox_music_filename" )->getText().c_str() );
	// Author
	m_level->Set_Author( CEGUI::WindowManager::getSingleton().getWindow( "editbox_author" )->getText().c_str() );
	// Version
	m_level->Set_Version( CEGUI::WindowManager::getSingleton().getWindow( "editbox_version" )->getText().c_str() );
	// Camera Limits
	pLevel_Manager->m_camera->Set_Limit_W( (static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_camera_limit_w" )))->getCurrentValue() );
	pLevel_Manager->m_camera->Set_Limit_H( (static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_camera_limit_h" )))->getCurrentValue() );
	m_level->m_camera_limits.m_w = pLevel_Manager->m_camera->m_limit_rect.m_w;
	m_level->m_camera_limits.m_h = pLevel_Manager->m_camera->m_limit_rect.m_h;
	// fixed camera horizontal velocity
	pLevel_Manager->m_camera->m_fixed_hor_vel = (static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_camera_hor_vel" )))->getCurrentValue();
	m_level->m_fixed_camera_hor_vel = pLevel_Manager->m_camera->m_fixed_hor_vel;

	// # Background Tab
	// Gradient
	m_level->m_background_manager->Get_Pointer(0)->Set_Color_1( m_bg_color_1 );
	m_level->m_background_manager->Get_Pointer(0)->Set_Color_2( m_bg_color_2 );

	// # Global Effect Tab
	m_level->m_global_effect->m_image_filename = CEGUI::WindowManager::getSingleton().getWindow( "editbox_global_effect_file" )->getText().c_str();
	m_level->m_global_effect->Set_Pos_Z( string_to_float( ( static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_global_effect_pos_z" )))->getText().c_str() ), string_to_float( ( static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_global_effect_pos_z_rand" )))->getText().c_str() ) );
	m_level->m_global_effect->Set_Type( CEGUI::WindowManager::getSingleton().getWindow( "combo_global_effect_type" )->getText().c_str() );
	m_level->m_global_effect->Init_Anim();
	// creation rect
	CEGUI::Spinner *spinner_rect_x = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_rect_x" ));
	CEGUI::Spinner *spinner_rect_y = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_rect_y" ));
	CEGUI::Spinner *spinner_rect_w = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_rect_w" ));
	CEGUI::Spinner *spinner_rect_h = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_rect_h" ));
	m_level->m_global_effect->Set_Emitter_Rect( spinner_rect_x->getCurrentValue(), spinner_rect_y->getCurrentValue(), spinner_rect_w->getCurrentValue(), spinner_rect_h->getCurrentValue() );
	// lifetime
	CEGUI::Spinner *spinner_time_to_live = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_time_to_live" ));
	m_level->m_global_effect->Set_Time_to_Live( spinner_time_to_live->getCurrentValue() );
	// scale
	CEGUI::Spinner *spinner_scale = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_scale" ));
	CEGUI::Spinner *spinner_scale_rand = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_scale_rand" ));
	m_level->m_global_effect->Set_Scale( spinner_scale->getCurrentValue(), spinner_scale_rand->getCurrentValue() );
	// Emitter Iteration Interval
	CEGUI::Spinner *spinner_emitter_iteration_interval = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_emitter_iteration_interval" ));
	m_level->m_global_effect->Set_Emitter_Iteration_Interval( spinner_emitter_iteration_interval->getCurrentValue() );
	// velocity
	CEGUI::Spinner *spinner_vel = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_vel" ));
	CEGUI::Spinner *spinner_vel_rand = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_vel_rand" ));
	m_level->m_global_effect->Set_Speed( spinner_vel->getCurrentValue(), spinner_vel_rand->getCurrentValue() );
	// direction
	CEGUI::Spinner *spinner_angle_start = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_angle_start" ));
	CEGUI::Spinner *spinner_angle_range = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_angle_range" ));
	m_level->m_global_effect->Set_Direction_Range( spinner_angle_start->getCurrentValue(), spinner_angle_range->getCurrentValue() );
	// rotation z
	CEGUI::Spinner *spinner_rot_z = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_rot_z" ));
	m_level->m_global_effect->Set_Rotation_Z( spinner_rot_z->getCurrentValue(), 1 );
	// constant rotation z
	CEGUI::Spinner *spinner_const_rot_z = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_const_rot_z" ));
	CEGUI::Spinner *spinner_const_rot_z_rand = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_global_effect_const_rot_z_rand" ));
	m_level->m_global_effect->Set_Const_Rotation_Z( spinner_const_rot_z->getCurrentValue(), spinner_const_rot_z_rand->getCurrentValue() );
}

void cLevel_Settings :: Unload( void )
{
	if( !m_gui_window )
	{
		return;
	}
	
	// destroy CEGUI window
	pGuiSystem->getGUISheet()->removeChildWindow( m_gui_window );
	CEGUI::WindowManager::getSingleton().destroyWindow( m_gui_window );
	m_gui_window = NULL;
	m_tabcontrol = NULL;

	// clear preview images
	m_background_preview->Set_Image( NULL, 1 );
	m_global_effect_preview->Set_Image( NULL, 1 );

	m_active = 0;
}

void cLevel_Settings :: Update( void )
{
	// uhm...

	// update performance timer
	pFramerate->m_perf_timer[PERF_UPDATE_LEVEL_SETTINGS]->Update();
}

void cLevel_Settings :: Draw( void )
{
	pVideo->Clear_Screen();
	pVideo->Draw_Rect( NULL, 0.00001f, &black );

	// background Tab
	if( m_tabcontrol->getSelectedTabIndex() == 1 )
	{
		// create request
		cGradient_Request *gradient_request = new cGradient_Request();
		// draw background gradient
		pVideo->Draw_Gradient( &m_background_preview->m_rect, 0.0001f, &m_bg_color_1, &m_bg_color_2, DIR_VERTICAL, gradient_request );
		// scale with image
		if( m_background_preview->m_image )
		{
			// set scale
			gradient_request->rect.m_w *= m_background_preview->m_scale_x;
			gradient_request->rect.m_h *= m_background_preview->m_scale_y;
		}
		// add request
		pRenderer_GUI->Add( gradient_request );

		// create request
		cSurface_Request *request = new cSurface_Request();
		// draw background image preview
		m_background_preview->Draw( request );
		// add request
		pRenderer_GUI->Add( request );
	}
	// Global Effect Tab
	else if( m_tabcontrol->getSelectedTabIndex() == 2 )
	{
		// create request
		cSurface_Request *request = new cSurface_Request();
		// draw global effect preview
		m_global_effect_preview->Draw( request );
		// add request
		pRenderer_GUI->Add( request );
	}

	// update performance timer
	pFramerate->m_perf_timer[PERF_DRAW_LEVEL_SETTINGS]->Update();
}

bool cLevel_Settings :: Key_Down( SDLKey key )
{
	if( !m_active )
	{
		return 0;
	}

	if( key == SDLK_ESCAPE )
	{
		// back to level mode
		Game_Action = GA_ENTER_LEVEL;
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

void cLevel_Settings :: Set_Level( cLevel *level )
{
	m_level = level;
}

void cLevel_Settings :: Set_Sprite_Manager( cSprite_Manager *sprite_manager )
{
	m_background_preview->Set_Sprite_Manager( sprite_manager );
	m_global_effect_preview->Set_Sprite_Manager( sprite_manager );
	m_camera->Set_Sprite_Manager( sprite_manager );
}

bool cLevel_Settings :: Add_Background_Image( const CEGUI::EventArgs &event )
{
	cBackground *background = new cBackground( m_level->m_sprite_manager );
	background->Set_Type( BG_IMG_BOTTOM );
	background->Set_Image( LEVEL_DEFAULT_BACKGROUND );

	m_level->m_background_manager->Add( background );

	Load_BG_Image_List();

	return 1;
}

bool cLevel_Settings :: Delete_Background_Image( const CEGUI::EventArgs &event )
{
	CEGUI::Listbox *listbox = static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_backgrounds" ));
	CEGUI::ListboxItem *item = listbox->getFirstSelectedItem();

	if( !item )
	{
		return 1;
	}

	// get background
	cBackground *background = static_cast<cBackground *>(item->getUserData());
	// delete it
	m_level->m_background_manager->Delete( background );

	// update list
	Load_BG_Image_List();
	// clear background image gui
	Update_BG_Image( CEGUI::EventArgs() );

	return 1;
}

bool cLevel_Settings :: Set_Background_Image( const CEGUI::EventArgs &event )
{
	CEGUI::Listbox *listbox = static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_backgrounds" ));
	CEGUI::ListboxItem *item = listbox->getFirstSelectedItem();

	// selected
	if( item )
	{
		// get background
		cBackground *background = static_cast<cBackground *>(item->getUserData());
		std::string background_filename = background->m_image_1_filename;

		// type
		CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "combo_bg_image_type" ));
		editbox->setText( reinterpret_cast<const CEGUI::utf8*>(background->Get_Type_Name().c_str()) );
		// filename
		editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_image_name" ));
		editbox->setText( background_filename.c_str() );
		// position
		editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_image_posx" ));
		editbox->setText( float_to_string( background->m_start_pos_x ).c_str() );
		editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_image_posy" ));
		editbox->setText( float_to_string( background->m_start_pos_y ).c_str() );
		editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_image_posz" ));
		editbox->setText( float_to_string( background->m_pos_z ).c_str() );
		// speed
		CEGUI::Spinner *spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_bg_image_speed_x" ));
		spinner->setCurrentValue( background->m_speed_x );
		spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_bg_image_speed_y" ));
		spinner->setCurrentValue( background->m_speed_y );
		// constant velocity
		spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_bg_image_const_vel_x" ));
		spinner->setCurrentValue( background->m_const_vel_x );
		spinner = static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_bg_image_const_vel_y" ));
		spinner->setCurrentValue( background->m_const_vel_y );

		// set image preview
		background_filename.insert( 0, DATA_DIR "/" GAME_PIXMAPS_DIR "/" );
		Set_Background_Image_Preview( background_filename );
	}
	// deselected
	else
	{
		Clear_Layer_Field();
	}

	return 1;
}

bool cLevel_Settings :: Button_Save( const CEGUI::EventArgs &event )
{
	// back to level mode
	Game_Action = GA_ENTER_LEVEL;
	return 1;
}

bool cLevel_Settings :: Update_BG_Colors( const CEGUI::EventArgs &event )
{
	CEGUI::Editbox *color_start_red = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_color_start_red" ));
	CEGUI::Editbox *color_start_green = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_color_start_green" ));
	CEGUI::Editbox *color_start_blue = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_color_start_blue" ));
	CEGUI::Editbox *color_end_red = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_color_end_red" ));
	CEGUI::Editbox *color_end_green = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_color_end_green" ));
	CEGUI::Editbox *color_end_blue = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_color_end_blue" ));

	m_bg_color_1.red = string_to_int( color_start_red->getText().c_str() );
	m_bg_color_1.green = string_to_int( color_start_green->getText().c_str() );
	m_bg_color_1.blue = string_to_int( color_start_blue->getText().c_str() );
	m_bg_color_2.red = string_to_int( color_end_red->getText().c_str() );
	m_bg_color_2.green = string_to_int( color_end_green->getText().c_str() );
	m_bg_color_2.blue = string_to_int( color_end_blue->getText().c_str() );


	// color start
	color_start_red->setProperty( "NormalTextColour", CEGUI::PropertyHelper::colourToString( CEGUI::colour( 1, 1 - ( static_cast<float>(m_bg_color_1.red) / 255 ), 1 - ( static_cast<float>(m_bg_color_1.red) / 255 ), 1 ) ) );
	color_start_green->setProperty( "NormalTextColour", CEGUI::PropertyHelper::colourToString( CEGUI::colour( 1 - ( static_cast<float>(m_bg_color_1.green) / 255 ), 1, 1 - ( static_cast<float>(m_bg_color_1.green) / 255 ), 1 ) ) );
	color_start_blue->setProperty( "NormalTextColour", CEGUI::PropertyHelper::colourToString( CEGUI::colour( 1 - ( static_cast<float>(m_bg_color_1.blue) / 255 ), 1 - ( static_cast<float>(m_bg_color_1.blue) / 255 ), 1, 1 ) ) );
	// color end
	color_end_red->setProperty( "NormalTextColour", CEGUI::PropertyHelper::colourToString( CEGUI::colour( 1, 1 - ( static_cast<float>(m_bg_color_2.red) / 255 ), 1 - ( static_cast<float>(m_bg_color_2.red) / 255 ), 1 ) ) );
	color_end_green->setProperty( "NormalTextColour", CEGUI::PropertyHelper::colourToString( CEGUI::colour( 1 - ( static_cast<float>(m_bg_color_2.green) / 255 ), 1, 1 - ( static_cast<float>(m_bg_color_2.green) / 255 ), 1 ) ) );
	color_end_blue->setProperty( "NormalTextColour", CEGUI::PropertyHelper::colourToString( CEGUI::colour( 1 - ( static_cast<float>(m_bg_color_2.blue) / 255 ), 1 - ( static_cast<float>(m_bg_color_2.blue) / 255 ), 1, 1 ) ) );

	CEGUI::Window *statictext = CEGUI::WindowManager::getSingleton().getWindow( "text_color_start" );
	statictext->setProperty( "TextColours", CEGUI::PropertyHelper::colourToString( m_bg_color_1.Get_cegui_Color() ) );
	statictext = CEGUI::WindowManager::getSingleton().getWindow( "text_color_end" );
	statictext->setProperty( "TextColours", CEGUI::PropertyHelper::colourToString( m_bg_color_2.Get_cegui_Color() ) );

	return 1;
}

void cLevel_Settings :: Load_BG_Image_List( void )
{
	CEGUI::Listbox *listbox = static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_backgrounds" ));
	listbox->resetList();

	for( vector<cBackground *>::iterator itr = m_level->m_background_manager->objects.begin(); itr != m_level->m_background_manager->objects.end(); ++itr )
	{
		cBackground *background = (*itr);

		// skip gradients
		if( background->m_type == BG_GR_HOR || background->m_type == BG_GR_VER )
		{
			continue;
		}

		CEGUI::ListboxTextItem *item = new CEGUI::ListboxTextItem( float_to_string( background->m_pos_z ).c_str(), 0, background );
		item->setSelectionColours( CEGUI::colour( 0.33f, 0.33f, 0.33f ) );
		item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
		listbox->addItem( static_cast<CEGUI::ListboxItem *>(item) );
	}

	CEGUI::PushButton *button_add = static_cast<CEGUI::PushButton *>(CEGUI::WindowManager::getSingleton().getWindow( "button_add_background_image" ));
	
	// 9 layers + default background is the maximum
	if( m_level->m_background_manager->size() >= 10 )
	{
		button_add->disable();
	}
	else
	{
		button_add->enable();
	}
}

void cLevel_Settings :: Set_Background_Image_Preview( std::string filename )
{
	Convert_Path_Separators( filename );

	// unset image
	m_background_preview->Set_Image( NULL );

	// set default rect
	CEGUI::Window *window_background_preview = CEGUI::WindowManager::getSingleton().getWindow( "window_background_preview" );
	m_background_preview->m_rect.m_w = window_background_preview->getUnclippedOuterRect().getWidth() * global_downscalex;
	m_background_preview->m_rect.m_h = window_background_preview->getUnclippedOuterRect().getHeight() * global_downscaley;

	if( !File_Exists( filename ) )
	{
		return;
	}

	cGL_Surface *temp = pVideo->Get_Surface( filename );
	
	if( !temp )
	{
		return;
	}

	// reset scale
	m_background_preview->m_scale_x = 1.0f;
	m_background_preview->m_scale_y = 1.0f;
	// Get zoom before setting image
	float zoom = pVideo->Get_Scale( temp, m_background_preview->m_rect.m_w, m_background_preview->m_rect.m_h );
	// Set image
	m_background_preview->Set_Image( temp );
	// Set Zoom
	m_background_preview->m_scale_x *= zoom;
	m_background_preview->m_scale_y *= zoom;
}

bool cLevel_Settings :: Update_BG_Image( const CEGUI::EventArgs &event )
{
	CEGUI::Listbox *listbox = static_cast<CEGUI::Listbox *>(CEGUI::WindowManager::getSingleton().getWindow( "listbox_backgrounds" ));
	CEGUI::ListboxItem *item = listbox->getFirstSelectedItem();

	// clear
	if( !item )
	{
		Clear_Layer_Field();
		return 1;
	}

	std::string bg_type = CEGUI::WindowManager::getSingleton().getWindow( "combo_bg_image_type" )->getText().c_str();
	std::string bg_filename = CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_image_name" )->getText().c_str();
	float posx = string_to_float( ( static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_image_posx" )))->getText().c_str() );
	float posy = string_to_float( ( static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_image_posy" )))->getText().c_str() );
	float posz = string_to_float( ( static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_image_posz" )))->getText().c_str() );
	float speed_x = (static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_bg_image_speed_x" )))->getCurrentValue();
	float speed_y = (static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_bg_image_speed_y" )))->getCurrentValue();
	float const_vel_x = (static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_bg_image_const_vel_x" )))->getCurrentValue();
	float const_vel_y = (static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_bg_image_const_vel_y" )))->getCurrentValue();

	// full filename for validation
	bg_filename.insert( 0, DATA_DIR "/" GAME_PIXMAPS_DIR "/" );

	// get background
	cBackground *background = static_cast<cBackground *>(item->getUserData());
	// set type
	background->Set_Type( bg_type );
	// set position
	background->Set_Start_Pos( posx, posy );
	// set position z
	background->Set_Pos_Z( posz );
	// set scroll speed
	background->Set_Scroll_Speed( speed_x, speed_y );
	// set constant velocity
	background->Set_Const_Velocity_X( const_vel_x );
	background->Set_Const_Velocity_Y( const_vel_y );


	// valid
	if( File_Exists( bg_filename ) )
	{
		// update image preview
		Set_Background_Image_Preview( bg_filename );
	}
	// invalid
	else
	{
		// clear image
		bg_filename.clear();
	}

	// set image
	background->Set_Image( bg_filename );
	// set new item name
	item->setText( float_to_string( posz ).c_str() );
	// fixme : should update sorting because of the new name
	listbox->handleUpdatedItemData();

	return 1;
}

void cLevel_Settings :: Clear_Layer_Field( void )
{
	(static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "combo_bg_image_type" )))->setText( "Disabled" );
	(static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_image_name" )))->setText( "" );
	(static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_image_posx" )))->setText( "" );
	(static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_image_posy" )))->setText( "" );
	(static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_bg_image_posz" )))->setText( "" );
	(static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_bg_image_speed_x" )))->setText( "" );
	(static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_bg_image_speed_y" )))->setText( "" );
	(static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_bg_image_const_vel_x" )))->setText( "" );
	(static_cast<CEGUI::Spinner *>(CEGUI::WindowManager::getSingleton().getWindow( "spinner_bg_image_const_vel_y" )))->setText( "" );
	Set_Background_Image_Preview( "" );
}

void cLevel_Settings :: Load_Global_Effect( void )
{
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "editbox_global_effect_file" ));
	editbox->setText( m_level->m_global_effect->m_image_filename.c_str() );

	editbox = static_cast<CEGUI::Editbox *>(CEGUI::WindowManager::getSingleton().getWindow( "combo_global_effect_type" ));
	editbox->setText( m_level->m_global_effect->Get_Type_Name().c_str() );
}

void cLevel_Settings :: Set_Global_Effect_Image_Preview( std::string filename )
{
	Convert_Path_Separators( filename );

	if( !File_Exists( filename ) )
	{
		m_global_effect_preview->Set_Image( NULL );
		return;
	}

	cGL_Surface *temp = pVideo->Get_Surface( filename );
	
	if( !temp )
	{
		return;
	}

	// reset scale
	m_global_effect_preview->m_scale_x = 1.0f;
	m_global_effect_preview->m_scale_y = 1.0f;
	// Set image
	m_global_effect_preview->Set_Image( temp );
	// Set Zoom
	float zoom = pVideo->Get_Scale( temp, 100.0f, 100.0f );
	m_global_effect_preview->m_scale_x *= zoom;
	m_global_effect_preview->m_scale_y *= zoom;
}

bool cLevel_Settings :: Update_Global_Effect_Image( const CEGUI::EventArgs &event )
{
	std::string ge_filename = CEGUI::WindowManager::getSingleton().getWindow( "editbox_global_effect_file" )->getText().c_str();

	// image preview
	ge_filename.insert( 0, DATA_DIR "/" GAME_PIXMAPS_DIR "/" );
	Set_Global_Effect_Image_Preview( ge_filename );

	return 1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
