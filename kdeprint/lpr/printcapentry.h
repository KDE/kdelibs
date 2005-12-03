/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef PRINTCAPENTRY_H
#define PRINTCAPENTRY_H

#if !defined( _KDEPRINT_COMPILE ) && defined( __GNUC__ )
#warning internal header, do not use except if you are a KDEPrint developer
#endif

#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qtextstream.h>

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
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
	QString toString() const;

	Type	type;
	QString	name;
	QString	value;
};

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class PrintcapEntry
{
public:
	QString			name;
	QStringList		aliases;
	QString			comment;
	QMap<QString,Field>	fields;
	QString			postcomment;

	bool has(const QString& f) const	{ return fields.contains(f); }
	QString field(const QString& f) const	{ return fields[f].value; }
	bool writeEntry(QTextStream&);
	void addField(const QString& name, Field::Type type = Field::Boolean, const QString& value = QString::null);
};

#endif
