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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "core.h"
#include "mcoputils.h"

using namespace std;
using namespace Arts;

namespace Arts {

class TmpGlobalComm_impl : virtual public TmpGlobalComm_skel {
public:
	bool put(const string& variable, const string& value)
	{
		string filename = MCOPUtils::createFilePath(variable);

		int fd = open(filename.c_str(),O_CREAT|O_EXCL|O_WRONLY,S_IRUSR|S_IWUSR);
		if(fd != -1)
		{
			int result = write(fd,value.c_str(),value.size());

			assert(result == (int)value.size());
			close(fd);
			return true;
		}

		return false;
	}

	string get(const string& variable)
	{
		string result = "";
		string filename = MCOPUtils::createFilePath(variable);

		int fd = open(filename.c_str(),O_RDONLY);
		if(fd != -1)
		{
			char buffer[8192];
			int size = read(fd,buffer,8192);
			if(size > 0 && size < 8192) {
				buffer[size] = 0;
				result = buffer;
			}

			close(fd);
		}
		return result;
	}

	void erase(const string& variable)
	{
		string filename = MCOPUtils::createFilePath(variable);
		unlink(filename.c_str());
	}
};

REGISTER_IMPLEMENTATION(TmpGlobalComm_impl);

};
