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

#include <VCardRToken.h>
#include <VCardAdrParam.h>
#include <VCardParam.h>

using namespace VCARD;

AdrParam::AdrParam()
	:	Param()
{
}

AdrParam::AdrParam(const AdrParam & x)
	:	Param(x),
		adrTypeList_	(x.adrTypeList_)
{
}

AdrParam::AdrParam(const QCString & s)
	:	Param(s)
{
}

	AdrParam &
AdrParam::operator = (AdrParam & x)
{
	if (*this == x) return *this;
	
	adrTypeList_	= x.adrTypeList();
	textParam_		= x.textParam();

	Param::operator = (x);
	return *this;
}

	AdrParam &
AdrParam::operator = (const QCString & s)
{
	Param::operator = (s);
	
	adrTypeList_.clear();
	textParam_.truncate(0);
	
	return *this;
}

	bool
AdrParam::operator == (AdrParam & x)
{
	parse();

	if (!x.textParam().isEmpty())
		return (x.textParam_ == textParam_);

	if (x.adrTypeList().count() != adrTypeList_.count())
		return false;
	
	QStrListIterator it(x.adrTypeList_);
	
	for (; it.current(); ++it)
		if (!adrTypeList_.find(it.current()))
			return false;
	
	return true;
}

AdrParam::~AdrParam()
{
}

	void
AdrParam::_parse()
{
	adrTypeList_.clear();

	if (strRep_.left(4) != "TYPE") {
		textParam_ = strRep_;
		return;
	}
	
	if (!strRep_.contains('='))
		return;
	
	RTokenise(strRep_, ",", adrTypeList_);
}

	void
AdrParam::_assemble()
{
	if (!textParam_.isEmpty()) {
		strRep_ = textParam_;
		return;
	}
	
	QStrListIterator it(adrTypeList_);
	
	for (; it.current(); ++it) {
		
		strRep_ += it.current();
		
		if (it.current() != adrTypeList_.last())
			strRep_ += ',';
	}
}
