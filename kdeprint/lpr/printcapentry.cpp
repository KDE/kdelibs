/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001,2002 Michael Goffioul <goffioul@imec.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "printcapentry.h"

bool PrintcapEntry::writeEntry(QTextStream& t)
{
	t << comment << endl;
	t << name;
	if (aliases.count() > 0)
		t << '|' << aliases.join("|");
	t << ':';
	for (QMap<QString,Field>::ConstIterator it=fields.begin(); it!=fields.end(); ++it)
	{
		t << '\\' << endl << "    :";
		t << (*it).name;
		switch ((*it).type)
		{
			case Field::String:
				t << '=' << (*it).value << ':';
				break;
			case Field::Integer:
				t << '#' << (*it).value << ':';
				break;
			case Field::Boolean:
				t << ':';
				break;
			default:
				t << endl << endl;
				return false;
		}
	}
	t << endl << endl;
	return true;
}

void PrintcapEntry::addField(const QString& name, Field::Type type, const QString& value)
{
	Field	f;
	f.name = name;
	f.type = type;
	f.value = value;
	fields[name] = f;
}
