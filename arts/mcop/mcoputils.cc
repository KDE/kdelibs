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

    */

#include "mcoputils.h"
#include "mcopconfig.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <config.h>
#include <map>
	
using namespace std;
using namespace Arts;

string MCOPUtils::createFilePath(string name)
{
	string logname = "unknown";
	if(getenv("LOGNAME")) logname = getenv("LOGNAME");

	string tmpdir = "/tmp/mcop-"+logname;
	if(mkdir(tmpdir.c_str(),0700) != 0 && errno != EEXIST)
	{
		cerr << "MCOP error: can't create " << tmpdir <<
		        " (" << strerror(errno) << ")" << endl;
		exit(1);
	}

	/** check that /tmp/mcop-<username>/ is a secure temporary dir **/
	struct stat st;
	if(lstat(tmpdir.c_str(),&st) != 0)
	{
		cerr << "MCOP error: can't stat " << tmpdir <<
		        " (" << strerror(errno) << ")" << endl;
		exit(1);
	}
	if (st.st_uid != getuid ())
    {
		cerr << "MCOP error: " << tmpdir << " not owned by user" << endl;
		exit(1);
	}
	if(st.st_mode & 0077)
	{
		cerr << "MCOP error: " << tmpdir << " accessible by others" << endl;
		exit(1);
	}
	if(!S_ISDIR(st.st_mode))
	{
		cerr << "MCOP error: " << tmpdir << " is not a directory" << endl;
		exit(1);
	}

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

/*
 * try to figure out full hostname - this is important as every client which
 * tries to connect objects located here will need to be able to resolve that 
 * correctly
 */
string MCOPUtils::getFullHostname()
{
	char buffer[1024];
	string result;

	if(gethostname(buffer,1024) == 0)
		result = buffer; 
	else
		return "localhost";

	if(getdomainname(buffer,1024) == 0)
	{
		/*
		 * I don't know why, but on my linux machine, the domainname
		 * always ends up being (none), which is certainly no valid
		 * domainname
		 */
		if(strcmp(buffer,"(none)") != 0)
		{
			result += ".";
			result += buffer; 
		}
	}

	return result;
}

string MCOPUtils::readConfigEntry(const string& key, const string& defaultValue)
{
	const char *home = getenv("HOME");
	if(home)
	{
		string rcname = home + string("/.mcoprc");

		MCOPConfig config(rcname);
		return config.readEntry(key,defaultValue);
	}
	return defaultValue;
}

/** IID generation **/

unsigned long MCOPUtils::makeIID(const string& interfaceName)
{
	static map<string, unsigned long> *iidmapobj = 0;
	static unsigned long nextiid = 1;

	if(!iidmapobj) iidmapobj = new map<string,unsigned long>;

	map<string,unsigned long>& iidmap = *iidmapobj;

	if(!iidmap[interfaceName])
		iidmap[interfaceName] = nextiid++;

	return iidmap[interfaceName];
}
