    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
   
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.


	Some inspiration taken from glib.
	Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
	Modified by the GLib Team and others 1997-1999.

    */

#ifndef _ARTSDEBUG_H_
#define _ARTSDEBUG_H_

/*
 * BC - Status (2000-09-30): Debug.
 *
 * Collection class, no instance, no members. Thus binary compatible (will
 * be kept).
 */

#define arts_fatal		::Arts::Debug::fatal
#define arts_warning	::Arts::Debug::warning
#define arts_info		::Arts::Debug::info
#define arts_debug		::Arts::Debug::debug

/* source compatibility with older sources */
#define artsdebug		::Arts::Debug::debug
#define setartsdebug(x)	arts_warning("setartsdebug is obsolete")

#ifdef __GNUC__

#define arts_return_if_fail(expr)										\
	 do {																\
       if (!(expr))                       								\
	   {																\
         arts_warning ("file %s: line %d (%s): assertion failed: (%s)", \
          __FILE__, __LINE__, __PRETTY_FUNCTION__, #expr);				\
	     return;														\
	   }																\
	 } while(0)

#define arts_return_val_if_fail(expr,val)								\
	 do {																\
       if (!(expr))                       								\
	   {																\
         arts_warning ("file %s: line %d (%s): assertion failed: (%s)", \
            __FILE__, __LINE__, __PRETTY_FUNCTION__, #expr);			\
	     return (val);													\
	   }																\
	 } while(0)

#define arts_assert(expr)												\
	 do {																\
       if (!(expr))                       								\
         arts_fatal ("file %s: line %d (%s): assertion failed: (%s)",  	\
            __FILE__, __LINE__, __PRETTY_FUNCTION__, #expr);			\
	 } while(0)

#else

#define arts_return_if_fail(expr)										\
	 do {																\
       if (!(expr))                       								\
	   {																\
         arts_warning ("file %s: line %d: assertion failed: (%s)",  	\
            __FILE__, __LINE__, #expr);									\
	     return;														\
	   }
	 } while(0)

#define arts_return_val_if_fail(expr,val)								\
	 do {																\
       if (!(expr))                       								\
	   {																\
         arts_warning ("file %s: line %d: assertion failed: (%s)", 	 	\
            __FILE__, __LINE__, #expr);									\
	     return (val);													\
	   }																\
	 } while(0)

#define arts_assert(expr)												\
	 do {																\
       if (!(expr))                       								\
         arts_fatal ("file %s: line %d: assertion failed: (%s)",  		\
            __FILE__, __LINE__, #expr);									\
	 } while(0)

#endif

namespace Arts {
	class Debug {
	public:
		enum Level { lFatal = 3, lWarning = 2, lInfo = 1, lDebug = 0 };

		/**
		 * Initializes at which is the minimum level to react to. If you
		 * call this, call this before creating the Arts::Dispatcher object.
		 */
		static void init(const char *prefix, Level level);

		static void fatal(const char *fmt,...);		// print on stderr & abort
		static void warning(const char *fmt,...);	// print on stderr
		static void info(const char *fmt,...);		// print on stdout
		static void debug(const char *fmt,...);		// print on stdout

		/**
 		 * This method sets the name of an external application to
		 * display messages graphically.
 		 */
		static void messageApp(const char *appName);

		static void initMutex();	// called from the dispatcher constructor
		static void freeMutex();	// called from the dispatcher destructor
	};
};

#endif
