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
#include "blowfish.h"
#include "sha1.h"

#include <assert.h>



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


// this should be SHA-512 for release probably
static int password2hash(const QByteArray& password, QByteArray& hash) {
	QByteArray first, second, third;

	first.resize(password.size()/3 + (password.size()%3 == 1 ? 1 : 0));
	second.resize(password.size()/3 + (password.size()%3 == 2 ? 1 : 0));
	third.resize(password.size()/3);

	// The hash works like this:
	//     Split the passphrase into thirds and make three hashes 
	//     with them.  Concatenate them.
	for (unsigned int i = 0; i < password.size(); i++) {
		switch (i % 3) {
			case 0:
				first[i/3] = password[i];
				break;
			case 1:
				second[(i-1)/3] = password[i];
				break;
			case 2:
				third[(i-2)/3] = password[i];
				break;
		}
	}

	SHA1 sha;

	sha.process(first.data(), first.size());

	hash.duplicate((const char *)sha.getHash(), 19);

	sha.reset();

	sha.process(second.data(), second.size());

	hash.resize(56);

	const char *t = (const char *)sha.getHash();

	for (unsigned int i = 0; i < 19; i++)
		hash[19+i] = t[i];

	sha.reset();

	sha.process(third.data(), third.size());

	t = (const char *)sha.getHash();

	for (unsigned int i = 0; i < 18; i++)
		hash[38+i] = t[i];

	sha.reset();

	first.fill(0);
	second.fill(0);
	third.fill(0);

	return 0;
}


int KWalletBackend::unlock(QByteArray& password) {
	QString path = KGlobal::dirs()->saveLocation("kwallet") + 
		       "/"+_name+".kwl";


	QByteArray passhash;

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

	db.readBlock(magicBuf, 2);

	// First byte is major version, second byte is minor version
	//    OUR VERSION: 0.0
	if (magicBuf[0] != 0)
		return -4;         // unknown version

	if (magicBuf[1] != 0)
		return -4;	   // unknown version

	QByteArray encrypted = db.readAll();
	assert(encrypted.size() < db.size());

	BlowFish bf;
	int blksz = bf.blockSize();
	if ((encrypted.size() % blksz) != 0)
		return -5;	   // invalid file structure

	// Decrypt the encrypted data
	password2hash(password, passhash);


	bf.setKey((void *)passhash.data(), 448);

	for (unsigned int i = 0; i < encrypted.size() % blksz; i++) {
		int rc = bf.decrypt(encrypted.data() + i*blksz, blksz);
		if (rc != 0) {
			passhash.fill(0);
			encrypted.fill(0);
			return -6;	// decrypt error
		}
	}

	passhash.fill(0);        // passhash is UNUSABLE NOW

	const char *t = encrypted.data();

	// strip the leading data
	t += blksz;    // one block of random data

	// strip the file size off
	long fsize = 0;

	fsize |= ((char)*t << 24);
	t++;
	fsize |= ((char)*t << 16);
	t++;
	fsize |= ((char)*t <<  8);
	t++;
	fsize |=  (char)*t;
	t++;

	if (fsize < 0) {
		encrypted.fill(0);
		return -7;         // file structure error.
	}

	// compute the hash ourself
	SHA1 sha;
	sha.process(t, fsize);
	const char *testhash = (const char *)sha.getHash();

	// compare hashes
	int sz = encrypted.size();
	for (int i = 0; i < 20; i++) {
		if (testhash[i] != encrypted[sz-20+i]) {
			encrypted.fill(0);
			sha.reset();
			return -8;         // hash error.
		}
	}
	
	sha.reset();
	// get rid of the trailing data
	encrypted.truncate(fsize+blksz+4);
	
	// Load the data structures up
	

	// "0" out all the structures that we are done with
	// that are not encrypted

	encrypted.fill(0);

	return 0;
}

	
int KWalletBackend::lock(QByteArray& password) {
	QString path = KGlobal::dirs()->saveLocation("kwallet") + 
		       "/"+_name+".kwl";

	QFile qf(path);

	if (!qf.open(IO_WriteOnly))
		return -1;		// error opening file

	qf.writeBlock(KWMAGIC, KWMAGIC_LEN);

	QByteArray decrypted;

	// populate decrypted


	// calculate the hash of the file
	SHA1 sha;

	sha.process(decrypted.data(), decrypted.size());

	// prepend and append the random data
	

	// append the hash
	
	
	// hash the passphrase
	QByteArray passhash;
	password2hash(password, passhash);
	
	// encrypt the data
	

	// write the file


	passhash.fill(0);

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





