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

#include <qcstring.h>
#include <qstrlist.h>

#include <VCardEntity.h>
#include <VCardVCard.h>
#include <VCardContentLine.h>
#include <VCardRToken.h>

#include <VCardDefines.h>

using namespace VCARD;

VCard::VCard()
	:	Entity()
{
}

VCard::VCard(const VCard & x)
	:	Entity(x),
		group_(x.group_),
		contentLineList_(x.contentLineList_)
{
}

VCard::VCard(const QCString & s)
	:	Entity(s)
{
}

	VCard &
VCard::operator = (VCard & x)
{
	if (*this == x) return *this;

	group_				= x.group();
	contentLineList_	= x.contentLineList_;

	Entity::operator = (x);
	return *this;
}

	VCard &
VCard::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
VCard::operator == (VCard & x)
{
	x.parse();
	return false;
}

VCard::~VCard()
{
}

	void
VCard::_parse()
{
	vDebug("parse() called");
	QStrList l;

	RTokenise(strRep_, "\r\n", l);

	if (l.count() < 3) { // Invalid VCARD !
		vDebug("Invalid vcard");
		return;
	}

	// Get the first line
	QCString beginLine = QCString(l.at(0)).stripWhiteSpace();

	vDebug("Begin line == \"" + beginLine + "\"");

	// Remove extra blank lines
	while (QCString(l.last()).isEmpty())
		l.remove(l.last());

	// Now we know this is the last line
	QCString endLine = l.last();

	// Trash the first and last lines as we have seen them.
	l.remove(0u);
	l.remove(l.last());

	///////////////////////////////////////////////////////////////
	// FIRST LINE

	int split = beginLine.find(':');

	if (split == -1) { // invalid, no BEGIN
		vDebug("No split");
		return;
	}

	QCString firstPart(beginLine.left(split));
	QCString valuePart(beginLine.mid(split + 1));

	split = firstPart.find('.');

	if (split != -1) {
		group_		= firstPart.left(split);
		firstPart	= firstPart.right(firstPart.length() - split - 1);
	}

	if (qstrnicmp(firstPart, "BEGIN", 5) != 0) { // No BEGIN !
		vDebug("No BEGIN");
		return;
	}

	if (qstrnicmp(valuePart, "VCARD", 5) != 0) { // Not a vcard !
		vDebug("No VCARD");
		return;
	}

	///////////////////////////////////////////////////////////////
	// CONTENT LINES
	//
	vDebug("Content lines");

	// Handle folded lines.

	QStrList refolded;

	QStrListIterator it(l);

	QCString cur;

	for (; it.current(); ++it) {

		cur = it.current();

		++it;

		while (
			it.current()		&&
			it.current()[0] == ' '	&&
			strlen(it.current()) != 1)
		{
			cur += it.current() + 1;
			++it;
		}

		--it;

		refolded.append(cur);
	}

	QStrListIterator it2(refolded);

	for (; it2.current(); ++it2) {

		vDebug("New contentline using \"" + QCString(it2.current()) + "\"");
		ContentLine * cl = new ContentLine(it2.current());

		cl->parse();

		contentLineList_.append(cl);
	}

	///////////////////////////////////////////////////////////////
	// LAST LINE

	split = endLine.find(':');

	if (split == -1) // invalid, no END
		return;

	firstPart = endLine.left(split);
	valuePart = endLine.right(firstPart.length() - split - 1);

	split = firstPart.find('.');

	if (split != -1) {
		group_		= firstPart.left(split);
		firstPart	= firstPart.right(firstPart.length() - split - 1);
	}

	if (qstricmp(firstPart, "END") != 0) // No END !
		return;

	if (qstricmp(valuePart, "VCARD") != 0) // Not a vcard !
		return;
}

	void
VCard::_assemble()
{
	vDebug("Assembling vcard");
	strRep_ = "BEGIN:VCARD\r\n";
	strRep_ += "VERSION:3.0\r\n";

	QPtrListIterator<ContentLine> it(contentLineList_);

	for (; it.current(); ++it)
		strRep_ += it.current()->asString() + "\r\n";

	strRep_ += "END:VCARD\r\n";
}

	bool
VCard::has(EntityType t)
{
	parse();
	return contentLine(t) == 0 ? false : true;
}

	bool
VCard::has(const QCString & s)
{
	parse();
	return contentLine(s) == 0 ? false : true;
}

	void
VCard::add(const ContentLine & cl)
{
	parse();
	ContentLine * c = new ContentLine(cl);
	contentLineList_.append(c);
}

	void
VCard::add(const QCString & s)
{
	parse();
	ContentLine * c = new ContentLine(s);
	contentLineList_.append(c);
}

	ContentLine *
VCard::contentLine(EntityType t)
{
	parse();
	QPtrListIterator<ContentLine> it(contentLineList_);

	for (; it.current(); ++it)
		if (it.current()->entityType() == t)
			return it.current();

	return 0;
}

	ContentLine *
VCard::contentLine(const QCString & s)
{
	parse();
	QPtrListIterator<ContentLine> it(contentLineList_);

	for (; it.current(); ++it)
		if (it.current()->entityType() == s)
			return it.current();

	return 0;
}

