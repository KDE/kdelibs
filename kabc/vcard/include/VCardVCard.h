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

#ifndef  VCARD_VCARD_H
#define  VCARD_VCARD_H

#include <qstring.h>
#include <qlist.h>

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
	
	QList<ContentLine>	contentLineList() { parse(); return contentLineList_; }
	
	private:

	QCString		group_;
	QList<ContentLine>	contentLineList_;
};

}

#endif
