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
#include "debug.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <config.h>
#include <ctype.h>
#include <map>
	
using namespace std;
using namespace Arts;

/*
 * Uglify does the following to strings:
 *  - keep [A-Za-z0-9-_] characters
 *  - transform all others in a mime-style hex code, that is for instance
 *    =3F for a '?' char.
 *
 * so uglify("Hello...?") will be "Hello=2E=2E=2E=3F". The resulting name
 * mangling is compatible with the CSL implementation.
 */ 

static string uglify(const string& ident)
{
	string result;

	for(string::const_iterator p = ident.begin(); p != ident.end(); p++)
	{
		if((*p >= 'a' && *p <= 'z') ||
		   (*p >= 'A' && *p <= 'Z') ||
		   (*p >= '0' && *p <= '9') ||
		    *p == '_' || *p == '-')
		{
			result += *p;
		}
		else
		{
			char hex[17] = "0123456789ABCDEF";

			result += '=';
			result += hex[(*p >> 4) & 0xf];
			result += hex[*p & 0xf];
		}
	}
	return result;
}

string MCOPUtils::createFilePath(string name)
{
	string logname = "unknown";
	if(getenv("LOGNAME")) logname = getenv("LOGNAME");

	string tmpdir = "/tmp/mcop-"+uglify(logname);
	if(mkdir(tmpdir.c_str(),0700) != 0 && errno != EEXIST)
		arts_fatal("can't create %s (%s)", tmpdir.c_str(),strerror(errno));

	/** check that /tmp/mcop-<username>/ is a secure temporary dir **/
	struct stat st;
	if(lstat(tmpdir.c_str(),&st) != 0)
		arts_fatal("can't stat %s (%s)", tmpdir.c_str(),strerror(errno));

	if (st.st_uid != getuid ())
		arts_fatal("%s is not owned by user", tmpdir.c_str());

	if(st.st_mode & 0077)
		arts_fatal("%s is accessible owned by user", tmpdir.c_str());

	if(!S_ISDIR(st.st_mode))
		arts_fatal("%s is not a directory", tmpdir.c_str());

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
	struct hostent *hp;

	if(gethostname(buffer,1024) == 0)
		result = buffer; 
	else
		return "localhost";

	/*
	 * if gethostname() isn't a FQDN (guess that by checking whether it
	 * contains a dot), try to look it up to ensure it is
	 */
	if(result.find('.') == string::npos && (hp = gethostbyname(buffer)) != 0)
		result = hp->h_name;
	
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

static vector<string> *readPath(const string& name, const string& defaultValue)
{
	vector<string> *result = 0;
	const char *home = getenv("HOME");

	if(home)
	{
		string rcname = home + string("/.mcoprc");

		MCOPConfig config(rcname);
		result = config.readListEntry(name);
	}
	if(!result)
		result = new vector<string>;
	if(result->empty())
		result->push_back(defaultValue);

	return result;
}

const vector<string> *MCOPUtils::extensionPath()
{
	static vector<string> *result = 0;

	if(!result) result = readPath("ExtensionPath", EXTENSION_DIR);
	return result;
}

const vector<string> *MCOPUtils::traderPath()
{
	static vector<string> *result = 0;

	if(!result)
	{
		result = readPath("TraderPath", TRADER_DIR);

		const char *home = getenv("HOME");
		if(home) result->push_back(home + string("/.mcop/trader-cache"));
	}
	return result;
}

string MCOPUtils::mcopDirectory()
{
	static bool initialized = false;
	static string mcopDirectory;

	if(initialized)
		return mcopDirectory;
	initialized = true;

	const char *home = getenv("HOME");
	arts_return_val_if_fail(home != 0, "");

	mcopDirectory = home + string("/.mcop");

	mkdir(home,0755);
	if(mkdir(mcopDirectory.c_str(),0755) != 0)
	{
		string why = strerror(errno);

		struct stat st;
		stat(mcopDirectory.c_str(),&st);
		if(!S_ISDIR(st.st_mode))
		{
			arts_warning("can't create directory %s (%s)",
					 	 mcopDirectory.c_str(), why.c_str());

			mcopDirectory = "";
		}
	}
	return mcopDirectory;
}

bool MCOPUtils::tokenize(const string& line, string& key,vector<string>& values)
{
	string value;
	enum { sKey, sValue, sValueQuoted, sValueQuotedEscaped, sBad } state;

	state = sKey;
	for(string::const_iterator i = line.begin(); i != line.end(); i++)
	{
		char c = *i;
		unsigned char uc = static_cast<unsigned char>(c);

		arts_assert(c != '\n');

		if(state == sKey)
		{
			if(c == ' ' || c == '\t')
				; // ignore
			else if(isalnum(c))
				key += c;
			else if(c == '=')
				state = sValue;
			else
				state = sBad;
		}
		else if(state == sValue)
		{
			if(c == ' ' || c == '\t')
				; // ignore
			else if(c == '"')
				state = sValueQuoted;
			else if(c == ',')
			{
				values.push_back(value);
				value = "";
			}
			else if(uc > 32 && uc < 128)
				value += c;
			else
				state = sBad;
		}
		else if(state == sValueQuoted)
		{
			if(c == '"')
				state = sValue;
			else if(c == '\\')
				state = sValueQuotedEscaped;
			else
				value += c;
		}
		else if(state == sValueQuotedEscaped)
		{
			value += c;
			state = sValueQuoted;
		}
	}
	if(state == sValue)
		values.push_back(value);

	return(state != sBad);
}
