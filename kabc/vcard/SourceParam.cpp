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

#include <VCardSourceParam.h>

#include <VCardParam.h>

using namespace VCARD;

SourceParam::SourceParam()
	:	Param(),
		type_(SourceParam::TypeUnknown)
{
}

SourceParam::SourceParam(const SourceParam & x)
	:	Param(x),
		type_	(x.type_),
		par_	(x.par_),
		val_	(x.val_)
{
}

SourceParam::SourceParam(const QCString & s)
	:	Param(s),
		type_(SourceParam::TypeUnknown)
{
}

	SourceParam &
SourceParam::operator = (SourceParam & x)
{
	if (*this == x) return *this;
	type_	= x.type();
	par_	= x.par();
	val_	= x.val();

	Param::operator = (x);
	return *this;
}

	SourceParam &
SourceParam::operator = (const QCString & s)
{
	Param::operator = (s);
	return *this;
}

	bool
SourceParam::operator == (SourceParam & x)
{
	x.parse();
	return false;
}

SourceParam::~SourceParam()
{
}

	void
SourceParam::_parse()
{
	int i = strRep_.find('=');
	if (i == -1) // Invalid
		return;
	
	par_ = strRep_.left(i);
	val_ = strRep_.right(strRep_.length() - i - 1);
	
	if (qstricmp(par_, "VALUE") == 0 && qstricmp(val_, "uri") == 0)
		type_ = TypeValue;
	else if (qstricmp(par_, "CONTEXT") == 0 && qstricmp(val_, "word") == 0)
		type_ = TypeContext;
	else if (qstrnicmp(par_, "X-", 2) == 0) {
		type_ = TypeX;
	}
	else type_ = TypeUnknown;
	
}

	void
SourceParam::_assemble()
{
	if (type_ == TypeValue)
		strRep_ = "VALUE=uri";
	else if (type_ == TypeContext)
		strRep_ = "CONTEXT=word";
	else if (type_ == TypeX)
		strRep_ = par_ + "=" + val_;
	else strRep_ = "";
}

