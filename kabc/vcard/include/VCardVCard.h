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

#ifndef  VCARD_VCARD_H
#define  VCARD_VCARD_H

#include <qstring.h>
#include <qptrlist.h>

#include <VCardEnum.h>
#include <VCardEntity.h>
#include <VCardContentLine.h>

namespace VCARD
{

class VCard : public Entity
{

#include "VCard-generated.h"
	
	bool has(EntityType);
	bool has(const QCString &);
	
	void add(const ContentLine &);
	void add(const QCString &);
	
	ContentLine * contentLine(EntityType);
	ContentLine * contentLine(const QCString &);
	
	QCString group() { parse(); return group_; }
	
	QPtrList<ContentLine>	contentLineList() { parse(); return contentLineList_; }
	
	private:

	QCString		group_;
	QPtrList<ContentLine>	contentLineList_;
};

}

#endif
