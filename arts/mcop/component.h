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
#ifndef MCOP_COMPONENT_H
#define MCOP_COMPONENT_H

class ScheduleNode;

class Component
{
public:
	virtual ScheduleNode *node() = 0;
/*
protected:
public:
	// return the default port name in this direction, or "" if no default
	// default port is always defined when the component has only one port (!)
	// and should be mentioned explicitely with the 'default' modifier in the
	// idl in case the component has multiple ports
	virtual std::string defaultPort(PortDirection dir) = 0;
	// return 0 in case the port doesn't exist
	virtual PortType* portType(const std::string& port) = 0;
*/
};

#endif
