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

#ifndef  NVALUE_H
#define  NVALUE_H

#include <VCardValue.h>

namespace VCARD
{

class NValue : public Value
{
#include "NValue-generated.h"
	NValue *clone();

	QCString family()	{ parse(); return family_;	}
	QCString given()	{ parse(); return given_;	}
	QCString middle()	{ parse(); return middle_;	}
	QCString prefix()	{ parse(); return prefix_;	}
	QCString suffix()	{ parse(); return suffix_;	}

	void setFamily	(const QCString & s) { family_	= s; assembled_ = false; }
	void setGiven	(const QCString & s) { given_	= s; assembled_ = false; }
	void setMiddle	(const QCString & s) { middle_	= s; assembled_ = false; }
	void setPrefix	(const QCString & s) { prefix_	= s; assembled_ = false; }
	void setSuffix	(const QCString & s) { suffix_	= s; assembled_ = false; }
	
	private:
	
	QCString family_, given_, middle_, prefix_, suffix_;
};

}

#endif
