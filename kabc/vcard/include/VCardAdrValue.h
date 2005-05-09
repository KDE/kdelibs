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

#include <q3strlist.h>
#include <VCardValue.h>

namespace VCARD
{

class KVCARD_EXPORT AdrValue : public Value
{
	
#include "AdrValue-generated.h"
	
	AdrValue *clone();
	
	void setPOBox(const Q3CString & s)
	{ poBox_ = s; assembled_ = false; }
	
	void setExtAddress(const Q3CString & s)
	{ extAddress_ = s; assembled_ = false; }

	void setStreet(const Q3CString & s)
	{ street_ = s; assembled_ = false; }

	void setLocality(const Q3CString & s)
	{ locality_ = s; assembled_ = false; }

	void setRegion(const Q3CString & s)
	{ region_ = s; assembled_ = false; }

	void setPostCode(const Q3CString & s)
	{ postCode_ = s; assembled_ = false; }
	
	void setCountryName(const Q3CString & s)
	{ countryName_ = s; assembled_ = false; }
	
	Q3CString poBox()	{ parse(); return poBox_;	}
	Q3CString extAddress()	{ parse(); return extAddress_;	}
	Q3CString street()	{ parse(); return street_;	}
	Q3CString locality()	{ parse(); return locality_;	}
	Q3CString region()	{ parse(); return region_;	}
	Q3CString postCode()	{ parse(); return postCode_;	}
	Q3CString countryName()	{ parse(); return countryName_;	}
	
	private:
		
		Q3CString poBox_;
		Q3CString extAddress_;
		Q3CString street_;
		Q3CString locality_;
		Q3CString region_;
		Q3CString postCode_;
		Q3CString countryName_;
};

}

#endif

