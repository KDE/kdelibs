/*
	libvcard - vCard parsing library for vCard version 3.0
	
	Copyright (C) 1999 Rik Hemsley rik@kde.org
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef  ENTITY_H
#define  ENTITY_H

#include <qcstring.h>

namespace VCARD
{

class Entity
{
	public:
		
		Entity();
		Entity(const Entity & e);
		Entity(const QCString & s);
		
		virtual Entity & operator = (const Entity & e);
		virtual Entity & operator = (const QCString & s);
		
		virtual bool operator == (Entity & e);
		virtual bool operator != (Entity & e);
		virtual bool operator == (const QCString & s);
		virtual bool operator != (const QCString & s);
		
		virtual ~Entity();
		
		QCString asString();
		
		virtual void parse();
		virtual void assemble();
		
		virtual void _parse() = 0;
		virtual void _assemble() = 0;
	
	protected:
		
		QCString strRep_;
		bool parsed_;
		bool assembled_;
};

}

#endif
