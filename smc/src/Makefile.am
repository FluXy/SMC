AM_CXXFLAGS = -Wall -Wpointer-arith

if RUNINPLACE
else
AM_CPPFLAGS = -DDATA_DIR="\"$(pkgdatadir)\""
endif

bin_PROGRAMS = smc

smc_SOURCES = \
	audio/audio.cpp \
	audio/random_sound.cpp \
	audio/sound_manager.cpp \
	core/camera.cpp \
	core/campaign_manager.cpp \
	core/collision.cpp \
	core/editor.cpp \
	core/filesystem/filesystem.cpp \
	core/filesystem/resource_manager.cpp \
	core/file_parser.cpp \
	core/framerate.cpp \
	core/game_core.cpp \
	core/i18n.cpp \
	core/main.cpp \
	core/math/vector.cpp \
	core/math/utilities.cpp \
	core/sprite_manager.cpp \
	enemies/bosses/turtle_boss.cpp \
	enemies/eato.cpp \
	enemies/enemy.cpp \
	enemies/gee.cpp \
	enemies/furball.cpp \
	enemies/flyon.cpp \
	enemies/krush.cpp \
	enemies/rokko.cpp \
	enemies/spika.cpp \
	enemies/spikeball.cpp \
	enemies/static.cpp \
	enemies/thromp.cpp \
	enemies/turtle.cpp \
	gui/generic.cpp \
	gui/hud.cpp \
	gui/menu.cpp \
	gui/menu_data.cpp \
	input/joystick.cpp \
	input/keyboard.cpp \
	input/mouse.cpp \
	level/level_background.cpp \
	level/level.cpp \
	level/level_editor.cpp \
	level/level_manager.cpp \
	level/level_player.cpp \
	level/level_settings.cpp \
	objects/animated_sprite.cpp \
	objects/ball.cpp \
	objects/bonusbox.cpp \
	objects/box.cpp \
	objects/enemystopper.cpp \
	objects/goldpiece.cpp \
	objects/level_entry.cpp \
	objects/level_exit.cpp \
	objects/moving_platform.cpp \
	objects/movingsprite.cpp \
	objects/path.cpp \
	objects/powerup.cpp \
	objects/spinbox.cpp \
	objects/sprite.cpp \
	objects/star.cpp \
	objects/text_box.cpp \
	overworld/world_layer.cpp \
	overworld/overworld.cpp \
	overworld/world_player.cpp \
	overworld/world_waypoint.cpp \
	overworld/world_editor.cpp \
	overworld/world_manager.cpp \
	overworld/world_sprite_manager.cpp \
	user/preferences.cpp \
	user/savegame.cpp \
	video/animation.cpp \
	video/font.cpp \
	video/gl_surface.cpp \
	video/img_manager.cpp \
	video/img_settings.cpp \
	video/renderer.cpp \
	video/video.cpp
