/* -*- C++ -*-
 * This header tries to include all used STL header files 
 * and to resolve the usual problems concerning the order
 * of inclusion. Additionally it tries to resolve the 
 * differences that matter in kab between the HP STL shipped
 * with g++ 2.7.2 and the SGI STL shipped with later 
 * versions.
 * 
 * the KDE addressbook
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class, Qt > 1.40
 *             NANA (for debugging)
 * $Revision$
 */

#ifndef MS_STL_HEADERS_H
#define MS_STL_HEADERS_H

#include <string>
#include <list>

#include <config.h>

/* The map-header is needed, but declares two global 
 * symbols that are also globally declared in Qt, "red"
 * and "black", in the HP version.
 * The fix has been delivered by Troll Tech, thanks.
 * 
 * The string class provided by the two STL versions have 
 * similar methods for deleting characters out of the 
 * strings, but one calls them "remove" (HP) and one calls 
 * them "erase" (SGI). kab uses the macro "ERASE" that is
 * set to the right value here. 
 *
 * I had to add an error message here if none of the versions
 * is selected since most people missed to read the FAQ...
 */

// --------------------
// #define HAVE_SGI_STL
// #define HAVE_HP_STL
// ^^^^^^^^^^^^^^^^^^^^

#if !defined HAVE_HP_STL && !defined HAVE_SGI_STL
#error The STL version has not been selected. You need to \
choose the STL version you use manually. \
Read the file COMPILE_PROBLEMS_FAQ.
#endif

/* This is to resolve the multiple definitions of the global 
 * identifiers "red" and "black" in the Qt and STL sources.
 */
#define red stl_red
#define black stl_black
#include <map>
#undef red
#undef black

/* This is declared to resolve incompatibilities found 
 * between the HP STL delivered with gnu C++ <2.8 and the
 * SGI STL delivered with later versions and with egcs.
 */
#if defined HAVE_HP_STL
#define ERASE remove
#else
#define ERASE erase
#endif

#endif // MS_STL_HEADERS_H

