	/*

    Copyright (C) 2001 Stefan Westerfeld
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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include "soundserver.h"
#include "debug.h"

using namespace Arts;
using namespace std;

namespace Arts {

class SampleStorageEntry_impl : public SampleStorageEntry_skel {
protected:
	string _name, _filename;
	bool _completed;
	FILE *file;

public:
	SampleStorageEntry_impl(const string& name, const string& filename)
	{
		_name = name;
		_filename = filename;
		_completed = false;
		file = fopen(_filename.c_str(),"w");
	}
	~SampleStorageEntry_impl()
	{
		if(file)
		{
			fclose(file);
			file = 0;
		}
		unlink(_filename.c_str());
	}

	string name() { return _name; }
	string filename() { return _filename; }
	bool completed() { return _completed; }

	void write(const vector<mcopbyte>& data)
	{
		arts_return_if_fail(file != 0);

		fwrite(&data[0],1,data.size(),file);
	}
	void finish()
	{
		arts_return_if_fail(file != 0);

		fclose(file);
		file = 0;
		_completed = true;
	}
};

class SampleStorage_impl : public SampleStorage_skel {
protected:
	string directory;
	long uniqueID;
	vector<SampleStorageEntry> entries;

public:
	SampleStorage_impl()
	{
		uniqueID = 1;
	}

	void constructor(const string& newDirectory, bool clearOnInit)
	{
		arts_return_if_fail(directory == "");

		directory = newDirectory;
		mkdir(directory.c_str(),0700);

		if(clearOnInit) clear();
	}

	void clear()
	{
		DIR *dir = opendir(directory.c_str());
		if(!dir) return;

		struct dirent *de;
		while((de = readdir(dir)) != 0)
		{
			string currentEntry = directory + "/" + de->d_name;

			if(de->d_name[0] != '.')
			{
				unlink(currentEntry.c_str());
			}
		}
		closedir(dir);
    }

	string encodeName(const string& name)
	{
		string::const_iterator i;
		string result;

		for(i = name.begin(); i != name.end(); i++)
		{
			unsigned char c = *i;

    		if ((c >= 'a' && c <= 'z') ||
    			(c >= 'A' && c <= 'Z') ||
    			(c >= '0' && c <= '9') ||
    			 c == '_' || c == '-')
      		{
				result += c;
			}
    		else
      		{
    			char hex[17] = "0123456789ABCDEF";
 
    			result += '=';
    			result += hex[(c >> 4) & 0xf];
    			result += hex[c & 0xf];
			}
		}
		return result;
	}

	SampleStorageEntry createEntry(const string& name)
	{
		char uniqueprefix[32];
		sprintf(uniqueprefix,"s%ld-",uniqueID++);
		string filename = directory + string("/")
						+ uniqueprefix + encodeName(name);

		return SampleStorageEntry::_from_base(
			new SampleStorageEntry_impl(name, filename)
		);
	}

	SampleStorageEntry findEntry(const string& name)
	{
		vector<SampleStorageEntry>::iterator i;

		for(i = entries.begin(); i != entries.end(); i++)
			if(i->name() == name) return *i;

		return SampleStorageEntry::null();
	}

	void addEntry(SampleStorageEntry entry)
	{
		arts_return_if_fail(!entry.isNull());

		/* remove the old entry for that name, if any */
		SampleStorageEntry oldEntry = findEntry(entry.name());
		if(!oldEntry.isNull())
			removeEntry(oldEntry);

		/* add the new entry */
		entries.push_back(entry);
	}

	void removeEntry(SampleStorageEntry entry)
	{
		arts_return_if_fail(!entry.isNull());

		vector<SampleStorageEntry>::iterator i;
		for(i = entries.begin(); i != entries.end(); i++)
		{
			if(i->name() == entry.name())
			{
				entries.erase(i);
				return;
			}
		}
		arts_warning("SampleStorage::removeEntry(%s) failed\n",
					 entry.name().c_str());
	}

};
REGISTER_IMPLEMENTATION(SampleStorage_impl);

};
