/*
	libvcard - vCard parsing library for vCard version 3.0

	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
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

#include <VCardParam.h>

#include <VCardEntity.h>

#include <VCardRToken.h>

using namespace VCARD;

Param::Param()
	:	Entity(),
		name_(""),
		value_("")
{
}

Param::Param(const Param & x)
	:	Entity(x),
		name_(x.name_),
		value_(x.value_)
{
}

Param::Param(const QCString & s)
	:	Entity(s),
		name_(""),
		value_("")
{
}

	Param &
Param::operator = (Param & x)
{
	if (*this == x) return *this;

	Entity::operator = (x);
        name_ = x.name_;
        value_ = x.value_;
        
	return *this;
}

	Param &
Param::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
Param::operator == (Param & x)
{
	x.parse();
	return false;
}

Param::~Param()
{
}

	void
Param::_parse()
{
}

	void
Param::_assemble()
{
	strRep_ = name_ + "=" + value_;
}

Param::Param(const QCString &name, const QCString &value)
	:	Entity(),
		name_(name),
		value_(value)
{
	parsed_ = true;
	assembled_ = false;
}

	void
Param::setName(const QCString & name)
{
	name_ = name;
        
        assembled_ = false;
}

	void
Param::setValue(const QCString & value)
{
	value_ = value;
        
        assembled_ = false;
}

	QCString
Param::name()
{
	return name_;
}

	QCString
Param::value()
{
	return value_;
}
