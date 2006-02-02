/* This file is part of the KDE project
 *
 * Copyright (C) 2001-2004 George Staikos <staikos@kde.org>
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

#include "kwalletbackend.h"

#include <stdlib.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcodecs.h>
#include <ksavefile.h>
#include <kstandarddirs.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>

#include "blowfish.h"
#include "sha1.h"
#include "cbc.h"

#include <assert.h>

#define KWALLET_VERSION_MAJOR		0
#define KWALLET_VERSION_MINOR		0

#define KWALLET_CIPHER_BLOWFISH_CBC	0
#define KWALLET_CIPHER_3DES_CBC		1 // unsupported

#define KWALLET_HASH_SHA1		0
#define KWALLET_HASH_MD5		1 // unsupported


using namespace KWallet;

#define KWMAGIC "KWALLET\n\r\0\r\n"
#define KWMAGIC_LEN 12

Backend::Backend(const QString& name, bool isPath) : _name(name), _ref(0) {
	KGlobal::dirs()->addResourceType("kwallet", "share/apps/kwallet");
	if (isPath) {
		_path = name;
	} else {
		_path = KGlobal::dirs()->saveLocation("kwallet") + "/" + _name + ".kwl";
	}

	_open = false;
}


Backend::~Backend() {
	if (_open) {
		close();
	}
}


int Backend::close() {
	for (FolderMap::ConstIterator i = _entries.begin(); i != _entries.end(); ++i) {
		for (EntryMap::ConstIterator j = i.data().begin(); j != i.data().end(); ++j) {
			delete j.data();
		}
	}
	_entries.clear();

return 0;
}


static int getRandomBlock(QByteArray& randBlock) {
	// First try /dev/urandom
	if (QFile::exists("/dev/urandom")) {
		QFile devrand("/dev/urandom");
		if (devrand.open(QIODevice::ReadOnly)) {
			int rc = devrand.readBlock(randBlock.data(), randBlock.size());

			if (rc != randBlock.size()) {
				return -3;		// not enough data read
			}

			return 0;
		}
	}

	// If that failed, try /dev/random
	// FIXME: open in noblocking mode!
	if (QFile::exists("/dev/random")) {
		QFile devrand("/dev/random");
		if (devrand.open(QIODevice::ReadOnly)) {
		int rc = 0;
		int cnt = 0;

			do {
				int rc2 = devrand.readBlock(randBlock.data() + rc, randBlock.size());

				if (rc2 < 0) {
					return -3;	// read error
				}

				rc += rc2;
				cnt++;
				if (cnt > randBlock.size()) {
					return -4;	// reading forever?!
				}
			} while(rc < randBlock.size());

			return 0;
		}
	}

	// EGD method
	char *randFilename;
	if ((randFilename = getenv("RANDFILE"))) {
		if (QFile::exists(randFilename)) {
			QFile devrand(randFilename);
			if (devrand.open(QIODevice::ReadOnly)) {
				int rc = devrand.readBlock(randBlock.data(), randBlock.size());
				if (rc != randBlock.size()) {
					return -3;      // not enough data read
				}
				return 0;
			}
		}
	}

	// Couldn't get any random data!!

	return -1;
}


// this should be SHA-512 for release probably
static int password2hash(const QByteArray& password, QByteArray& hash) {
	SHA1 sha;
	int shasz = sha.size() / 8;

	assert(shasz >= 20);

	QByteArray block1(shasz);

	sha.process(password.data(), qMin(password.size(), 16));

	// To make brute force take longer
	for (int i = 0; i < 2000; i++) {
		memcpy(block1.data(), sha.hash(), shasz);
		sha.reset();
		sha.process(block1.data(), shasz);
	}

	sha.reset();

	if (password.size() > 16) {
		sha.process(password.data() + 16, qMin(password.size() - 16, 16));
		QByteArray block2(shasz);
		// To make brute force take longer
		for (int i = 0; i < 2000; i++) {
			memcpy(block2.data(), sha.hash(), shasz);
			sha.reset();
			sha.process(block2.data(), shasz);
		}

		sha.reset();

		if (password.size() > 32) {
			sha.process(password.data() + 32, qMin(password.size() - 32, 16));

			QByteArray block3(shasz);
			// To make brute force take longer
			for (int i = 0; i < 2000; i++) {
				memcpy(block3.data(), sha.hash(), shasz);
				sha.reset();
				sha.process(block3.data(), shasz);
			}

			sha.reset();

			if (password.size() > 48) {
				sha.process(password.data() + 48, password.size() - 48);

				QByteArray block4(shasz);
				// To make brute force take longer
				for (int i = 0; i < 2000; i++) {
					memcpy(block4.data(), sha.hash(), shasz);
					sha.reset();
					sha.process(block4.data(), shasz);
				}

				sha.reset();
				// split 14/14/14/14
				hash.resize(56);
				memcpy(hash.data(),      block1.data(), 14);
				memcpy(hash.data() + 14, block2.data(), 14);
				memcpy(hash.data() + 28, block3.data(), 14);
				memcpy(hash.data() + 42, block4.data(), 14);
				block4.fill(0);
			} else {
				// split 20/20/16
				hash.resize(56);
				memcpy(hash.data(),      block1.data(), 20);
				memcpy(hash.data() + 20, block2.data(), 20);
				memcpy(hash.data() + 40, block3.data(), 16);
			}
			block3.fill(0);
		} else {
			// split 20/20
			hash.resize(40);
			memcpy(hash.data(),      block1.data(), 20);
			memcpy(hash.data() + 20, block2.data(), 20);
		}
		block2.fill(0);
	} else {
		// entirely block1
		hash.resize(20);
		memcpy(hash.data(), block1.data(), 20);
	}

	block1.fill(0);

	return 0;
}


bool Backend::exists(const QString& wallet) {
QString path = KGlobal::dirs()->saveLocation("kwallet") + "/" + wallet + ".kwl";
	// Note: 60 bytes is presently the minimum size of a wallet file.
	//       Anything smaller is junk.
return QFile::exists(path) && QFileInfo(path).size() >= 60;
}


QString Backend::openRCToString(int rc) {
	switch (rc) {
		case -255:
			return i18n("Already open.");
		case -2:
			return i18n("Error opening file.");
		case -3:
			return i18n("Not a wallet file.");
		case -4:
			return i18n("Unsupported file format revision.");
		case -42:
			return i18n("Unknown encryption scheme.");
		case -43:
			return i18n("Corrupt file?");
		case -8:
			return i18n("Error validating wallet integrity. Possibly corrupted.");
		case -5:
		case -7:
		case -9:
			return i18n("Read error - possibly incorrect password.");
		case -6:
			return i18n("Decryption error.");
		default:
			return QString();
	}
}


int Backend::open(const QByteArray& password) {

	if (_open) {
		return -255;  // already open
	}

	QByteArray passhash;

	// No wallet existed.  Let's create it.
	// Note: 60 bytes is presently the minimum size of a wallet file.
	//       Anything smaller is junk and should be deleted.
	if (!QFile::exists(_path) || QFileInfo(_path).size() < 60) {
		QFile newfile(_path);
		if (!newfile.open(QIODevice::ReadWrite)) {
			return -2;   // error opening file
		}
		newfile.close();
		_open = true;
		sync(password);
		return 1;          // new file opened, but OK
	}

	QFile db(_path);

	if (!db.open(QIODevice::ReadOnly)) {
		return -2;         // error opening file
	}

	char magicBuf[KWMAGIC_LEN];
	db.readBlock(magicBuf, KWMAGIC_LEN);
	if (memcmp(magicBuf, KWMAGIC, KWMAGIC_LEN) != 0) {
		return -3;         // bad magic
	}

	db.readBlock(magicBuf, 4);

	// First byte is major version, second byte is minor version
	if (magicBuf[0] != KWALLET_VERSION_MAJOR) {
		return -4;         // unknown version
	}

	if (magicBuf[1] != KWALLET_VERSION_MINOR) {
		return -4;	   // unknown version
	}

	if (magicBuf[2] != KWALLET_CIPHER_BLOWFISH_CBC) {
		return -42;	   // unknown cipher
	}

	if (magicBuf[3] != KWALLET_HASH_SHA1) {
		return -42;	   // unknown hash
	}

	_hashes.clear();
	// Read in the hashes
	QDataStream hds(&db);
	quint32 n;
	hds >> n;
	if (n > 0xffff) { // sanity check
		return -43;
	}

	for (size_t i = 0; i < n; ++i) {
		KMD5::Digest d, d2; // judgment day
		MD5Digest ba;
		QMap<MD5Digest,QList<MD5Digest> >::iterator it;
		quint32 fsz;
		if (hds.atEnd()) return -43;
		hds.readRawBytes(reinterpret_cast<char *>(d), 16);
		hds >> fsz;
		ba.duplicate(reinterpret_cast<char *>(d), 16);
		it = _hashes.insert(ba, QList<MD5Digest>());
		for (size_t j = 0; j < fsz; ++j) {
			hds.readRawBytes(reinterpret_cast<char *>(d2), 16);
			ba.duplicate(reinterpret_cast<char *>(d2), 16);
			(*it).append(ba);
		}
	}

	// Read in the rest of the file.
	QByteArray encrypted = db.readAll();
	assert(encrypted.size() < db.size());

	BlowFish _bf;
	CipherBlockChain bf(&_bf);
	int blksz = bf.blockSize();
	if ((encrypted.size() % blksz) != 0) {
		return -5;	   // invalid file structure
	}

	// Decrypt the encrypted data
	passhash.resize(bf.keyLen()/8);
	password2hash(password, passhash);

	bf.setKey((void *)passhash.data(), passhash.size()*8);

	if (!encrypted.data()) {
		passhash.fill(0);
		encrypted.fill(0);
		return -7; // file structure error
	}

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

	fsize |= (long(*t) << 24) & 0xff000000;
	t++;
	fsize |= (long(*t) << 16) & 0x00ff0000;
	t++;
	fsize |= (long(*t) <<  8) & 0x0000ff00;
	t++;
	fsize |= long(*t) & 0x000000ff;
	t++;

	if (fsize < 0 || fsize > long(encrypted.size()) - blksz - 4) {
		//kDebug() << "fsize: " << fsize << " encrypted.size(): " << encrypted.size() << " blksz: " << blksz << endl;
		encrypted.fill(0);
		return -9;         // file structure error.
	}

	// compute the hash ourself
	SHA1 sha;
	sha.process(t, fsize);
	const char *testhash = (const char *)sha.hash();

	// compare hashes
	int sz = encrypted.size();
	for (int i = 0; i < 20; i++) {
		if (testhash[i] != encrypted[sz - 20 + i]) {
			encrypted.fill(0);
			sha.reset();
			return -8;         // hash error.
		}
	}

	sha.reset();

	// chop off the leading blksz+4 bytes
	QByteArray tmpenc;
	tmpenc.duplicate(encrypted.data()+blksz+4, fsize);
	encrypted.fill(0);
	encrypted.duplicate(tmpenc.data(), tmpenc.size());
	tmpenc.fill(0);

	// Load the data structures up
	QDataStream eStream(encrypted);

	while (!eStream.atEnd()) {
		QString folder;
		quint32 n;

		eStream >> folder;
		eStream >> n;

		// Force initialisation
		_entries[folder].clear();

		for (size_t i = 0; i < n; i++) {
			QString key;
			KWallet::Wallet::EntryType et = KWallet::Wallet::Unknown;
			Entry *e = new Entry;
			eStream >> key;
			qint32 x = 0; // necessary to read properly
			eStream >> x;
			et = static_cast<KWallet::Wallet::EntryType>(x);

			switch (et) {
			case KWallet::Wallet::Password:
			case KWallet::Wallet::Stream:
			case KWallet::Wallet::Map:
			break;
			default: // Unknown entry
				delete e;
				continue;
			}

			QByteArray a;
		 	eStream >> a;
			e->setValue(a);
			e->setType(et);
			e->setKey(key);
			_entries[folder][key] = e;
		}
	}

	_open = true;
	return 0;
}


int Backend::sync(const QByteArray& password) {
	if (!_open) {
		return -255;  // not open yet
	}

	KSaveFile sf(_path, 0600);
	QFile *qf = sf.file();

	if (!qf) {
		sf.abort();
		return -1;		// error opening file
	}

	qf->writeBlock(KWMAGIC, KWMAGIC_LEN);

	// Write the version number
	QByteArray version(4);
	version[0] = KWALLET_VERSION_MAJOR;
	version[1] = KWALLET_VERSION_MINOR;
	version[2] = KWALLET_CIPHER_BLOWFISH_CBC;
	version[3] = KWALLET_HASH_SHA1;
	qf->writeBlock(version, 4);

	// Holds the hashes we write out
	QByteArray hashes;
	QDataStream hashStream(&hashes, QIODevice::WriteOnly);
	KMD5 md5;
	hashStream << static_cast<quint32>(_entries.count());

	// Holds decrypted data prior to encryption
	QByteArray decrypted;

	// FIXME: we should estimate the amount of data we will write in each
	// buffer and resize them approximately in order to avoid extra
	// resizes.

	// populate decrypted
	QDataStream dStream(&decrypted, QIODevice::WriteOnly);
	for (FolderMap::ConstIterator i = _entries.begin(); i != _entries.end(); ++i) {
		dStream << i.key();
		dStream << static_cast<quint32>(i.data().count());

		md5.reset();
		md5.update(i.key().toUtf8());
		hashStream.writeRawBytes(reinterpret_cast<const char*>(&(md5.rawDigest()[0])), 16);
		hashStream << static_cast<quint32>(i.data().count());

		for (EntryMap::ConstIterator j = i.data().begin(); j != i.data().end(); ++j) {
			dStream << j.key();
			dStream << static_cast<qint32>(j.data()->type());
			dStream << j.data()->value();

			md5.reset();
			md5.update(j.key().toUtf8());
			hashStream.writeRawBytes(reinterpret_cast<const char*>(&(md5.rawDigest()[0])), 16);
		}
	}

	qf->writeBlock(hashes, hashes.size());

	// calculate the hash of the file
	SHA1 sha;
	BlowFish _bf;
	CipherBlockChain bf(&_bf);

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
		sf.abort();
		return -3;		// Fatal error: can't get random
	}

	for (int i = 0; i < blksz; i++) {
		wholeFile[i] = randBlock[i];
	}

	for (int i = 0; i < 4; i++) {
		wholeFile[(int)(i+blksz)] = (decrypted.size() >> 8*(3-i))&0xff;
	}

	for (int i = 0; i < decrypted.size(); i++) {
		wholeFile[(int)(i+blksz+4)] = decrypted[i];
	}

	for (int i = 0; i < delta; i++) {
		wholeFile[(int)(i+blksz+4+decrypted.size())] = randBlock[(int)(i+blksz)];
	}

	const char *hash = (const char *)sha.hash();
	for (int i = 0; i < 20; i++) {
		wholeFile[(int)(newsize - 20 + i)] = hash[i];
	}

	sha.reset();
	decrypted.fill(0);

	// hash the passphrase
	QByteArray passhash;
	password2hash(password, passhash);

	// encrypt the data
	if (!bf.setKey(passhash.data(), passhash.size() * 8)) {
		passhash.fill(0);
		wholeFile.fill(0);
		sf.abort();
		return -2;
	}

	int rc = bf.encrypt(wholeFile.data(), wholeFile.size());
	if (rc < 0) {
		passhash.fill(0);
		wholeFile.fill(0);
		sf.abort();
		return -2;	// encrypt error
	}

	passhash.fill(0);        // passhash is UNUSABLE NOW

	// write the file
	qf->writeBlock(wholeFile, wholeFile.size());
	if (!sf.close()) {
		wholeFile.fill(0);
		sf.abort();
		return -4; // write error
	}

	wholeFile.fill(0);

return 0;
}


int Backend::close(const QByteArray& password) {
	int rc = sync(password);
	_open = false;
	if (rc != 0) {
		return rc;
	}
	return close();
}


const QString& Backend::walletName() const {
	return _name;
}


bool Backend::isOpen() const {
	return _open;
}


QStringList Backend::folderList() const {
	return _entries.keys();
}


QStringList Backend::entryList() const {
	return _entries[_folder].keys();
}


Entry *Backend::readEntry(const QString& key) {
Entry *rc = 0L;

	if (_open && hasEntry(key)) {
		rc = _entries[_folder][key];
	}

return rc;
}


QList<Entry*> Backend::readEntryList(const QString& key) {
	QList<Entry*> rc;

	if (!_open) {
		return rc;
	}

	QRegExp re(key, true, true);

	const EntryMap& map = _entries[_folder];
	for (EntryMap::ConstIterator i = map.begin(); i != map.end(); ++i) {
		if (re.exactMatch(i.key())) {
			rc.append(i.data());
		}
	}
	return rc;
}


bool Backend::createFolder(const QString& f) {
	if (_entries.contains(f)) {
		return false;
	}

	_entries.insert(f, EntryMap());

	KMD5 folderMd5;
	folderMd5.update(f.toUtf8());
	_hashes.insert(MD5Digest(folderMd5.rawDigest()), QList<MD5Digest>());

return true;
}


int Backend::renameEntry(const QString& oldName, const QString& newName) {
EntryMap& emap = _entries[_folder];
EntryMap::Iterator oi = emap.find(oldName);
EntryMap::Iterator ni = emap.find(newName);

	if (oi != emap.end() && ni == emap.end()) {
		Entry *e = oi.data();
		emap.remove(oi);
		emap[newName] = e;

		KMD5 folderMd5;
		folderMd5.update(_folder.toUtf8());

		HashMap::iterator i = _hashes.find(MD5Digest(folderMd5.rawDigest()));
		if (i != _hashes.end()) {
			KMD5 oldMd5, newMd5;
			oldMd5.update(oldName.toUtf8());
			newMd5.update(newName.toUtf8());
			i.data().remove(MD5Digest(oldMd5.rawDigest()));
			i.data().append(MD5Digest(newMd5.rawDigest()));
		}
		return 0;
	}

return -1;
}


void Backend::writeEntry(Entry *e) {
	if (!_open)
		return;

	if (!hasEntry(e->key())) {
		_entries[_folder][e->key()] = new Entry;
	}
	_entries[_folder][e->key()]->copy(e);

	KMD5 folderMd5;
	folderMd5.update(_folder.toUtf8());

	HashMap::iterator i = _hashes.find(MD5Digest(folderMd5.rawDigest()));
	if (i != _hashes.end()) {
		KMD5 md5;
		md5.update(e->key().toUtf8());
		i.data().append(MD5Digest(md5.rawDigest()));
	}
}


bool Backend::hasEntry(const QString& key) const {
	return _entries.contains(_folder) && _entries[_folder].contains(key);
}


bool Backend::removeEntry(const QString& key) {
	if (!_open) {
		return false;
	}

	FolderMap::Iterator fi = _entries.find(_folder);
	EntryMap::Iterator ei = fi.data().find(key);

	if (fi != _entries.end() && ei != fi.data().end()) {
		delete ei.data();
		fi.data().remove(ei);
		KMD5 folderMd5;
		folderMd5.update(_folder.toUtf8());

		HashMap::iterator i = _hashes.find(MD5Digest(folderMd5.rawDigest()));
		if (i != _hashes.end()) {
			KMD5 md5;
			md5.update(key.toUtf8());
			i.data().remove(MD5Digest(md5.rawDigest()));
		}
		return true;
	}

return false;
}


bool Backend::removeFolder(const QString& f) {
	if (!_open) {
		return false;
	}

	FolderMap::Iterator fi = _entries.find(f);

	if (fi != _entries.end()) {
		if (_folder == f) {
			_folder.clear();
		}

		for (EntryMap::Iterator ei = fi.data().begin(); ei != fi.data().end(); ++ei) {
			delete ei.data();
		}

		_entries.remove(fi);

		KMD5 folderMd5;
		folderMd5.update(f.toUtf8());
		_hashes.erase(MD5Digest(folderMd5.rawDigest()));
		return true;
	}

return false;
}


bool Backend::folderDoesNotExist(const QString& folder) const {
	KMD5 md5;
	md5.update(folder.toUtf8());
	return !_hashes.contains(MD5Digest(md5.rawDigest()));
}


bool Backend::entryDoesNotExist(const QString& folder, const QString& entry) const {
	KMD5 md5;
	md5.update(folder.toUtf8());
	HashMap::const_iterator i = _hashes.find(MD5Digest(md5.rawDigest()));
	if (i != _hashes.end()) {
		md5.reset();
		md5.update(entry.toUtf8());
		return !i.data().contains(MD5Digest(md5.rawDigest()));
	}
	return true;
}


