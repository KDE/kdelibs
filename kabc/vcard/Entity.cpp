/*
	libvcard - vCard parsing library for vCard version 3.0
	
	Copyright (C) 1999 Rik Hemsley rik@kde.org
	
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
//	vDebug("Entity::asString()");
	assemble();

	return strRep_;
}
		
	void
Entity::parse()
{
//	vDebug( "Entity::parse()" );

	if (!parsed_) _parse();
	
	parsed_		= true;
	assembled_	= false;
}
		
	void
Entity::assemble()
{
//	vDebug( "Entity::assemble()" );

	if (assembled_) return;

	parse();
	_assemble();
	
	assembled_	= true;
}

