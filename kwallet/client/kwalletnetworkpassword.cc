/* This file is part of the KDE project
 *
 * Copyright (C) 2002 Tim Jansen <tim@tjansen.de>
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


#include "kwalletnetworkpassword.h"
#include <qglobal.h> 
#include <qdatastream.h> 


using namespace KWallet;

const char *NetworkPassword::typeName = "kde.org-NetworkPassword";

NetworkPassword::NetworkPassword(const QString &type,
				 const Key &key, 
				 const Value &value) :
	Entry(type, key, value) {

	QDataStream s(value, IO_ReadOnly);
	int version;
	int blockedInt = _blocked ? 1 : 0;
	s >> version >> blockedInt >> _userName >> _password;
	Q_ASSERT(version == 1); // do something when this fails?
}

NetworkPassword::~NetworkPassword() {
}
		
QString NetworkPassword::url() const {
	return key()["url"];
}

bool NetworkPassword::blocked() const {
	return _blocked;
}

QString NetworkPassword::userName() const {
	return _userName;
}

QString NetworkPassword::password() const {
	return _password;
}

QString NetworkPassword::type() const {
	return typeName;
}

QPtrList<NetworkPassword> NetworkPassword::getNetworkPasswords(
	const QString& url, const QString& userName) {
	QPtrList<Entry> l = Entry::getEntries(QString(typeName), &factory, 
					      url.isNull() ? QString::null : "url",
					      url);
	QPtrList<NetworkPassword> l2;

	Entry *e = l.first();
	while (e) {
		Q_ASSERT(e->type() == typeName);
		NetworkPassword *np = (NetworkPassword*) e;	
		if (userName.isNull() || (np->userName() == userName))
			l2.append(np);
		e = l.next();
	}
	return l2;
}

Entry *NetworkPassword::factory(const QString &type,
				const Key &key, 
				const Value &value) {
	return new NetworkPassword(type, key, value);
}
