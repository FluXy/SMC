/***************************************************************************
 * global_basic.h
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

#ifndef SMC_GLOBAL_BASIC_H
#define SMC_GLOBAL_BASIC_H

/* uncomment to enable experimental rendering in a thread
* still randomly crashes because gl functions are called while the render thread is active
*/
//#define SMC_RENDER_THREAD_TEST

/* *** *** *** *** *** *** *** Debugging *** *** *** *** *** *** *** *** *** *** */

#ifdef _WIN32
	#ifndef __WIN32__
		#define __WIN32__
	#endif
	#ifdef _DEBUG
		// disable possible loss of data
		#pragma warning ( disable : 4267 )
	#endif
#endif

// debug printf macro
#ifdef _DEBUG
	#define debug_print(format, ...) printf(format, ##__VA_ARGS__)
#else
	#define debug_print(format, ...)
#endif

/* *** *** *** *** *** *** *** Standard setup *** *** *** *** *** *** *** *** *** *** */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <time.h>
#include <math.h>

using std::vector;
using std::ifstream;
using std::fstream;
using std::ofstream;
using std::stringstream;
using std::ios;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
