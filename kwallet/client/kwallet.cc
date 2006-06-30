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

#include "kwallet.h"
#include <kconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <qapplication.h>
#include <qwidget.h>
#include <QtDBus/QtDBus>

#include <assert.h>

using namespace KWallet;

typedef QMap<QString, QString> StringStringMap;
Q_DECLARE_METATYPE(StringStringMap)
typedef QMap<QString, StringStringMap> StringToStringStringMapMap;
Q_DECLARE_METATYPE(StringToStringStringMapMap)
typedef QMap<QString, QByteArray> StringByteArrayMap;
Q_DECLARE_METATYPE(StringByteArrayMap)

static void registerTypes()
{
	static bool registered = false;
	if (!registered) {
		qDBusRegisterMetaType<StringStringMap>();
		qDBusRegisterMetaType<StringToStringStringMapMap>();
		qDBusRegisterMetaType<StringByteArrayMap>();
		registered = true;
	}
}

static QDBusInterface *findWallet()
{
	return new QDBusInterface("org.kde.kded", "/modules/kwalletd", "org.kde.KWallet");
}

const QString Wallet::LocalWallet() {
	KConfig cfg("kwalletrc", true);
	cfg.setGroup("Wallet");
	if (!cfg.readEntry("Use One Wallet", true)) {
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
	KConfig cfg("kwalletrc", true);
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
: QObject(0L), d(0L), _name(name), _handle(handle) {

	_wallet = findWallet();
	_wallet->setParent(this);

	connect(QDBus::sessionBus().interface(),
                SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                this,
                SLOT(slotAppUnregistered(QString,QString,QString)));

	connect(_wallet, SIGNAL(walletClosed(int)), SLOT(slotWalletClosed(int)));
	connect(_wallet, SIGNAL(folderListUpdated(QString)), SLOT(slotFolderListUpdated(QString)));
	connect(_wallet, SIGNAL(folderUpdated(QString,QString)), SLOT(slotFolderUpdated(QString, QString)));
	connect(_wallet, SIGNAL(applicationDisconnected(QString, QCString)), SLOT(slotApplicationDisconnected(QString, QCString)));

	// Verify that the wallet is still open
	if (_handle != -1) {
		QDBusReply<bool> r = _wallet->call("isOpen", _handle);
		if (r.isValid() && !r) {
			_handle = -1;
			_name.clear();
		}
	}
}


Wallet::~Wallet() {
	if (_handle != -1) {
		_wallet->call("close", _handle, false);
		_handle = -1;
		_folder.clear();
		_name.clear();
	}
}


QStringList Wallet::walletList() {
        QDBusInterface wallet("org.kde.kded", "/modules/kwalletd", "org.kde.KWallet");
	QDBusReply<QStringList> r = wallet.call("wallets");
	return r;					// default is QStringList()
}


void Wallet::changePassword(const QString& name, WId w) {
	QDBusInterface wallet("org.kde.kded", "/modules/kwalletd", "org.kde.KWallet");
	wallet.call("changePassword", name, qlonglong(w));
}


bool Wallet::isEnabled() {
	QDBusInterface wallet("org.kde.kded", "/modules/kwalletd", "org.kde.KWallet");
	QDBusReply<bool> r = wallet.call("isEnabled");
	return r;					// default is false
}


bool Wallet::isOpen(const QString& name) {
	QDBusInterface wallet("org.kde.kded", "/modules/kwalletd", "org.kde.KWallet");
	QDBusReply<bool> r = wallet.call("isOpen", name);
	return r;					// default is false
}


int Wallet::closeWallet(const QString& name, bool force) {
	QDBusInterface wallet("org.kde.kded", "/modules/kwalletd", "org.kde.KWallet");
	QDBusReply<int> r = wallet.call("close", name, force);
	int rc = -1;
	if (r.isValid()) {
		rc = r;
	}
	return rc;
}


int Wallet::deleteWallet(const QString& name) {
	QDBusInterface wallet("org.kde.kded", "/modules/kwalletd", "org.kde.KWallet");
	QDBusReply<int> r = wallet.call("deleteWallet", name);
	int rc = -1;
	if (r.isValid()) {
		rc = r;
	}
	return rc;
}


Wallet *Wallet::openWallet(const QString& name, WId w, OpenType ot) {
	if (ot == Asynchronous) {
		Wallet *wallet = new Wallet(-1, name);

		// place an asynchronous call
		QVariantList args;
		args << name << qlonglong(w);
		wallet->_wallet->callWithArgumentList("open", args, wallet, SLOT(walletOpenResult(int)));

		return wallet;
	}

		// avoid deadlock if the app has some popup open (#65978/#71048)
		while( QWidget* widget = qApp->activePopupWidget())
			widget->close();

	bool isPath = ot == Path;
	QDBusInterface wallet("org.kde.kded", "/modules/kwalletd", "org.kde.KWallet");
	QDBusReply<int> r = wallet.call(isPath ? "openPath" : "open",
									 name, qlonglong(w));
	if (r.isValid()) {
		int drc = r;
		if (drc != -1) {
			return new Wallet(drc, name);
		}
	}

	return 0;
}


bool Wallet::disconnectApplication(const QString& wallet, const QString& app) {
	QDBusInterface w("org.kde.kded", "/modules/kwalletd", "org.kde.KWallet");
	QDBusReply<bool> r = w.call("disconnectApplication", wallet, app);
	return r;					// default is false
}


QStringList Wallet::users(const QString& name) {
	QDBusInterface wallet("org.kde.kded", "/modules/kwalletd", "org.kde.KWallet");
	QDBusReply<QStringList> r = wallet.call("users", name);
	return r;				  // default is QStringList()
}


int Wallet::sync() {
	if (_handle == -1) {
		return -1;
	}

	_wallet->call("sync", _handle);
	return 0;
}


int Wallet::lockWallet() {
	if (_handle == -1) {
		return -1;
	}

	QDBusReply<int> r = _wallet->call("close", _handle, true);
	_handle = -1;
	_folder.clear();
	_name.clear();
	if (r.isValid()) {
		return r;
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

	_wallet->call("changePassword", _name, qlonglong(w));
}


void Wallet::slotWalletClosed(int handle) {
	if (_handle == handle) {
		_handle = -1;
		_folder.clear();
		_name.clear();
		emit walletClosed();
	}
}


QStringList Wallet::folderList() {
	if (_handle == -1) {
		return QStringList();
	}

	QDBusReply<QStringList> r = _wallet->call("folderList", _handle);
	return r;
}


QStringList Wallet::entryList() {
	if (_handle == -1) {
		return QStringList();
	}

	QDBusReply<QStringList> r = _wallet->call("entryList", _handle, _folder);
	return r;
}


bool Wallet::hasFolder(const QString& f) {
	if (_handle == -1) {
		return false;
	}

	QDBusReply<bool> r = _wallet->call("hasFolder", _handle, f);
	return r;					// default is false
}


bool Wallet::createFolder(const QString& f) {
	if (_handle == -1) {
		return false;
	}

	if (!hasFolder(f)) {
		QDBusReply<bool> r = _wallet->call("createFolder", _handle, f);
		return r;
	}

	return true;				// folder already exists
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
	if (_handle == -1) {
		return false;
	}

	QDBusReply<bool> r = _wallet->call("removeFolder", _handle, f);
	if (_folder == f) {
		setFolder(QString());
	}

	return r;					// default is false
}


const QString& Wallet::currentFolder() const {
	return _folder;
}


int Wallet::readEntry(const QString& key, QByteArray& value) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	QDBusReply<QByteArray> r = _wallet->call("readEntry", _handle, _folder, key);
	if (r.isValid()) {
		value = r;
		rc = 0;
	}

	return rc;
}


int Wallet::readEntryList(const QString& key, QMap<QString, QByteArray>& value) {
	registerTypes();

	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	QDBusReply<QMap<QString, QByteArray> > r = _wallet->call("readEntryList", _handle, _folder, key);
	if (r.isValid()) {
		rc = 0;
                value = r;
	}

	return rc;
}


int Wallet::renameEntry(const QString& oldName, const QString& newName) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	QDBusReply<int> r = _wallet->call("renameEntry", _handle, _folder, oldName, newName);
	if (r.isValid()) {
		rc = r;
	}

	return rc;
}


int Wallet::readMap(const QString& key, QMap<QString,QString>& value) {
	registerTypes();

	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	QDBusReply<QMap<QString,QString> > r = _wallet->call("readMap", _handle, _folder, key);
	if (r.isValid()) {
		rc = 0;
                value = r;
	}

	return rc;
}


int Wallet::readMapList(const QString& key, QMap<QString, QMap<QString, QString> >& value) {
	registerTypes();

	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	QDBusReply<QMap<QString, QMap<QString, QString> > > r =
            _wallet->call("readMapList", _handle, _folder, key);
	if (r.isValid()) {
		rc = 0;
                value = r;
	}

	return rc;
}


int Wallet::readPassword(const QString& key, QString& value) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	QDBusReply<QString> r = _wallet->call("readPassword", _handle, _folder, key);
	if (r.isValid()) {
		value = r;
		rc = 0;
	}

	return rc;
}


int Wallet::readPasswordList(const QString& key, QMap<QString, QString>& value) {
	registerTypes();

	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	QDBusReply<QMap<QString, QString> > r = _wallet->call("readPasswordList", _handle, _folder, key);
	if (r.isValid()) {
		rc = 0;
                value = r;
	}

	return rc;
}


int Wallet::writeEntry(const QString& key, const QByteArray& value, EntryType entryType) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	QDBusReply<int> r = _wallet->call("writeEntry", _handle, _folder, key, value, int(entryType));
	if (r.isValid()) {
		rc = r;
	}

	return rc;
}


int Wallet::writeEntry(const QString& key, const QByteArray& value) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	QDBusReply<int> r = _wallet->call("writeEntry", _handle, _folder, key, value);
	if (r.isValid()) {
		rc = r;
	}

	return rc;
}


int Wallet::writeMap(const QString& key, const QMap<QString,QString>& value) {
	registerTypes();

	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	QDBusReply<int> r = _wallet->call("writeMap.issa{ss}", _handle, _folder, key,
                                          qVariantFromValue(value));
	if (r.isValid()) {
		rc = r;
	}

	return rc;
}


int Wallet::writePassword(const QString& key, const QString& value) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	QDBusReply<int> r = _wallet->call("writePassword", _handle, _folder, key, value);
	if (r.isValid()) {
		rc = r;
	}

	return rc;
}


bool Wallet::hasEntry(const QString& key) {
	if (_handle == -1) {
		return false;
	}

	QDBusReply<bool> r = _wallet->call("hasEntry", _handle, _folder, key);
	return r;					// default is false
}


int Wallet::removeEntry(const QString& key) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	QDBusReply<int> r = _wallet->call("removeEntry", _handle, _folder, key);
	if (r.isValid()) {
		rc = r;
	}

	return rc;
}


Wallet::EntryType Wallet::entryType(const QString& key) {
	int rc = 0;

	if (_handle == -1) {
		return Wallet::Unknown;
	}

	QDBusReply<int> r = _wallet->call("entryType", _handle, _folder, key);
	if (r.isValid()) {
		rc = r;
	}

	return static_cast<EntryType>(rc);
}


void Wallet::slotAppUnregistered(const QString& app) {
	if (_handle >= 0 && app == "org.kde.kded") {
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


void Wallet::slotApplicationDisconnected(const QString& wallet, const QString& application) {
	if (_handle >= 0
			&& _name == wallet
			&& application == QDBus::sessionBus().baseService()) {
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

bool Wallet::folderDoesNotExist(const QString& wallet, const QString& folder)
{
	QDBusInterface w("org.kde.kded", "/modules/kwalletd", "org.kde.KWallet");
	QDBusReply<bool> r = w.call("folderDoesNotExist", wallet, folder);
	return r;
}


bool Wallet::keyDoesNotExist(const QString& wallet, const QString& folder, const QString& key)
{
	QDBusInterface w("org.kde.kded", "/modules/kwalletd", "org.kde.KWallet");
	QDBusReply<bool> r = w.call("keyDoesNotExist", wallet, folder, key);
	return r;
}

void Wallet::virtual_hook(int, void*) {
	//BASE::virtual_hook( id, data );
}

#include "kwallet.moc"
