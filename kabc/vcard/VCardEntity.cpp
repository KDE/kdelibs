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

#include <qregexp.h>

#include <VCardDefines.h>
#include <VCardVCardEntity.h>

using namespace VCARD;

VCardEntity::VCardEntity()
	:	Entity()
{
}

VCardEntity::VCardEntity(const VCardEntity & x)
	:	Entity(x)
{
}

VCardEntity::VCardEntity(const QCString & s)
	:	Entity(s)
{
}

	VCardEntity &
VCardEntity::operator = (VCardEntity & x)
{
	if (*this == x) return *this;

	Entity::operator = (x);
	return *this;
}

	VCardEntity &
VCardEntity::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
VCardEntity::operator == (VCardEntity & x)
{
	x.parse();
	return false;
}

VCardEntity::~VCardEntity()
{
}

	void
VCardEntity::_parse()
{
	vDebug("parse");
	QCString s(strRep_);
	
	int i = s.find(QRegExp("BEGIN:VCARD", false));
	
	while (i != -1) {
		
		i = s.find(QRegExp("BEGIN:VCARD", false), 11);
		
		QCString cardStr(s.left(i));
		
		VCard * v = new VCard(cardStr);
		
		cardList_.append(v);
		
		v->parse();
		
		s.remove(0, i);
	}
}

	void
VCardEntity::_assemble()
{
	VCardListIterator it(cardList_);
	
	for (; it.current(); ++it)
		strRep_ += it.current()->asString() + "\r\n"; // One CRLF for luck.
}

	VCardList &
VCardEntity::cardList()
{
	parse();
	return cardList_;
}

	void
VCardEntity::setCardList(const VCardList & l)
{
	parse();
	cardList_ = l;
}

