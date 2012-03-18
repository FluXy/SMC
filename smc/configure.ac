AC_PREREQ([2.57])
AC_INIT([smc], [2.0], , [smc], [http://www.secretmaryo.org/])
AC_CANONICAL_HOST
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/core/main.cpp])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.11 foreign])
AM_SILENT_RULES([yes])

### Check for programs ###

AC_PROG_CXX
AC_LANG([C++])
CXXFLAGS="${CXXFLAGS} -Wall"
AC_PROG_INSTALL
AC_PROG_MKDIR_P
PKG_PROG_PKG_CONFIG

### Check for libraries ###

# Check for the Boost Filesystem library
AC_CHECK_LIB([boost_filesystem], [main], ,
	[AC_MSG_ERROR([Unable to find Boost Filesystem library])])

# Check for the Boost Thread library
AC_CHECK_LIB([boost_thread], [main], ,
	[AC_MSG_ERROR([Unable to find Boost Thread library])])

# Check for the Boost System library
AC_CHECK_LIB([boost_system], [main], ,
	[AC_MSG_ERROR([Unable to find Boost System library])])

# Check for the OpenGL and GLU library
case "${host}" in
*darwin*|*macosx*)
	# Mac OS X
	LIBS="$LIBS -framework OpenGL -framework Carbon -lintl -lboost_system"
	;;
*)
	# Other Unix (Linux)
	AC_CHECK_LIB([X11], [main], ,
		[AC_MSG_ERROR([Unable to find libX11])])
	AC_CHECK_LIB([GL], [main], , [has_gl=no])
	AC_CHECK_LIB([GLU], [main], , [has_glu=no])
	if test "x$has_gl" = "xno" -o "x$has_glu" = "xno"; then
		AC_MSG_ERROR([Unable to find the OpenGL library])
	fi
	;;
esac

# Check for the SDL library
PKG_CHECK_MODULES([SDL], [sdl >= 1.2.10])
CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

# Check for the libpng library
AC_CHECK_LIB([png], [png_init_io], ,
	[AC_MSG_ERROR([libpng library not found])])

# Check for the SDL_image library
AC_CHECK_LIB([SDL_image], [IMG_LoadPNG_RW], ,
	[AC_MSG_ERROR([SDL_image library with PNG support not found])])
AC_CHECK_LIB([SDL_image], [IMG_Init], ,
	[AC_MSG_ERROR([SDL_image library must be >= 1.2.8])])

# Check for the SDL_mixer library
AC_CHECK_LIB([SDL_mixer], [Mix_OpenAudio], ,
	[AC_MSG_ERROR([SDL_mixer library not found])])

# Check for the SDL_ttf library
AC_CHECK_LIB([SDL_ttf], [TTF_Init], ,
	[AC_MSG_ERROR([SDL_ttf library not found])])

# Check for the CEGUI library
PKG_CHECK_MODULES([CEGUI_OPENGL], [CEGUI-OPENGL >= 0.7.2])
CPPFLAGS="$CPPFLAGS $CEGUI_OPENGL_CFLAGS"
LIBS="$LIBS $CEGUI_OPENGL_LIBS"
# CEGUI 0.7.5 does not install CEGUI-NULL.pc
#PKG_CHECK_MODULES([CEGUI_NULL], [CEGUI-NULL >= 0.7.2])
#CPPFLAGS="$CPPFLAGS $CEGUI_NULL_CFLAGS"
#LIBS="$LIBS $CEGUI_NULL_LIBS"
AC_CHECK_LIB([CEGUINullRenderer], [main], ,
	[AC_MSG_ERROR([CEGUINullRenderer library not found - try building CEGUI with --enable-null-renderer])])

# Check for the gettext library
AM_GNU_GETTEXT([external])

### Check for header files ###

AC_CHECK_HEADERS([SDL.h], ,
	[AC_MSG_ERROR([SDL include files not found. The SDL development package is needed.])])

AC_CHECK_HEADERS([SDL_image.h], ,
	[AC_MSG_ERROR([SDL_image include files not found. The SDL_image development package is needed.])])

AC_CHECK_HEADERS([SDL_mixer.h], ,
	[AC_MSG_ERROR([SDL_mixer include files not found. The SDL_mixer development package is needed.])])

AC_CHECK_HEADERS([SDL_ttf.h], ,
	[AC_MSG_ERROR([SDL_ttf include files not found. The SDL_ttf development package is needed.])])

### Option to enable debug build ###
AC_ARG_ENABLE([debug],
	[AC_HELP_STRING([--enable-debug], [enable debug build])],
	[CPPFLAGS="$CPPFLAGS -D_DEBUG=1"])

### Allow running in-place for development ###
AC_ARG_ENABLE([runinplace],
	[AC_HELP_STRING([--enable-runinplace], [build to run inplace])],
	[runinplace=yes])
AM_CONDITIONAL([RUNINPLACE], [test "$runinplace" = yes])

### Create files ###

AC_CONFIG_FILES([Makefile src/Makefile data/Makefile])
AC_OUTPUT
