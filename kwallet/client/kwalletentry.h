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


#ifndef _KWALLETENTRY_H
#define _KWALLETENTRY_H


#include <qcstring.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qpair.h>
#include <qvaluelist.h>

namespace KWallet {

typedef QPair<QString,QString> NVPair;
typedef QValueList<NVPair> Key;
typedef QByteArray Value;

class Entry {
	public:
		Entry();
		~Entry();
		
		bool isDirty() const;
		void clearDirty();

		const Key& key() const;
		const Value& value() const;

		void setValue(const Value& val);
		void setKey(const Key& key);
		void addKey(const NVPair& key);

	private:
		class KWalletEntryPrivate;
		KWalletEntryPrivate *d;
		QString _app;
		bool _dirty;
		Key _key;
		Value _value;
};


};

#endif

