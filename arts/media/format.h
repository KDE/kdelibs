/*

    Copyright (C) 2000 Stefan Schimanski <1Stein@gmx.de>                      

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

#ifndef FORMAT_H_INCLUDED
#define FORMAT_H_INCLUDED

#include <std>
#include <map>
#include <string>

namespace "Arts" {

class Format : public map<string, string>
{
 public:
    bool isSet( string key ) 
	{
	    Format::iterator p = find( key );
	    return p!=end() && p.length()>0;
	};
};

};

#endif
