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

#ifndef  EMAILPARAM_H
#define  EMAILPARAM_H

#include <qcstring.h>

#include <VCardParam.h>

namespace VCARD
{

class EmailParam : public Param
{
	
#include "EmailParam-generated.h"
	
	QCString	emailType() { parse(); return emailType_;	}
	bool		pref()		{ parse(); return pref_;		}
	
	void setEmailType(const QCString & s)
	{ emailType_ = s; assembled_ = false; }
	
	void setPref(bool b)
	{ pref_ = b; assembled_ = false; }
	
	private:
	
		QCString	emailType_;
		bool		pref_;
};

}

#endif
