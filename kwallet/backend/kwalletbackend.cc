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

#include "kwalletbackend.h"
#include <kglobal.h>
#include <kstddirs.h>
#include <qfile.h>
#include "sha1.h"



#define KWMAGIC "KWALLET\n\0"
#define KWMAGIC_LEN 9

KWalletBackend::KWalletBackend(QString name) : _name(name) {
	KGlobal::dirs()->addResourceType("kwallet", "share/apps/kwallet");
	_open = false;
}


KWalletBackend::~KWalletBackend() {
	if (_open) {
		// Discard changes
	}
}


static int password2hash(const QByteArray& password, QByteArray& hash) {
	QByteArray even;
	QByteArray odd;

	// Split the odd entries and the even entries in the password
	// into separate hashes

	even.resize(password.size()/2+password.size()%2);
	odd.resize(password.size()/2);

	for (unsigned int i = 0; i < password.size(); i++) {
		if (i % 2 == 0)
			even[i/2] = password[i];
		else 
			odd[i/2] = password[i];
	}

	SHA1 sha;

	sha.process(password.data(), password.size());

	hash.duplicate((const char *)sha.getHash(), 20);

	sha.reset();

	sha.process(even.data(), even.size());

	// FIXME: fill in this part
	
	sha.reset();

	sha.process(odd.data(), odd.size());

	// FIXME: fill in this part
	
	sha.reset();

	even.fill(0);
	odd.fill(0);
	return 0;
}


int KWalletBackend::unlock(QByteArray& password) {
	QString path = KGlobal::dirs()->saveLocation("kwallet") + 
		       "/"+_name+".kwl";


	QByteArray passhash;
	password2hash(password, passhash);

	// No wallet existed.  Let's create it.
	if (!QFile::exists(path)) {
		QFile newfile(path);
		if (!newfile.open(IO_ReadWrite))
			return -2;
		newfile.close();
		return 1;
	}

	QFile db(path);
	db.open(IO_ReadOnly);

	if (!db.open(IO_ReadOnly))
		return -2;

	char magicBuf[10];
	db.readBlock(magicBuf, KWMAGIC_LEN);
	if (qstrncmp(magicBuf, KWMAGIC, KWMAGIC_LEN)) {
		return -3;         // bad magic
	}

	QByteArray encrypted = db.readAll();

	// Decrypt the encrypted data

	// Load the data structures up

	// "0" out all the structures that we are done with
	// that are not encrypted (including hashes)

	return -1;
}

	
int KWalletBackend::lock(QByteArray& password) {
	return -1;
}


const QString& KWalletBackend::walletName() const {
	return _name;
}


bool KWalletBackend::isOpen() const {
	return _open;
}


bool KWalletBackend::changeWallet(QString name) {
	if (_open)
		return false;

	_name = name;
	return true;
}


const QPtrList<KWalletEntry>& KWalletBackend::getEntriesByApp(QString app) const {
	return _entries[app];
}


const QStringList KWalletBackend::getAppList() const {
QStringList list;

	for (QMap< QString, QPtrList< KWalletEntry > >::ConstIterator i = _entries.begin(); i != _entries.end(); ++i) {
		list << i.key();
	}
	return list;
}





