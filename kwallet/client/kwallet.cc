/* This file is part of the KDE project
 *
 * Copyright (C) 2002-2004 George Staikos <staikos@kde.org>
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

#include "kwallettypes.h"
#include "kwallet.h"
#include <kconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <q3popupmenu.h>
#include <qapplication.h>

#include <assert.h>

using namespace KWallet;


const QString Wallet::LocalWallet() {
	KConfig cfg("kwalletrc");
	cfg.setGroup("Wallet");
	if (!cfg.readBoolEntry("Use One Wallet", true)) {
		QString tmp = cfg.readEntry("Local Wallet", "localwallet");
		if (tmp.isEmpty()) {
			return "localwallet";
		}
		return tmp;
	}

	QString tmp = cfg.readEntry("Default Wallet", "kdewallet");
	if (tmp.isEmpty()) {
		return "kdewallet";
	}
	return tmp;
}

const QString Wallet::NetworkWallet() {
	KConfig cfg("kwalletrc");
	cfg.setGroup("Wallet");

	QString tmp = cfg.readEntry("Default Wallet", "kdewallet");
	if (tmp.isEmpty()) {
		return "kdewallet";
	}
	return tmp;
}

const QString Wallet::PasswordFolder() {
	return "Passwords";
}

const QString Wallet::FormDataFolder() {
	return "Form Data";
}



Wallet::Wallet(int handle, const QString& name)
: QObject(0L), DCOPObject(), d(0L), _name(name), _handle(handle) {

	_dcopRef = new DCOPRef("kded", "kwalletd");

	_dcopRef->dcopClient()->setNotifications(true);
	connect(_dcopRef->dcopClient(),
			SIGNAL(applicationRemoved(const QByteArray&)),
			this,
			SLOT(slotAppUnregistered(const QByteArray&)));

	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletClosed(int)", "slotWalletClosed(int)", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "folderListUpdated(QString)", "slotFolderListUpdated(QString)", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "folderUpdated(QString, QString)", "slotFolderUpdated(QString, QString)", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "applicationDisconnected(QString, QCString)", "slotApplicationDisconnected(QString, QCString)", false);

	// Verify that the wallet is still open
	if (_handle != -1) {
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


void Wallet::changePassword(const QString& name, WId w) {
	DCOPRef("kded", "kwalletd").send("changePassword", name, uint(w));
}


bool Wallet::isEnabled() {
	DCOPReply r = DCOPRef("kded", "kwalletd").call("isEnabled");
	bool rc = false;
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


Wallet *Wallet::openWallet(const QString& name, WId w, OpenType ot) {
	if (ot == Asynchronous) {
		Wallet *wallet = new Wallet(-1, name);
		DCOPRef("kded", "kwalletd").send("openAsynchronous", name, wallet->objId(), uint(w));
		return wallet;
	}

        // avoid deadlock if the app has some popup open (#65978/#71048)
        while( QWidget* widget = qApp->activePopupWidget())
            widget->close();

	bool isPath = ot == Path;
	DCOPReply r;

	if (isPath) {
		r = DCOPRef("kded", "kwalletd").call("openPath", name, uint(w));
	} else {
		r = DCOPRef("kded", "kwalletd").call("open", name, uint(w));
	}

	if (r.isValid()) {
		int drc = -1;
		r.get(drc);
		if (drc != -1) {
			return new Wallet(drc, name);
		}
	}

	return 0;
}


bool Wallet::disconnectApplication(const QString& wallet, const DCOPCString& app) {
	DCOPReply r = DCOPRef("kded", "kwalletd").call("disconnectApplication", wallet, app);
	bool rc = false;
	if (r.isValid()) {
		r.get(rc);
	}
	return rc;
}


QStringList Wallet::users(const QString& name) {
	DCOPReply r = DCOPRef("kded", "kwalletd").call("users", name);
	QStringList drc;
	if (r.isValid()) {
		r.get(drc);
	}
	return drc;
}


int Wallet::sync() {
	if (_handle == -1) {
		return -1;
	}

	_dcopRef->call("sync", _handle);
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


void Wallet::requestChangePassword(WId w) {
	if (_handle == -1) {
		return;
	}

	_dcopRef->send("changePassword", _name, uint(w));
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


QStringList Wallet::entryList() {
	QStringList rc;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("entryList", _handle, _folder);
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


bool Wallet::createFolder(const QString& f) {
	bool rc = true;

	if (_handle == -1) {
		return false;
	}

	if (!hasFolder(f)) {
		DCOPReply r = _dcopRef->call("createFolder", _handle, f);
		if (r.isValid()) {
			r.get(rc);
		}
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


int Wallet::readEntryList(const QString& key, QMap<QString, QByteArray>& value) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("readEntryList", _handle, _folder, key);
	if (r.isValid()) {
		r.get(value);
		rc = 0;
	}

	return rc;
}


int Wallet::renameEntry(const QString& oldName, const QString& newName) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("renameEntry", _handle, _folder, oldName, newName);
	if (r.isValid()) {
		r.get(rc);
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
		QByteArray v;
		r.get(v);
		if (!v.isEmpty()) {
			QDataStream ds(v);
			ds >> value;
		}
		rc = 0;
	}

	return rc;
}


int Wallet::readMapList(const QString& key, QMap<QString, QMap<QString, QString> >& value) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("readMapList", _handle, _folder, key);
	if (r.isValid()) {
		QMap<QString,QByteArray> unparsed;
		r.get(unparsed);
		for (QMap<QString,QByteArray>::ConstIterator i = unparsed.begin(); i != unparsed.end(); ++i) {
			if (!i.data().isEmpty()) {
				QDataStream ds(i.data());
				QMap<QString,QString> v;
				ds >> v;
				value.insert(i.key(), v);
			}
		}
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


int Wallet::readPasswordList(const QString& key, QMap<QString, QString>& value) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("readPasswordList", _handle, _folder, key);
	if (r.isValid()) {
		r.get(value);
		rc = 0;
	}

	return rc;
}


int Wallet::writeEntry(const QString& key, const QByteArray& value, EntryType entryType) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("writeEntry", _handle, _folder, key, value, int(entryType));
	if (r.isValid()) {
		r.get(rc);
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

	QByteArray a;
	QDataStream ds(&a, QIODevice::WriteOnly);
	ds << value;
	DCOPReply r = _dcopRef->call("writeMap", _handle, _folder, key, a);
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
	int rc = -1;

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
	int rc = 0;

	if (_handle == -1) {
		return Wallet::Unknown;
	}

	DCOPReply r = _dcopRef->call("entryType", _handle, _folder, key);
	if (r.isValid()) {
		r.get(rc);
	}

	return static_cast<EntryType>(rc);
}


void Wallet::slotAppUnregistered(const QByteArray& app) {
	if (_handle >= 0 && app == "kded") {
		slotWalletClosed(_handle);
	}
}


void Wallet::slotFolderUpdated(const QString& wallet, const QString& folder) {
	if (_name == wallet) {
		emit folderUpdated(folder);
	}
}


void Wallet::slotFolderListUpdated(const QString& wallet) {
	if (_name == wallet) {
		emit folderListUpdated();
	}
}


void Wallet::slotApplicationDisconnected(const QString& wallet, const DCOPCString& application) {
	if (_handle >= 0
			&& _name == wallet
			&& application == _dcopRef->dcopClient()->appId()) {
		slotWalletClosed(_handle);
	}
}


void Wallet::walletOpenResult(int id) {
	if (_handle != -1) {
		// This is BAD.
		return;
	}

	if (id > 0) {
		_handle = id;
		emit walletOpened(true);
	} else if (id < 0) {
		emit walletOpened(false);
	} // id == 0 => wait
}


bool Wallet::folderDoesNotExist(const QString& wallet, const QString& folder) {
DCOPReply r = DCOPRef("kded", "kwalletd").call("folderDoesNotExist", wallet, folder);
bool rc = true;
	if (r.isValid()) {
		r.get(rc);
	}
return rc;
}


bool Wallet::keyDoesNotExist(const QString& wallet, const QString& folder, const QString& key) {
DCOPReply r = DCOPRef("kded", "kwalletd").call("keyDoesNotExist", wallet, folder, key);
bool rc = true;
	if (r.isValid()) {
		r.get(rc);
	}
return rc;
}


void Wallet::virtual_hook(int, void*) {
	//BASE::virtual_hook( id, data );
}

#include "kwallet.moc"
