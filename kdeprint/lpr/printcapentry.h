/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
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

#ifndef PRINTCAPENTRY_H
#define PRINTCAPENTRY_H

#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qtextstream.h>

class Field
{
public:
	enum Type { String, Integer, Boolean };
	Field() : type(String) {}
	Field(const Field &f) : type(f.type), name(f.name), value(f.value) {}
	Field& operator= (const Field& f)
	{
		type = f.type;
		name = f.name;
		value = f.value;
		return (*this);
	}

	Type	type;
	QString	name;
	QString	value;
};

class PrintcapEntry
{
public:
	QString			name;
	QStringList		aliases;
	QString			comment;
	QMap<QString,Field>	fields;

	bool has(const QString& f) const	{ return fields.contains(f); }
	QString field(const QString& f) const	{ return fields[f].value; }
	bool writeEntry(QTextStream&);
	void addField(const QString& name, Field::Type type = Field::Boolean, const QString& value = QString::null);
};

#endif
