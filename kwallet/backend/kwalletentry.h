/* This file is part of the KDE project
 *
 * Copyright (C) 2001-2003 George Staikos <staikos@kde.org>
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */ 


#ifndef _KWALLETENTRY_H
#define _KWALLETENTRY_H

#include <qstring.h>
#include <qdatastream.h>

#include "kwallet.h"

namespace KWallet {

/* @internal
 */
class KDE_EXPORT Entry {
	public:
		Entry();
		~Entry();
		
		const QString& key() const;
		const QByteArray& value() const;
		QString password() const;
		const QByteArray& map() const { return value(); }

		void setValue(const QByteArray& val);
		void setValue(const QString& val);
		void setKey(const QString& key);

		Wallet::EntryType type() const;
		void setType(Wallet::EntryType type);

		void copy(const Entry* x);

	private:
		QString _key;
		QByteArray _value;
		Wallet::EntryType _type;
};

}

#endif

