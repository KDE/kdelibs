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

#include <VCardEntity.h>

using namespace VCARD;

Entity::Entity()
	:	parsed_		(false),
		assembled_	(true)
{
	// empty
}
		
Entity::Entity(const Entity & e)
	:	strRep_		(e.strRep_),
		parsed_		(e.parsed_),
		assembled_	(e.assembled_)
{
	// empty
}
		
Entity::Entity(const QCString & s)
	:	strRep_		(s),
		parsed_		(false),
		assembled_	(true)
{
	// empty
}
		
	Entity &
Entity::operator = (const Entity & e)
{
	if (this == &e) return *this;
	
	strRep_		= e.strRep_;
	parsed_		= e.parsed_;
	assembled_	= e.assembled_;

	return *this;
}
		
	Entity &
Entity::operator = (const QCString & s)
{
	strRep_		= s;
	parsed_		= false;
	assembled_	= true;
	
	return *this;
}
		
	bool
Entity::operator == (Entity & e)
{
	return asString() == e.asString();
}
		
	bool
Entity::operator != (Entity & e)
{
	return !(*this == e);
}
		
	bool
Entity::operator == (const QCString & s)
{
	return asString() == s;
}
		
	bool
Entity::operator != (const QCString & s)
{
	return !(*this == s);
}
		
Entity::~Entity()
{
	// empty
}
		
	QCString
Entity::asString()
{
	assemble();
	
	return strRep_;
}
		
	void
Entity::parse()
{
	if (!parsed_) _parse();
	
	parsed_		= true;
	assembled_	= false;
}
		
	void
Entity::assemble()
{
	if (assembled_) return;

	parse();
	_assemble();
	
	assembled_	= true;
}

