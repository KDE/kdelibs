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

#include <VCardAgentParam.h>

#include <VCardParam.h>

using namespace VCARD;

AgentParam::AgentParam()
	:	Param()
{
}

AgentParam::AgentParam(const AgentParam & x)
	:	Param(x),
		refer_	(x.refer_),
		uri_	(x.uri_)
{
}

AgentParam::AgentParam(const QCString & s)
	:	Param(s)
{
}

	AgentParam &
AgentParam::operator = (AgentParam & x)
{
	if (*this == x) return *this;
	
	refer_	= x.refer_;
	uri_	= x.uri_;

	Param::operator = (x);
	return *this;
}

	AgentParam &
AgentParam::operator = (const QCString & s)
{
	Param::operator = (s);
	return *this;
}

	bool
AgentParam::operator == (AgentParam & x)
{
	parse();

	if (refer_)
		return (x.refer() && uri_ == x.uri_);
	
	return !x.refer();
}

AgentParam::~AgentParam()
{
}

	void
AgentParam::_parse()
{
	if (strRep_.isEmpty()) {
		refer_ = false;
		return;
	}
	
	refer_	= true;
	uri_	= strRep_;
}

	void
AgentParam::_assemble()
{
	if (!refer_) {
		strRep_.truncate(0);
		return;
	}
	
	strRep_ = uri_.asString();
	return;
}
