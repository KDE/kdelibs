/*
   This file is part of the KDE libraries

   Copyright (c) 2002-2003 George Staikos <staikos@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/

#include "kwalletwizard.h"
#include "kwalletd.h"
#include "ktimeout.h"

#include <dcopclient.h>
#include <dcopref.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpassdlg.h>
#include <kstddirs.h>
#include <kwalletentry.h>

#include <qdir.h>
#include <qregexp.h>

#include <assert.h>

#include <X11/Xlib.h>

extern "C" {
   KDEDModule *create_kwalletd(const QCString &name) {
	   return new KWalletD(name);
   }
}


class KWalletTransaction {
	public:
		KWalletTransaction() { 
			tType = Unknown;
			transaction = 0L;
			client = 0L;
		}

		~KWalletTransaction() {
			// Don't delete these!
			transaction = 0L;
			client = 0L;
		}

		enum Type { Unknown, Open, ChangePassword };
		DCOPClient *client;
		DCOPClientTransaction *transaction;
		Type tType;
		QCString appid;
		uint wId;
		QString wallet;
};


KWalletD::KWalletD(const QCString &name)
: KDEDModule(name), _failed(0) {
	srand(time(0));
	_transactions.setAutoDelete(true);
	_timeouts = new KTimeout(17);
	_closeIdle = false;
	_idleTime = 0;
	connect(_timeouts, SIGNAL(timedOut(int)), this, SLOT(timedOut(int)));
	reconfigure();
	KGlobal::dirs()->addResourceType("kwallet", "share/apps/kwallet");
	connect(KApplication::dcopClient(),
		SIGNAL(applicationRemoved(const QCString&)),
		this,
		SLOT(slotAppUnregistered(const QCString&)));
	_dw = new KDirWatch(this, "KWallet Directory Watcher");
	_dw->addDir(KGlobal::dirs()->saveLocation("kwallet"));
	_dw->startScan(true);
	connect(_dw, SIGNAL(dirty(const QString&)), this, SLOT(emitWalletListDirty()));
}
  

KWalletD::~KWalletD() {
	delete _timeouts;
	_timeouts = 0;

	closeAllWallets();
	_transactions.clear();
}


int KWalletD::generateHandle() {
int rc;

	// ASSUMPTION: RAND_MAX is fairly large.
	do {
		rc = rand();
	} while (_wallets.find(rc));

return rc;
}


void KWalletD::processTransactions() {
	// Process remaining transactions
	for (KWalletTransaction *xact = _transactions.first(); xact; /**/) {
		QCString replyType;
		int res;

		assert(xact->tType != KWalletTransaction::Unknown);

		switch (xact->tType) {
		case KWalletTransaction::Open:
			res = doTransactionOpen(xact->appid, xact->wallet, xact->wId);
			replyType = "int";
			break;
		case KWalletTransaction::ChangePassword:
			doTransactionChangePassword(xact->appid, xact->wallet, xact->wId);
			// fall through - no return
		default:
			KWalletTransaction *tmp = xact;
			xact = _transactions.next();
			_transactions.removeRef(tmp);
			continue;
		}

		QByteArray replyData;
		QDataStream stream(replyData, IO_WriteOnly);
		stream << res;
		xact->client->endTransaction(xact->transaction, replyType, replyData);
		KWalletTransaction *tmp = xact;
		xact = _transactions.next();
		_transactions.removeRef(tmp);
	}
}

void KWalletD::openAsynchronous(const QString& wallet, const QCString& returnObject, uint wId) {
	DCOPClient *dc = callingDcopClient();
	if (!dc) return;
	
	int rc = open(wallet, wId);
	QCString appid = dc->senderId();
	DCOPRef(appid, returnObject).send("walletOpenResult", rc);
}


int KWalletD::openPath(const QString& path, uint wId) {
	if (!_enabled) { // guard
		return -1;
	}

	// FIXME: setup transaction
	return internalOpen(friendlyDCOPPeerName(), path, true, wId);
}


int KWalletD::open(const QString& wallet, uint wId) {
	if (!_enabled) { // guard
		return -1;
	}

	if (!QRegExp("^[A-Za-z0-9]+[A-Za-z0-9\\s\\-_]*$").exactMatch(wallet)) {
		return -1;
	}

	QCString appid = friendlyDCOPPeerName();

	KWalletTransaction *xact = new KWalletTransaction;
	_transactions.append(xact);

	if (_transactions.count() > 1) {
		xact->appid = appid;
		xact->client = callingDcopClient();
		xact->transaction = xact->client->beginTransaction();
		xact->wallet = wallet;
		xact->wId = wId;
		xact->tType = KWalletTransaction::Open;
		return 0; // process later
	}

	int rc = doTransactionOpen(appid, wallet, wId);

	_transactions.remove(xact);

	if (rc < 0) {
		// Kill off multiple requests from the same client on a failure
		for (KWalletTransaction *x = _transactions.first(); x; /**/) {
			if (appid == x->appid && x->tType == KWalletTransaction::Open && x->wallet == wallet && x->wId == wId) {
				KWalletTransaction *tmp = x;
				x = _transactions.next();
				_transactions.removeRef(tmp);
			}
		}
	}

	processTransactions();

	return rc;
}


int KWalletD::doTransactionOpen(const QCString& appid, const QString& wallet, uint wId) {
	if (_firstUse && !wallets().contains(KWallet::Wallet::LocalWallet())) {
	       	// First use wizard
		KWalletWizard *wiz = new KWalletWizard(0);
		XSetTransientForHint(qt_xdisplay(), wiz->winId(), wId);
		int rc = wiz->exec();
		if (rc == QDialog::Accepted) {
			KConfig cfg("kwalletrc");
			cfg.setGroup("Wallet");
			cfg.writeEntry("First Use", false);
			cfg.writeEntry("Enabled", wiz->_useWallet->isChecked());
			cfg.writeEntry("Close When Idle", wiz->_closeIdle->isChecked());
			cfg.writeEntry("Use One Wallet", !wiz->_networkWallet->isChecked());
			cfg.sync();
			reconfigure();

			if (!wiz->_useWallet->isChecked()) {
				delete wiz;
				return -1;
			}

			// Create the wallet
			KWallet::Backend *b = new KWallet::Backend(KWallet::Wallet::LocalWallet());
			QByteArray p;
			p.duplicate(wiz->_pass1->text().utf8(), wiz->_pass1->text().length());
			b->open(p);
			b->createFolder(KWallet::Wallet::PasswordFolder());
			b->createFolder(KWallet::Wallet::FormDataFolder());
			b->close(p);
			p.fill(0);
			delete b;
			delete wiz;
		} else {
			delete wiz;
			return -1;
		}
	} else if (_firstUse) {
		KConfig cfg("kwalletrc");
		_firstUse = false;
		cfg.setGroup("Wallet");
		cfg.writeEntry("First Use", false);
		cfg.sync();
	}

	return internalOpen(appid, wallet, false, wId);
}


int KWalletD::internalOpen(const QCString& appid, const QString& wallet, bool isPath, WId w) {
	int rc = -1;
	bool brandNew = false;

	for (QIntDictIterator<KWallet::Backend> i(_wallets); i.current(); ++i) {
		if (i.current()->walletName() == wallet) {
			rc = i.currentKey();
			break;
		}
	}

	if (rc == -1) {
		if (_wallets.count() > 20) {
			kdDebug() << "Too many wallets open." << endl;
			return -1;
		}

		KWallet::Backend *b = new KWallet::Backend(wallet, isPath);
		KPasswordDialog *kpd;
		if ((isPath || QFile::exists(wallet)) || KWallet::Backend::exists(wallet)) {
			kpd = new KPasswordDialog(KPasswordDialog::Password, false, 0);
			if (appid.isEmpty()) {
				kpd->setPrompt(i18n("KDE has requested to open the wallet '%1'. Please enter the password for this wallet below.").arg(wallet));
			} else {
				kpd->setPrompt(i18n("The application '%1' has requested to open the wallet '%2'. Please enter the password for this wallet below.").arg(appid).arg(wallet));
			}
			brandNew = false;
			kpd->setButtonOKText(i18n("&Open"));
		} else if (wallet == KWallet::Wallet::LocalWallet() ||
				wallet == KWallet::Wallet::NetworkWallet()) {
			// Auto create these wallets.
			kpd = new KPasswordDialog(KPasswordDialog::NewPassword, false, 0);
			if (appid.isEmpty()) {
				kpd->setPrompt(i18n("KDE has requested to open the wallet. This is used to store sensitive data in a secure fashion. Please enter a password to use with this wallet or click cancel to deny the application's request."));
			} else {
				kpd->setPrompt(i18n("The application '%1' has requested to open the KDE wallet. This is used to store sensitive data in a secure fashion. Please enter a password to use with this wallet or click cancel to deny the application's request.").arg(appid));
			}
			brandNew = true;
			kpd->setButtonOKText(i18n("&Open"));
		} else {
			kpd = new KPasswordDialog(KPasswordDialog::NewPassword, false, 0);
			if (appid.length() == 0) {
				kpd->setPrompt(i18n("KDE has requested to create a new wallet named '%1'. Please choose a password for this wallet, or cancel to deny the application's request.").arg(wallet));
			} else {
				kpd->setPrompt(i18n("The application '%1' has requested to create a new wallet named '%2'. Please choose a password for this wallet, or cancel to deny the application's request.").arg(appid).arg(wallet));
			}
			brandNew = true;
			kpd->setButtonOKText(i18n("&Create"));
		}

		kpd->setCaption(i18n("KDE Wallet Service"));
		const char *p = 0L;
		while (!b->isOpen()) {
			XSetTransientForHint(qt_xdisplay(), kpd->winId(), w);
			if (kpd->exec() == KDialog::Accepted) {
				p = kpd->password();
				int rc = b->open(QByteArray().duplicate(p, strlen(p)));
				if (!b->isOpen()) {
					kpd->setPrompt(i18n("Invalid password for wallet '%1'. Please try again. %2").arg(wallet).arg(rc));
				}
			} else {
				break;
			}
		}

		if (!p || !b->isOpen()) {
			delete b;
			delete kpd;
			return -1;
		}

		_wallets.insert(rc = generateHandle(), b);
		_passwords[wallet] = p;
		_handles[appid].append(rc);
		
		if (brandNew) {
			createFolder(rc, KWallet::Wallet::PasswordFolder());
			createFolder(rc, KWallet::Wallet::FormDataFolder());
		}

		b->ref();
		if (_closeIdle && _timeouts) {
			_timeouts->addTimer(rc, _idleTime);
		}
		delete kpd;
		QByteArray data;
		QDataStream ds(data, IO_WriteOnly);
		ds << wallet;
		if (brandNew) {
			emitDCOPSignal("walletCreated(QString)", data);
		}
		emitDCOPSignal("walletOpened(QString)", data);
		if (_wallets.count() == 1 && _launchManager) {
			KApplication::startServiceByDesktopName("kwalletmanager");
		}
	} else {
		int response = KMessageBox::Yes;

		if (_openPrompt && !_handles[appid].contains(rc) && !implicitAllow(wallet, appid)) {
// ### FIXME 3.3: add a new string and get rid of the 'KDE' one here
			response = KMessageBox::questionYesNoCancel(0L, i18n("The application '%1' has requested access to the open wallet '%2'.").arg(appid.isEmpty() ? i18n("KDE") : QString(appid)).arg(wallet), i18n("KDE Wallet Service"), i18n("Allow &Once"), i18n("Allow &Always"));
		}

		if (response == KMessageBox::Yes || response == KMessageBox::No) {
			_handles[appid].append(rc);
			_wallets.find(rc)->ref();
			if (response == KMessageBox::No) {
				KConfig cfg("kwalletrc");
				cfg.setGroup("Auto Allow");
				QStringList apps = cfg.readListEntry(wallet);
				if (!apps.contains(appid)) {
					apps += appid;
					_implicitAllowMap[wallet] += appid;
					cfg.writeEntry(wallet, apps);
					cfg.sync();
				}
			}
		} else {
			return -1;
		}
	}

return rc;
}


int KWalletD::deleteWallet(const QString& wallet) {
	QString path = KGlobal::dirs()->saveLocation("kwallet") + QDir::separator() + wallet + ".kwl";

	if (QFile::exists(path)) {
		close(wallet, true);
		QFile::remove(path);
		QByteArray data;
		QDataStream ds(data, IO_WriteOnly);
		ds << wallet;
		emitDCOPSignal("walletDeleted(QString)", data);
		return 0;
	}

return -1;
}


void KWalletD::changePassword(const QString& wallet, uint wId) {
	QCString appid = friendlyDCOPPeerName();

	KWalletTransaction *xact = new KWalletTransaction;
	_transactions.append(xact);

	if (_transactions.count() > 1) {
		xact->appid = appid;
		xact->client = callingDcopClient();
		xact->transaction = xact->client->beginTransaction();
		xact->wallet = wallet;
		xact->wId = wId;
		xact->tType = KWalletTransaction::ChangePassword;
		return; // process later
	}

	doTransactionChangePassword(appid, wallet, wId);

	_transactions.remove(xact);

	processTransactions();
}


void KWalletD::doTransactionChangePassword(const QCString& appid, const QString& wallet, uint wId) {
QIntDictIterator<KWallet::Backend> it(_wallets);
KWallet::Backend *w = 0L;
int handle = -1;
bool reclose = false;

	for (; it.current(); ++it) {
		if (it.current()->walletName() == wallet) {
			break;
		}
	}

	if (!it.current()) {
		handle = doTransactionOpen(appid, wallet, wId);
		if (-1 == handle) {
			KMessageBox::sorryWId(wId, i18n("Unable to open wallet. The wallet must be opened in order to change the password."), i18n("KDE Wallet Service"));
			return;
		}

		w = _wallets.find(handle);
		reclose = true;
	} else {
		handle = it.currentKey();
		w = it.current();
	}

	assert(w);

	KPasswordDialog *kpd;
	kpd = new KPasswordDialog(KPasswordDialog::NewPassword, false, 0);
	kpd->setPrompt(i18n("Please choose a new password for the wallet '%1'.").arg(wallet));
	kpd->setCaption(i18n("KDE Wallet Service"));
	XSetTransientForHint(qt_xdisplay(), kpd->winId(), wId);
	if (kpd->exec() == KDialog::Accepted) {
		const char *p = kpd->password();
		if (p) {
			_passwords[wallet] = p;
			QByteArray pa;
			pa.duplicate(p, strlen(p));
			int rc = w->close(pa);
			if (rc < 0) {
				KMessageBox::sorryWId(wId, i18n("Error re-encrypting the wallet. Password was not changed."), i18n("KDE Wallet Service"));
				reclose = true;
			} else {
				rc = w->open(pa);
				if (rc < 0) {
					KMessageBox::sorryWId(wId, i18n("Error reopening the wallet. Data may be lost."), i18n("KDE Wallet Service"));
					reclose = true;
				}
			}
		}
	}

	delete kpd;
	
	if (reclose) {
		close(handle, true);
	}
}


int KWalletD::close(const QString& wallet, bool force) {
int handle = -1;
KWallet::Backend *w = 0L;

	for (QIntDictIterator<KWallet::Backend> it(_wallets);
						it.current();
							++it) {
		if (it.current()->walletName() == wallet) {
			handle = it.currentKey();
			w = it.current();
			break;
		}
	}

return closeWallet(w, handle, force);
}


int KWalletD::closeWallet(KWallet::Backend *w, int handle, bool force) {
	if (w) {
		const QString& wallet = w->walletName();
		if (w->refCount() == 0 || force) {
			invalidateHandle(handle);
			if (_closeIdle && _timeouts) {
				_timeouts->removeTimer(handle);
			}
			_wallets.remove(handle);
			if (_passwords.contains(wallet)) {
				w->close(QByteArray().duplicate(_passwords[wallet].data(), _passwords[wallet].length()));
				_passwords[wallet].fill(0);
				_passwords.remove(wallet);
			}
			doCloseSignals(handle, wallet);
			delete w;
			return 0;
		}
		return 1;
	}

return -1;
}


int KWalletD::close(int handle, bool force) {
QCString appid = friendlyDCOPPeerName();
KWallet::Backend *w = _wallets.find(handle);
bool contains = false;

	if (w) { // the handle is valid
		if (_handles.contains(appid)) { // we know this app
			if (_handles[appid].contains(handle)) {
				// the app owns this handle
				_handles[appid].remove(_handles[appid].find(handle));
				contains = true;
				if (_handles[appid].isEmpty()) {
					_handles.remove(appid);
				}
			}
		}

		// watch the side effect of the deref()
		if ((contains && w->deref() == 0 && !_leaveOpen) || force) {
			if (_closeIdle && _timeouts) {
				_timeouts->removeTimer(handle);
			}
			_wallets.remove(handle);
			if (force) {
				invalidateHandle(handle);
			}
			if (_passwords.contains(w->walletName())) {
				w->close(QByteArray().duplicate(_passwords[w->walletName()].data(), _passwords[w->walletName()].length()));
				_passwords[w->walletName()].fill(0);
				_passwords.remove(w->walletName());
			}
			doCloseSignals(handle, w->walletName());
			delete w;
			return 0;
		}
		return 1; // not closed
	}

return -1; // not open to begin with, or other error
}


bool KWalletD::isOpen(const QString& wallet) const {
	for (QIntDictIterator<KWallet::Backend> it(_wallets);
						it.current();
							++it) {
		if (it.current()->walletName() == wallet) {
			return true;
		}
	}
return false;
}


bool KWalletD::isOpen(int handle) {
	KWallet::Backend *rc = _wallets.find(handle);

	if (rc == 0 && ++_failed > 5) {
		// FIXME: Make this part of a transaction or offload it from
		//        the main execution path somehow.
		KMessageBox::information(0, i18n("There have been repeated failed attempts to gain access to a wallet. An application may be misbehaving."), i18n("KDE Wallet Service"));
		_failed = 0;
	} else if (rc != 0) {
		_failed = 0;
	}

return rc != 0;
}


QStringList KWalletD::wallets() const {
QString path = KGlobal::dirs()->saveLocation("kwallet");
QDir dir(path, "*.kwl");
QStringList rc;

	dir.setFilter(QDir::Files | QDir::NoSymLinks);

	const QFileInfoList *list = dir.entryInfoList();
	QFileInfoListIterator it(*list);
	QFileInfo *fi;
	while ((fi = it.current()) != 0L) {
		QString fn = fi->fileName();
		if (fn.endsWith(".kwl")) {
			fn.truncate(fn.length()-4);
		}
		rc += fn;
		++it;
	}
return rc;
}


void KWalletD::sync(int handle) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		QByteArray p;
		QString wallet = b->walletName();
		p.duplicate(_passwords[wallet].data(), _passwords[wallet].length());
		b->sync(p);
		p.fill(0);
	}
}


QStringList KWalletD::folderList(int handle) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		return b->folderList();
	}

return QStringList();
}


bool KWalletD::hasFolder(int handle, const QString& f) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		return b->hasFolder(f);
	}

return false;
}


bool KWalletD::removeFolder(int handle, const QString& f) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		bool rc = b->removeFolder(f);
		QByteArray data;
		QDataStream ds(data, IO_WriteOnly);
		ds << b->walletName();
		emitDCOPSignal("folderListUpdated(QString)", data);
		return rc;
	}

return false;
}


bool KWalletD::createFolder(int handle, const QString& f) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		bool rc = b->createFolder(f);
		QByteArray data;
		QDataStream ds(data, IO_WriteOnly);
		ds << b->walletName();
		emitDCOPSignal("folderListUpdated(QString)", data);
		return rc;
	}

return false;
}


QByteArray KWalletD::readMap(int handle, const QString& folder, const QString& key) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		KWallet::Entry *e = b->readEntry(key);
		if (e && e->type() == KWallet::Wallet::Map) {
			return e->map();
		}
	}

return QByteArray();
}


QByteArray KWalletD::readEntry(int handle, const QString& folder, const QString& key) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		KWallet::Entry *e = b->readEntry(key);
		if (e) {
			return e->value();
		}
	}

return QByteArray();
}


QStringList KWalletD::entryList(int handle, const QString& folder) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		return b->entryList();
	}

return QStringList();
}


QString KWalletD::readPassword(int handle, const QString& folder, const QString& key) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		KWallet::Entry *e = b->readEntry(key);
		if (e && e->type() == KWallet::Wallet::Password) {
			return e->password();
		}
	}

return QString::null;
}


int KWalletD::writeMap(int handle, const QString& folder, const QString& key, const QByteArray& value) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		KWallet::Entry e;
		e.setKey(key);
		e.setValue(value);
		e.setType(KWallet::Wallet::Map);
		b->writeEntry(&e);
		emitFolderUpdated(b->walletName(), folder);
		return 0;
	}

return -1;
}


int KWalletD::writeEntry(int handle, const QString& folder, const QString& key, const QByteArray& value, int entryType) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		KWallet::Entry e;
		e.setKey(key);
		e.setValue(value);
		e.setType(KWallet::Wallet::EntryType(entryType));
		b->writeEntry(&e);
		emitFolderUpdated(b->walletName(), folder);
		return 0;
	}

return -1;
}


int KWalletD::writeEntry(int handle, const QString& folder, const QString& key, const QByteArray& value) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		KWallet::Entry e;
		e.setKey(key);
		e.setValue(value);
		e.setType(KWallet::Wallet::Stream);
		b->writeEntry(&e);
		emitFolderUpdated(b->walletName(), folder);
		return 0;
	}

return -1;
}


int KWalletD::writePassword(int handle, const QString& folder, const QString& key, const QString& value) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		KWallet::Entry e;
		e.setKey(key);
		e.setValue(value);
		e.setType(KWallet::Wallet::Password);
		b->writeEntry(&e);
		emitFolderUpdated(b->walletName(), folder);
		return 0;
	}

return -1;
}


int KWalletD::entryType(int handle, const QString& folder, const QString& key) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		if (!b->hasFolder(folder)) {
			return KWallet::Wallet::Unknown;
		}
		b->setFolder(folder);
		if (b->hasEntry(key)) {
			return b->readEntry(key)->type();
		}
	}

return KWallet::Wallet::Unknown;
}


bool KWalletD::hasEntry(int handle, const QString& folder, const QString& key) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		if (!b->hasFolder(folder)) {
			return false;
		}
		b->setFolder(folder);
		return b->hasEntry(key);
	}

return false;
}


int KWalletD::removeEntry(int handle, const QString& folder, const QString& key) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		if (!b->hasFolder(folder)) {
			return 0;
		}
		b->setFolder(folder);
		bool rc = b->removeEntry(key);
		emitFolderUpdated(b->walletName(), folder);
		return rc ? 0 : -3;
	}

return -1;
}


void KWalletD::slotAppUnregistered(const QCString& app) {
	if (_handles.contains(app)) {
		QValueList<int> l = _handles[app];
		for (QValueList<int>::Iterator i = l.begin(); i != l.end(); i++) {
			_handles[app].remove(*i);
			KWallet::Backend *w = _wallets.find(*i);
			if (w && !_leaveOpen && 0 == w->deref()) {
				close(w->walletName(), true);
			}
		}
		_handles.remove(app);
	}
}


void KWalletD::invalidateHandle(int handle) {
	for (QMap<QCString,QValueList<int> >::Iterator i = _handles.begin();
							i != _handles.end();
									++i) {
		i.data().remove(handle);
	}
}


KWallet::Backend *KWalletD::getWallet(const QCString& appid, int handle) {
KWallet::Backend *w = _wallets.find(handle);

	if (w) { // the handle is valid
		if (_handles.contains(appid)) { // we know this app
			if (_handles[appid].contains(handle)) {
				// the app owns this handle
				_failed = 0;
				if (_closeIdle && _timeouts) {
					_timeouts->resetTimer(handle, _idleTime);
				}
				return w;
			}
		}
	}

	if (++_failed > 5) {
		// FIXME: Make this part of a transaction or offload it from
		//        the main execution path somehow.
		KMessageBox::information(0, i18n("There have been repeated failed attempts to gain access to a wallet. An application may be misbehaving."), i18n("KDE Wallet Service"));
		_failed = 0;
	}

return 0L;
}


void KWalletD::doCloseSignals(int handle, const QString& wallet) {
	QByteArray data;
	QDataStream ds(data, IO_WriteOnly);
	ds << handle;
	emitDCOPSignal("walletClosed(int)", data);

	QByteArray data2;
	QDataStream ds2(data2, IO_WriteOnly);
	ds2 << wallet;
	emitDCOPSignal("walletClosed(QString)", data2);

	if (_wallets.isEmpty()) {
		emitDCOPSignal("allWalletsClosed()", QByteArray());
	}
}


int KWalletD::renameEntry(int handle, const QString& folder, const QString& oldName, const QString& newName) {
KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		int rc = b->renameEntry(oldName, newName);
		emitFolderUpdated(b->walletName(), folder);
		return rc;
	}

return -1;
}


QStringList KWalletD::users(const QString& wallet) const {
QStringList rc;

	for (QIntDictIterator<KWallet::Backend> it(_wallets);
						it.current();
							++it) {
		if (it.current()->walletName() == wallet) {
			for (QMap<QCString,QValueList<int> >::ConstIterator hit = _handles.begin(); hit != _handles.end(); ++hit) {
				if (hit.data().contains(it.currentKey())) {
					rc += hit.key();
				}
			}
			break;
		}
	}

return rc;
}


bool KWalletD::disconnectApplication(const QString& wallet, const QCString& application) {
	for (QIntDictIterator<KWallet::Backend> it(_wallets);
						it.current();
							++it) {
		if (it.current()->walletName() == wallet) {
			if (_handles[application].contains(it.currentKey())) {
				_handles[application].remove(it.currentKey());

				if (_handles[application].isEmpty()) {
					_handles.remove(application);
				}

				if (it.current()->deref() == 0) {
					close(it.current()->walletName(), true);
				}

				QByteArray data;
				QDataStream ds(data, IO_WriteOnly);
				ds << wallet;
				ds << application;
				emitDCOPSignal("applicationDisconnected(QString,QCString)", data);

				return true;
			}
		}
	}

return false;
}


void KWalletD::emitFolderUpdated(const QString& wallet, const QString& folder) {
	QByteArray data;
	QDataStream ds(data, IO_WriteOnly);
	ds << wallet;
	ds << folder;
	emitDCOPSignal("folderUpdated(QString,QString)", data);
}


void KWalletD::emitWalletListDirty() {
	emitDCOPSignal("walletListDirty()", QByteArray());
}


void KWalletD::reconfigure() {
	KConfig cfg("kwalletrc");
	cfg.setGroup("Wallet");
	_firstUse = cfg.readBoolEntry("First Use", true);
	_enabled = cfg.readBoolEntry("Enabled", true);
	_launchManager = cfg.readBoolEntry("Launch Manager", true);
	_leaveOpen = cfg.readBoolEntry("Leave Open", false);
	bool idleSave = _closeIdle;
	_closeIdle = cfg.readBoolEntry("Close When Idle", false);
	_openPrompt = cfg.readBoolEntry("Prompt on Open", true);
	int timeSave = _idleTime;
	// in minutes!
	_idleTime = cfg.readNumEntry("Idle Timeout", 10) * 60 * 1000;

	if (cfg.readBoolEntry("Close on Screensaver", false)) {
		connectDCOPSignal("kdesktop", "KScreensaverIface", "KDE_start_screensaver()", "closeAllWallets()", false);
	} else {
		disconnectDCOPSignal("kdesktop", "KScreensaverIface", "KDE_start_screensaver()", "closeAllWallets()");
	}

	// Handle idle changes
	if (_closeIdle) {
		if (_idleTime != timeSave) { // Timer length changed
			QIntDictIterator<KWallet::Backend> it(_wallets);
			for (; it.current(); ++it) {
				_timeouts->resetTimer(it.currentKey(), _idleTime);
			}
		}

		if (!idleSave) { // add timers for all the wallets
			QIntDictIterator<KWallet::Backend> it(_wallets);
			for (; it.current(); ++it) {
				_timeouts->addTimer(it.currentKey(), _idleTime);
			}
		}
	} else {
		_timeouts->clear();
	}

	// Update the implicit allow stuff
	_implicitAllowMap.clear();
	cfg.setGroup("Auto Allow");
	QStringList entries = cfg.entryMap("Auto Allow").keys();
	for (QStringList::Iterator i = entries.begin(); i != entries.end(); ++i) {
		_implicitAllowMap[*i] = cfg.readListEntry(*i);
	}

	// Update if wallet was enabled/disabled
	if (!_enabled) { // close all wallets
		while (!_wallets.isEmpty()) { 
			QIntDictIterator<KWallet::Backend> it(_wallets);
			if (!it.current()) { // necessary?
				break;
			}
			closeWallet(it.current(), it.currentKey(), true);
		}
	}
}


bool KWalletD::isEnabled() const {
	return _enabled;
}


bool KWalletD::folderDoesNotExist(const QString& wallet, const QString& folder) {
	if (!wallets().contains(wallet)) {
		return true;
	}

	for (QIntDictIterator<KWallet::Backend> it(_wallets); it.current(); ++it) {
		if (it.current()->walletName() == wallet) {
			return it.current()->folderDoesNotExist(folder);
		}
	}

	KWallet::Backend *b = new KWallet::Backend(wallet);
	b->open(QByteArray());
	bool rc = b->folderDoesNotExist(folder);
	delete b;
	return rc;
}


bool KWalletD::keyDoesNotExist(const QString& wallet, const QString& folder, const QString& key) {
	if (!wallets().contains(wallet)) {
		return true;
	}

	for (QIntDictIterator<KWallet::Backend> it(_wallets); it.current(); ++it) {
		if (it.current()->walletName() == wallet) {
			return it.current()->entryDoesNotExist(folder, key);
		}
	}

	KWallet::Backend *b = new KWallet::Backend(wallet);
	b->open(QByteArray());
	bool rc = b->entryDoesNotExist(folder, key);
	delete b;
	return rc;
}


bool KWalletD::implicitAllow(const QString& wallet, const QCString& app) {
	return _implicitAllowMap[wallet].contains(QString::fromLocal8Bit(app));
}


QCString KWalletD::friendlyDCOPPeerName() {
	DCOPClient *dc = callingDcopClient();
	if (!dc) {
		return "";
	}
	return dc->senderId().replace(QRegExp("-[0-9]+$"), "");
}


void KWalletD::timedOut(int id) {
	KWallet::Backend *w = _wallets.find(id);
	if (w) {
		closeWallet(w, id, true);
	}
}


void KWalletD::closeAllWallets() {
	QIntDict<KWallet::Backend> tw = _wallets;

	for (QIntDictIterator<KWallet::Backend> it(tw); it.current(); ++it) {
		closeWallet(it.current(), it.currentKey(), true);
	}

	tw.clear();

	// All of this should be basically noop.  Let's just be safe.
	_wallets.clear();

	for (QMap<QString,QCString>::Iterator it = _passwords.begin();
						it != _passwords.end();
						++it) {
		it.data().fill(0);
	}
	_passwords.clear();
}

#include "kwalletd.moc"
