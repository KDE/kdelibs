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

#include <iostream.h>

#include <VCardEmailParam.h>
#include <VCardParam.h>
#include <VCardDefines.h>

using namespace VCARD;

EmailParam::EmailParam()
	:	Param()
{
	vDebug("ctor");
}

EmailParam::EmailParam(const EmailParam & x)
	:	Param(x),
		emailType_	(x.emailType_),
		pref_		(x.pref_)
{
}

EmailParam::EmailParam(const QCString & s)
	:	Param(s)
{
}

	EmailParam &
EmailParam::operator = (EmailParam & x)
{
	if (*this == x) return *this;
	
	emailType_	= x.emailType();
	pref_		= x.pref_;

	Param::operator = (x);
	return *this;
}

	EmailParam &
EmailParam::operator = (const QCString & s)
{
	Param::operator = (s);
	return *this;
}

	bool
EmailParam::operator == (EmailParam & x)
{
	parse();

	if (pref_)
		return (x.pref_ && x.emailType() == emailType_);

	return !x.pref();
}

EmailParam::~EmailParam()
{
}

	void
EmailParam::_parse()
{
#if 0
	Param::parseToList();
	
	SubParamListIterator it(subParamList_);
	
	pref_ = true;
	emailType_ = "";

	for (; it.current(); ++it) {
	
		if (qstricmp(it.current()->name(), "TYPE") == 0) {
			emailType_ = it.current()->value();
			continue;
		}
		
		if (qstricmp(it.current()->name(), "PREF") == 0) {
			pref_ = true;
		}
	}
#endif
}

	void
EmailParam::_assemble()
{
	strRep_ = "TYPE=";
	strRep_ += emailType_;

	if (pref_)
		strRep_ += ",PREF";
}

