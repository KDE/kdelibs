/* This file is part of the KDE project
 *
 * Copyright (C) 2001,2002 George Staikos <staikos@kde.org>
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
#include <kdebug.h>
#include <kstandarddirs.h>
#include <qfile.h>
#include "blowfish.h"
#include "sha1.h"

#include <assert.h>



using namespace KWallet;

#define KWMAGIC "KWALLET\n\0"
#define KWMAGIC_LEN 9

Backend::Backend(const QString& name) : _name(name) {
	KGlobal::dirs()->addResourceType("kwallet", "share/apps/kwallet");
	_open = false;
}


Backend::~Backend() {
	if (_open) {
		// FIXME: Discard changes
	}
}


static int getRandomBlock(QByteArray& randBlock) {
	// First try /dev/urandom
	if (QFile::exists("/dev/urandom")) {
		QFile devrand("/dev/urandom");
		if (devrand.open(IO_ReadOnly)) {

		unsigned int rc = devrand.readBlock(randBlock.data(), randBlock.size());

			if (rc != randBlock.size())
				return -3;		// not enough data read

			return 0;
		}
	}

	// If that failed, try /dev/random
	if (QFile::exists("/dev/random")) {
		QFile devrand("/dev/random");
		if (devrand.open(IO_ReadOnly)) {

		unsigned int rc = 0;
		unsigned int cnt = 0;
		
			do {
				int rc2 = 
				       devrand.readBlock(randBlock.data() + rc, 
							 randBlock.size());

				if (rc2 < 0)
					return -3;	// read error

				rc += rc2;
				cnt++;
				if (cnt > randBlock.size())
					return -4;	// reading forever?!
			} while(rc < randBlock.size());

			return 0;
		}
	}

	// EGD method?


	// Couldn't get any random data!!

	return -1;
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


int Backend::open(const QByteArray& password) {

	if (_open)
		return -255;  // already open

	QString path = KGlobal::dirs()->saveLocation("kwallet") + 
		       "/"+_name+".kwl";


	QByteArray passhash;

	// No wallet existed.  Let's create it.
	if (!QFile::exists(path)) {
		QFile newfile(path);
		if (!newfile.open(IO_ReadWrite))
			return -2;   // error opening file
		newfile.close();
		_open = true;
		return 1;          // new file opened, but OK
	}

	QFile db(path);

	if (!db.open(IO_ReadOnly))
		return -2;         // error opening file

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


	bf.setKey((void *)passhash.data(), passhash.size()*8);

	int rc = bf.decrypt(encrypted.data(), encrypted.size());
	if (rc < 0) {
		passhash.fill(0);
		encrypted.fill(0);
		return -6;	// decrypt error
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
	
	// Load the data structures up
	/* LOOK HERE DAWIT */

	// chop off the leading blksz+4 bytes
	QByteArray tmpenc;
	tmpenc.duplicate(encrypted.data()+blksz+4, fsize);
	encrypted.fill(0);
	encrypted.duplicate(tmpenc.data(), tmpenc.size());
	tmpenc.fill(0);

	_open = true;
	return 0;
}

	
int Backend::close(const QByteArray& password) {
	if (!_open)
		return -255;  // not open yet

	QString path = KGlobal::dirs()->saveLocation("kwallet") + 
		       "/"+_name+".kwl";

	QFile qf(path);

	if (!qf.open(IO_WriteOnly))
		return -1;		// error opening file

	qf.writeBlock(KWMAGIC, KWMAGIC_LEN);

	// Write the version number
	QByteArray version(1);
	version[0] = 0;
	qf.writeBlock(version, 1);
	qf.writeBlock(version, 1);

	QByteArray decrypted;

	// populate decrypted
	/* LOOK HERE DAWIT */


	// calculate the hash of the file
	SHA1 sha;
	BlowFish bf;

	sha.process(decrypted.data(), decrypted.size());

	// prepend and append the random data
	QByteArray wholeFile;
	long blksz = bf.blockSize();
	long newsize = decrypted.size() + 
		       blksz            +    // encrypted block
		       4                +    // file size
		       20;      // size of the SHA hash

	int delta = (blksz - (newsize % blksz));
	newsize += delta;
	wholeFile.resize(newsize);

	QByteArray randBlock;
	randBlock.resize(blksz+delta);
	if (getRandomBlock(randBlock) < 0) {
		sha.reset();
		decrypted.fill(0);
		return -3;		// Fatal error: can't get random
	}
	
	for (int i = 0; i < blksz; i++)
		wholeFile[i] = randBlock[i];

	for (int i = 0; i < 4; i++)
		wholeFile[(int)(i+blksz)] = (decrypted.size() >> 8*(3-i))&0xff;

	for (unsigned int i = 0; i < decrypted.size(); i++)
		wholeFile[(int)(i+blksz+4)] = decrypted[i];

	for (int i = 0; i < delta; i++)
		wholeFile[(int)(i+blksz+4+decrypted.size())] = randBlock[(int)(i+blksz)];

	const char *hash = (const char *)sha.getHash();
	for (int i = 0; i < 20; i++)
		wholeFile[(int)(newsize-20+i)] = hash[i];

	sha.reset();
	decrypted.fill(0);

	// hash the passphrase
	QByteArray passhash;
	password2hash(password, passhash);
	
	// encrypt the data
	if (!bf.setKey(passhash.data(), passhash.size()*8)) {
		passhash.fill(0);
		wholeFile.fill(0);
		return -2;
	}

	int rc = bf.encrypt(wholeFile.data(), wholeFile.size());
	if (rc < 0) {
		passhash.fill(0);
		wholeFile.fill(0);
		return -2;	// encrypt error
	}

	passhash.fill(0);        // passhash is UNUSABLE NOW

	// write the file
	qf.writeBlock(wholeFile, wholeFile.size());
	qf.close();

	wholeFile.fill(0);

	_open = false;
	return 0;
}


const QString& Backend::walletName() const {
	return _name;
}


bool Backend::isOpen() const {
	return _open;
}


bool Backend::changeWallet(const QString& name) {
	if (_open)
		return false;

	_name = name;
	return true;
}


const QPtrList<Entry>& Backend::getEntriesByType(const QString& type) const {
	return _entries[type];
}


const QStringList Backend::getTypeList() const {
QStringList list;

	for (QMap< QString, QPtrList< Entry > >::ConstIterator i = _entries.begin(); i != _entries.end(); ++i) {
		list << i.key();
	}
	return list;
}





