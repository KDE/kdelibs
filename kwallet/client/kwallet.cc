/* This file is part of the KDE project
 *
 * Copyright (C) 2002-2003 George Staikos <staikos@kde.org>
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

#include "kwallet.h"
#include <kdebug.h>
#include <dcopclient.h>
#include <dcopref.h>

#include <assert.h>


using namespace KWallet;


const QString Wallet::LocalWallet() {
// FIXME: observe preferences to store local+network together
return "kdewallet";
}

const QString Wallet::NetworkWallet() {
// FIXME: observe preferences to store local+network together
return "kdewallet";
}

const QString Wallet::PasswordFolder("Passwords");

const QString Wallet::FormDataFolder("Form Data");



Wallet::Wallet(int handle, const QString& name)
: QObject(0L), DCOPObject(), d(0L), _name(name), _handle(handle) {
	_dcopRef = new DCOPRef("kded", "kwalletd");

	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletClosed(int)", "slotWalletClosed(int)", false);

	// Verify that the wallet is still open
	DCOPReply r = _dcopRef->call("isOpen", _handle);
	if (r.isValid()) {
		bool rc = false;
		r.get(rc);
		if (!rc) {
			_handle = -1;
			_name = QString::null;
		}
	}
}


Wallet::~Wallet() {
	if (_handle != -1) {
		_dcopRef->call("close", _handle, false);
		_handle = -1;
		_folder = QString::null;
		_name = QString::null;
	}

	delete _dcopRef;
	_dcopRef = 0L;
}


QStringList Wallet::walletList() {
DCOPReply r = DCOPRef("kded", "kwalletd").call("wallets");
QStringList rc;
	if (r.isValid()) {
		r.get(rc);
	}
return rc;
}


bool Wallet::isOpen(const QString& name) {
DCOPReply r = DCOPRef("kded", "kwalletd").call("isOpen", name);
bool rc = false;
	if (r.isValid()) {
		r.get(rc);
	}
return rc;
}


int Wallet::closeWallet(const QString& name, bool force) {
DCOPReply r = DCOPRef("kded", "kwalletd").call("close", name, force);
int rc = -1;
	if (r.isValid()) {
		r.get(rc);
	}
return rc;
}


int Wallet::deleteWallet(const QString& name) {
DCOPReply r = DCOPRef("kded", "kwalletd").call("deleteWallet", name);
int rc = -1;
	if (r.isValid()) {
		r.get(rc);
	}
return rc;
}


Wallet *Wallet::openWallet(const QString& name) {
DCOPReply r = DCOPRef("kded", "kwalletd").call("open", name);
	if (r.isValid()) {
		int drc = -1;
		r.get(drc);
		if (drc != -1) {
			return new Wallet(drc, name);
		}
	}
return 0;
}


int Wallet::lockWallet() {
	if (_handle == -1) {
		return -1;
	}

	DCOPReply r = _dcopRef->call("close", _handle, true);
	_handle = -1;
	_folder = QString::null;
	_name = QString::null;
	if (r.isValid()) {
		int drc = -1;
		r.get(drc);
		return drc;
	}
	return -1;
}


const QString& Wallet::walletName() const {
	return _name;
}


bool Wallet::isOpen() const {
	return _handle != -1;
}


void Wallet::requestChangePassword() {
	if (_handle == -1) {
		return;
	}

	_dcopRef->send("changePassword", _handle);
}


void Wallet::slotWalletClosed(int handle) {
	if (_handle == handle) {
		_handle = -1;
		_folder = QString::null;
		_name = QString::null;
		emit walletClosed();
	}
}


QStringList Wallet::folderList() {
QStringList rc;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("folderList", _handle);
	if (r.isValid()) {
		r.get(rc);
	}

return rc;
}


bool Wallet::hasFolder(const QString& f) {
bool rc = false;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("hasFolder", _handle, f);
	if (r.isValid()) {
		r.get(rc);
	}

return rc;
}


bool Wallet::setFolder(const QString& f) {
bool rc = false;

	if (_handle == -1) {
		return rc;
	}

// Don't do this - the folder could have disappeared?
#if 0
	if (f == _folder) {
		return true;
	}
#endif

	if (hasFolder(f)) {
		_folder = f;
		rc = true;
	}

return rc;
}


bool Wallet::removeFolder(const QString& f) {
bool rc = false;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("removeFolder", _handle, f);
	if (r.isValid()) {
		r.get(rc);
	}

	if (_folder == f) {
		setFolder(QString::null);
	}

return rc;
}


const QString& Wallet::currentFolder() const {
return _folder;
}


int Wallet::readEntry(const QString& key, QByteArray& value) {
int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("readEntry", _handle, _folder, key);
	if (r.isValid()) {
		r.get(value);
		rc = 0;
	}

return rc;
}


int Wallet::readMap(const QString& key, QMap<QString,QString>& value) {
int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("readMap", _handle, _folder, key);
	if (r.isValid()) {
		r.get(value);
		rc = 0;
	}

return rc;
}


int Wallet::readPassword(const QString& key, QString& value) {
int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("readPassword", _handle, _folder, key);
	if (r.isValid()) {
		r.get(value);
		rc = 0;
	}

return rc;
}


int Wallet::writeEntry(const QString& key, const QByteArray& value) {
int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("writeEntry", _handle, _folder, key, value);
	if (r.isValid()) {
		r.get(rc);
	}

return rc;
}


int Wallet::writeMap(const QString& key, const QMap<QString,QString>& value) {
int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("writeMap", _handle, _folder, key, value);
	if (r.isValid()) {
		r.get(rc);
	}

return rc;
}


int Wallet::writePassword(const QString& key, const QString& value) {
int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("writePassword", _handle, _folder, key, value);
	if (r.isValid()) {
		r.get(rc);
	}

return rc;
}


bool Wallet::hasEntry(const QString& key) {
bool rc = false;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("hasEntry", _handle, _folder, key);
	if (r.isValid()) {
		r.get(rc);
	}

return rc;
}


int Wallet::removeEntry(const QString& key) {
bool rc = false;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("removeEntry", _handle, _folder, key);
	if (r.isValid()) {
		r.get(rc);
	}

return rc;
}


Wallet::EntryType Wallet::entryType(const QString& key) {
long rc = 0;

	if (_handle == -1) {
		return Wallet::Unknown;
	}

	DCOPReply r = _dcopRef->call("entryType", _handle, _folder, key);
	if (r.isValid()) {
		r.get(rc);
	}

return static_cast<EntryType>(rc);
}


void Wallet::virtual_hook(int, void*) {
	//BASE::virtual_hook( id, data );
}

#include "kwallet.moc"
