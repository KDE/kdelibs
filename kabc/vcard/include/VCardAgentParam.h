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

#ifndef  AGENTPARAM_H
#define  AGENTPARAM_H

#include <qcstring.h>

#include <VCardParam.h>
#include <VCardURIValue.h>

namespace VCARD
{

class AgentParam : public Param
{
	
#include "AgentParam-generated.h"
	
	bool refer()
	{ parse(); return refer_; }
	
	URIValue uri()
	{ parse(); return uri_; }
	
	void setRefer(bool b)
	{ refer_ = b; assembled_ = false; }

	void setURI(const QCString & s)
	{ uri_ = s; assembled_ = false; }
	
	private:
		
		bool		refer_;
		URIValue	uri_;
};

}

#endif
