/*  
    Copyright (C) 1997 Mark Donohoe (donohoe@kde.org)
    Copyright (C) 2000 Stefan Westerfeld (stefan@space.twc.de)

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
*/

/*
 * This is based on Mark Donohoe's nice KRootProp class (found in
 * kdelibs/kdecore/krootprop.cpp). I couldn't use KRootProp directly, since
 * I wanted aRts (and even it's X11 support) to work without KDE and/or Qt
 * installed at all.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "x11globalcomm.h"
#include "debug.h"
#include <iostream>

using namespace std;
using namespace Arts;

class X11GlobalComm_impl : virtual public X11GlobalComm_skel {
protected:
	Display *X11display;
	Window rootWin;
	Atom atom;
	map<string,string> propertyMap;
	bool init;

	void warn(const char *what)
	{
		arts_warning("X11GlobalComm: %s", what);
	}
public:
	X11GlobalComm_impl() :rootWin(0), atom(0)
	{
		X11display = XOpenDisplay(NULL);
		if(!X11display)
		{
		  warn("Can't connect to the XServer - Initial references won't work.");
		  init = false;
		}
		else
		{
		  rootWin = DefaultRootWindow(X11display);
  		  atom = XInternAtom( X11display, "MCOPGLOBALS" , False);
		  init = true;
		}
	}

	~X11GlobalComm_impl()
	{
		if(X11display)
			XCloseDisplay(X11display);
	}

	void read()
	{
		if(!init) return;

		Atom type;
		int format;
  		unsigned long nitems;
  		unsigned long bytes_after;
  		long offset;
  		char *buf;
	
		propertyMap.clear();

  		// If a property has already been opened write
  		// the dictionary back to the root window

		string s;
  		offset = 0; bytes_after = 1;
  		while (bytes_after != 0)
  		{
			XGetWindowProperty( X11display, rootWin, atom, offset, 256,
                        False, XA_STRING, &type, &format, &nitems, &bytes_after,
                        (unsigned char **)&buf);

			if(type == None)	// no property -> no contents
				return;
		
    		s += buf;
    		offset += nitems/4;
    		if (buf)
      			XFree(buf);
  		}

  		// Parse through the property string stripping out key value pairs
  		// and putting them in the dictionary
		
  		while(s.length() >0 )
  		{
    		// parse the string for first key-value pair separator '\n'
    		int i = s.find("\n",0);
		
    		// extract the key-values pair and remove from string
			
    		string keypair = s.substr(0,i);
    		s = s.substr(i+1,s.size()-(i+1));
			
    		// split key and value and add to dictionary
			
    		i = keypair.find( "=" );
    		if( i != -1 )
    		{
      			string key = keypair.substr( 0, i );
      			string value = keypair.substr( i+1, keypair.size()-(i+1) );
				propertyMap[key] = value;
    		}
  		}
	}

	void write()
	{
		if(!init) return;

		// bring map in the variable=value form with \n as seperator
  		string propString;

    	map<string,string>::iterator i = propertyMap.begin();
    	for(i = propertyMap.begin(); i != propertyMap.end(); i++)
      		propString += i->first + "=" + i->second + "\n";

		// send to the X11 server
  		XChangeProperty( X11display, rootWin, atom,
                  XA_STRING, 8, PropModeReplace,
                  (const unsigned char *)propString.c_str(),
                  propString.size()+1);
  		XFlush(X11display);
	}

	bool put(const string& variable, const string& value)
	{
		if(!init) return false;

		read();
		if(propertyMap[variable] == "")
		{
			propertyMap[variable] = value;
			write();
			return true;
		}
		return false;
	}

	string get(const string& variable)
	{
		if(!init) return "";

		read();
		return propertyMap[variable];
	}

	void erase(const string& variable)
	{
		if(!init) return;

		read();
		propertyMap.erase(variable);
		write();
	}
};

REGISTER_IMPLEMENTATION(X11GlobalComm_impl);
