/*
	libvcard - vCard parsing library for vCard version 3.0

	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
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
