/*
	Copyright (C) 2000 Nicolas Brodu
	nicolas.brodu@free.fr

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
#include "component.h"
#include "connect.h"
#include "flowsystem.h"
//#include <cassert>

//namespace MCOP {

// Connect function overloaded for components with default port
void connect(Component* src, const std::string& output, Component* dest, const std::string& input)
{
	assert(src); assert(dest);
	src->node()->connect(output, dest->node(), input);
}

/* // default ports not implemented yet
void connect(Component* src, const string& output, Component* dest);
void connect(Component* src, Component* dest, const string& input);
void connect(Component* src, Component* dest);
*/

// setValue function overloaded for components with default port
void setValue(Component* c, const std::string& port, const float fvalue)
{
	assert(c);
	c->node()->setFloatValue(port, fvalue);
}

/* // default ports not implemented yet
void setValue(Component* c, const string& port, const string& svalue);
void setValue(Component* c, const float fvalue);
void setValue(Component* c, const string& svalue);
*/
//}
