    /*

    Copyright (C) 1999 Stefan Westerfeld
                       stefan@space.twc.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */

#include "mcoputils.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
	
string MCOPUtils::createFilePath(string name)
{
	string logname = "unknown";
	if(getenv("LOGNAME")) logname = getenv("LOGNAME");

	string tmpdir = "/tmp/mcop-"+logname;
	mkdir(tmpdir.c_str(),0700);
	chmod(tmpdir.c_str(),0700);

	string::iterator si;
	for(si = name.begin(); si != name.end(); si++)
	{
		unsigned char c = (unsigned char)*si;
		if((c >= 'a' && c <= 'z') ||
		   (c >= 'A' && c <= 'Z') ||
		   (c >= '0' && c <= '9') ||
		   (c == '-'))
		{
			// ok, these are fine for our filesystem ;)
		}
		else
		{
			*si = '_';
		}
	}
	return tmpdir+"/"+name;
}
