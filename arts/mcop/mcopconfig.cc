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

#include "mcopconfig.h"
#include "mcoputils.h"
#include <fstream>

using namespace std;
using namespace Arts;

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

vector<string> *MCOPConfig::readListEntry(const string& key)
{
	vector<string> *result = new vector<string>;

	ifstream in(filename.c_str());
	string keyvalue;

	while(in >> keyvalue)
	{
		string k;

		MCOPUtils::tokenize(keyvalue,k,*result);
		if(k == key)
			return result;

		result->clear();
	}

	return result;
}
