/*
	libvcard - vCard parsing library for vCard version 3.0
	
	Copyright (C) 1999 Rik Hemsley rik@kde.org
	
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

