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

#ifndef  ADRVALUE_H
#define  ADRVALUE_H

#include <qstrlist.h>
#include <VCardValue.h>

namespace VCARD
{

class AdrValue : public Value
{
	
#include "AdrValue-generated.h"
	
	AdrValue *clone();
	
	void setPOBox(const QCString & s)
	{ poBox_ = s; assembled_ = false; }
	
	void setExtAddress(const QCString & s)
	{ extAddress_ = s; assembled_ = false; }

	void setStreet(const QCString & s)
	{ street_ = s; assembled_ = false; }

	void setLocality(const QCString & s)
	{ locality_ = s; assembled_ = false; }

	void setRegion(const QCString & s)
	{ region_ = s; assembled_ = false; }

	void setPostCode(const QCString & s)
	{ postCode_ = s; assembled_ = false; }
	
	void setCountryName(const QCString & s)
	{ countryName_ = s; assembled_ = false; }
	
	QCString poBox()	{ parse(); return poBox_;	}
	QCString extAddress()	{ parse(); return extAddress_;	}
	QCString street()	{ parse(); return street_;	}
	QCString locality()	{ parse(); return locality_;	}
	QCString region()	{ parse(); return region_;	}
	QCString postCode()	{ parse(); return postCode_;	}
	QCString countryName()	{ parse(); return countryName_;	}
	
	private:
		
		QCString poBox_;
		QCString extAddress_;
		QCString street_;
		QCString locality_;
		QCString region_;
		QCString postCode_;
		QCString countryName_;
};

}

#endif

