    /*

    Copyright (C) 2000 Stefan Westerfeld
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

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

    */

#include "mcopconfig.h"
#include <fstream>

using namespace std;

MCOPConfig::MCOPConfig(const string& filename) :filename(filename)
{
}

string MCOPConfig::readEntry(const string& key, const string& defaultValue)
{
	ifstream in(filename.c_str());
	string keyvalue;

	while(in >> keyvalue)
	{
    	int i = keyvalue.find("=",0);
      	if(i != 0 && keyvalue.substr( 0, i ) == key)
      		return keyvalue.substr( i+1, keyvalue.size()-(i+1) );
	}
	return defaultValue;
}
