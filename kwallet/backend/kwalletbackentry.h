/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


#ifndef _KWALLETBACKENTRY_H
#define _KWALLETBACKENTRY_H


#include <qcstring.h>
#include <qstring.h>
#include <qstringlist.h>


class KWalletBackEntry {
	public:
		KWalletBackEntry();
		~KWalletBackEntry();
		
		bool isDirty() const;

		const QStringList& key() const;
		const QByteArray& value() const;

		void setValue(QByteArray& val);
		void setKey(QStringList key);
		void addKey(QString key);

	protected:

	private:
		class KWalletBackEntryPrivate;
		KWalletBackEntryPrivate *d;
		QString _app;
		bool _dirty;
		QStringList _key;
		QByteArray _value;
};


#endif

