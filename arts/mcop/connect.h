    /*

	Copyright (C) 2000 Nicolas Brodu
	nicolas.brodu@free.fr

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

#ifndef MCOP_CONNECT_H
#define MCOP_CONNECT_H

#include <string>
class SmartWrapper;
//namespace MCOP {

// Connect function overloaded for components with default port
void connect(const SmartWrapper& src, const std::string& output, const SmartWrapper& dest, const std::string& input);
void connect(const SmartWrapper& src, const string& output, const SmartWrapper& dest);
void connect(const SmartWrapper& src, const SmartWrapper& dest, const string& input);
void connect(const SmartWrapper& src, const SmartWrapper& dest);
// Same for disconnect function
void disconnect(const SmartWrapper& src, const std::string& output, const SmartWrapper& dest, const std::string& input);
void disconnect(const SmartWrapper& src, const string& output, const SmartWrapper& dest);
void disconnect(const SmartWrapper& src, const SmartWrapper& dest, const string& input);
void disconnect(const SmartWrapper& src, const SmartWrapper& dest);

// setValue function overloaded for components with default port
void setValue(const SmartWrapper& c, const std::string& port, const float fvalue);
void setValue(const SmartWrapper& c, const float fvalue);
//void setValue(const SmartWrapper& c, const string& port, const string& svalue);
//void setValue(const SmartWrapper& c, const string& svalue);

//}

#endif
