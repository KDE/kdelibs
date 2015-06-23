/*
   This file is part of the KDE libraries

   Copyright (c) 2002-2004 George Staikos <staikos@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#include "kbetterthankdialogbase.h"
#include "kwalletwizard.h"
#include "kwalletd.h"
#include "ktimeout.h"

#include <dcopclient.h>
#include <dcopref.h>
#include <kactivelabel.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpassdlg.h>
#include <kstandarddirs.h>
#include <kwalletentry.h>
#include <kwin.h>

#include <qdir.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qstylesheet.h>
#include <qvbox.h>

#include <assert.h>

extern "C" {
   KDE_EXPORT KDEDModule *create_kwalletd(const QCString &name) {
	   return new KWalletD(name);
   }
}


class KWalletTransaction {
	public:
		KWalletTransaction() {
			tType = Unknown;
			transaction = 0L;
			client = 0L;
			modal = false;
		}

		~KWalletTransaction() {
			// Don't delete these!
			transaction = 0L;
			client = 0L;
		}

		enum Type { Unknown, Open, ChangePassword, OpenFail };
		DCOPClient *client;
		DCOPClientTransaction *transaction;
		Type tType;
		QCString rawappid, returnObject;
		QCString appid;
		uint wId;
		QString wallet;
		bool modal;
};


KWalletD::KWalletD(const QCString &name)
: KDEDModule(name), _failed(0) {
	srand(time(0));
	_showingFailureNotify = false;
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
	} while (_wallets.find(rc) || rc == 0);

	return rc;
}


void KWalletD::processTransactions() {
	static bool processing = false;

	if (processing) {
		return;
	}

	processing = true;

	// Process remaining transactions
	KWalletTransaction *xact;
	while (!_transactions.isEmpty()) {
		xact = _transactions.first();
		QCString replyType;
		int res;

		assert(xact->tType != KWalletTransaction::Unknown);

		switch (xact->tType) {
			case KWalletTransaction::Open:
				res = doTransactionOpen(xact->appid, xact->wallet, xact->wId, xact->modal);
				replyType = "int";
				if (!xact->returnObject.isEmpty()) {
					DCOPRef(xact->rawappid, xact->returnObject).send("walletOpenResult", res);
				}

				// multiple requests from the same client
				// should not produce multiple password
				// dialogs on a failure
				if (res < 0) {
					QPtrListIterator<KWalletTransaction> it(_transactions);
					KWalletTransaction *x;
					while ((x = it.current()) && x != xact) {
						++it;
					}
					if (x) {
						++it;
					}
					while ((x = it.current())) {
						if (xact->appid == x->appid && x->tType == KWalletTransaction::Open && x->wallet == xact->wallet && x->wId == xact->wId) {
							x->tType = KWalletTransaction::OpenFail;
						}
						++it;
					}
				}
				break;
			case KWalletTransaction::OpenFail:
				res = -1;
				replyType = "int";
				if (!xact->returnObject.isEmpty()) {
					DCOPRef(xact->rawappid, xact->returnObject).send("walletOpenResult", res);
				}
				break;
			case KWalletTransaction::ChangePassword:
				doTransactionChangePassword(xact->appid, xact->wallet, xact->wId);
				// fall through - no return
			default:
				_transactions.removeRef(xact);
				continue;
		}

		if (xact->returnObject.isEmpty() && xact->tType != KWalletTransaction::ChangePassword) {
			QByteArray replyData;
			QDataStream stream(replyData, IO_WriteOnly);
			stream << res;
			xact->client->endTransaction(xact->transaction, replyType, replyData);
		}
		_transactions.removeRef(xact);
	}

	processing = false;
}


void KWalletD::openAsynchronous(const QString& wallet, const QCString& returnObject, uint wId) {
	DCOPClient *dc = callingDcopClient();
	if (!dc) {
		return;
	}

	QCString appid = dc->senderId();
	if (!_enabled ||
		!QRegExp("^[A-Za-z0-9]+[A-Za-z0-9\\s\\-_]*$").exactMatch(wallet)) {
		DCOPRef(appid, returnObject).send("walletOpenResult", -1);
		return;
	}

	QCString peerName = friendlyDCOPPeerName();

	KWalletTransaction *xact = new KWalletTransaction;

	xact->appid = peerName;
	xact->rawappid = appid;
	xact->client = callingDcopClient();
	xact->wallet = wallet;
	xact->wId = wId;
	xact->tType = KWalletTransaction::Open;
	xact->returnObject = returnObject;
	_transactions.append(xact);

	DCOPRef(appid, returnObject).send("walletOpenResult", 0);

	QTimer::singleShot(0, this, SLOT(processTransactions()));
	checkActiveDialog();
}


int KWalletD::openPath(const QString& path, uint wId) {
	if (!_enabled) { // guard
		return -1;
	}

	// FIXME: setup transaction
	int rc = internalOpen(friendlyDCOPPeerName(), path, true, wId);
	return rc;
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

	xact->appid = appid;
	xact->client = callingDcopClient();
	xact->transaction = xact->client->beginTransaction();
	xact->wallet = wallet;
	xact->wId = wId;
	xact->tType = KWalletTransaction::Open;
	xact->modal = true; // mark dialogs as modal, the app has blocking wait
	QTimer::singleShot(0, this, SLOT(processTransactions()));
	checkActiveDialog();
	return 0; // process later
}


// Sets up a dialog that will be shown by kwallet.
void KWalletD::setupDialog( QWidget* dialog, WId wId, const QCString& appid, bool modal ) {
	if( wId != 0 ) 
		KWin::setMainWindow( dialog, wId ); // correct, set dialog parent
	else {
		if( appid.isEmpty())
			kdWarning() << "Using kwallet without parent window!" << endl;
		else
			kdWarning() << "Application '" << appid << "' using kwallet without parent window!" << endl;
		// allow dialog activation even if it interrupts, better than trying hacks
		// with keeping the dialog on top or on all desktops
		kapp->updateUserTimestamp();
	}
	if( modal )
		KWin::setState( dialog->winId(), NET::Modal );
	else
		KWin::clearState( dialog->winId(), NET::Modal );
	activeDialog = dialog;
}

// If there's a dialog already open and another application tries some operation that'd lead to
// opening a dialog, that application will be blocked by this dialog. A proper solution would
// be to set the second application's window also as a parent for the active dialog, so that
// KWin properly handles focus changes and so on, but there's currently no support for multiple
// dialog parents. Hopefully to be done in KDE4, for now just use all kinds of bad hacks to make
//  sure the user doesn't overlook the active dialog.
void KWalletD::checkActiveDialog() {
	if( !activeDialog || !activeDialog->isShown())
		return;
	kapp->updateUserTimestamp();
	KWin::setState( activeDialog->winId(), NET::KeepAbove );
	KWin::setOnAllDesktops( activeDialog->winId(), true );
	KWin::forceActiveWindow( activeDialog->winId());
}

int KWalletD::doTransactionOpen(const QCString& appid, const QString& wallet, uint wId, bool modal) {
	if (_firstUse && !wallets().contains(KWallet::Wallet::LocalWallet())) {
		// First use wizard
		KWalletWizard *wiz = new KWalletWizard(0);
		setupDialog( wiz, wId, appid, modal );
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

	int rc = internalOpen(appid, wallet, false, wId, modal);
	return rc;
}


int KWalletD::internalOpen(const QCString& appid, const QString& wallet, bool isPath, WId w, bool modal) {
	int rc = -1;
	bool brandNew = false;

	QCString thisApp;
	if (appid.isEmpty()) {
		thisApp = "KDE System";
	} else {
		thisApp = appid;
	}

	if (implicitDeny(wallet, thisApp)) {
		return -1;
	}

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
		KPasswordDialog *kpd = 0L;
		bool emptyPass = false;
		if ((isPath && QFile::exists(wallet)) || (!isPath && KWallet::Backend::exists(wallet))) {
			int pwless = b->open(QByteArray());
			if (0 != pwless || !b->isOpen()) {
				if (pwless == 0) {
					// release, start anew
					delete b;
					b = new KWallet::Backend(wallet, isPath);
				}
				kpd = new KPasswordDialog(KPasswordDialog::Password, false, 0);
				if (appid.isEmpty()) {
					kpd->setPrompt(i18n("<qt>KDE has requested to open the wallet '<b>%1</b>'. Please enter the password for this wallet below.").arg(QStyleSheet::escape(wallet)));
				} else {
					kpd->setPrompt(i18n("<qt>The application '<b>%1</b>' has requested to open the wallet '<b>%2</b>'. Please enter the password for this wallet below.").arg(QStyleSheet::escape(appid)).arg(QStyleSheet::escape(wallet)));
				}
				brandNew = false;
				kpd->setButtonOK(KGuiItem(i18n("&Open"),"fileopen"));
			} else {
				emptyPass = true;
			}
		} else if (wallet == KWallet::Wallet::LocalWallet() ||
				wallet == KWallet::Wallet::NetworkWallet()) {
			// Auto create these wallets.
			kpd = new KPasswordDialog(KPasswordDialog::NewPassword, false, 0);
			if (appid.isEmpty()) {
				kpd->setPrompt(i18n("KDE has requested to open the wallet. This is used to store sensitive data in a secure fashion. Please enter a password to use with this wallet or click cancel to deny the application's request."));
			} else {
				kpd->setPrompt(i18n("<qt>The application '<b>%1</b>' has requested to open the KDE wallet. This is used to store sensitive data in a secure fashion. Please enter a password to use with this wallet or click cancel to deny the application's request.").arg(QStyleSheet::escape(appid)));
			}
			brandNew = true;
			kpd->setButtonOK(KGuiItem(i18n("&Open"),"fileopen"));
		} else {
			kpd = new KPasswordDialog(KPasswordDialog::NewPassword, false, 0);
			if (appid.length() == 0) {
				kpd->setPrompt(i18n("<qt>KDE has requested to create a new wallet named '<b>%1</b>'. Please choose a password for this wallet, or cancel to deny the application's request.").arg(QStyleSheet::escape(wallet)));
			} else {
				kpd->setPrompt(i18n("<qt>The application '<b>%1</b>' has requested to create a new wallet named '<b>%2</b>'. Please choose a password for this wallet, or cancel to deny the application's request.").arg(QStyleSheet::escape(appid)).arg(QStyleSheet::escape(wallet)));
			}
			brandNew = true;
			kpd->setButtonOK(KGuiItem(i18n("C&reate"),"filenew"));
		}

		if (kpd) {
			kpd->setCaption(i18n("KDE Wallet Service"));
			kpd->setAllowEmptyPasswords(true);
		}

		const char *p = 0L;
		while (!b->isOpen()) {
			assert(kpd); // kpd can't be null if isOpen() is false
			setupDialog( kpd, w, appid, modal );
			if (kpd->exec() == KDialog::Accepted) {
				p = kpd->password();
				int rc = b->open(QByteArray().duplicate(p, strlen(p)));
				if (!b->isOpen()) {
					kpd->setPrompt(i18n("<qt>Error opening the wallet '<b>%1</b>'. Please try again.<br>(Error code %2: %3)").arg(QStyleSheet::escape(wallet)).arg(rc).arg(KWallet::Backend::openRCToString(rc)));
					kpd->clearPassword();
				}
			} else {
				break;
			}
		}

		if (!emptyPass && (!p || !b->isOpen())) {
			delete b;
			delete kpd;
			return -1;
		}

		if (emptyPass && _openPrompt && !isAuthorizedApp(appid, wallet, w)) {
			delete b;
			delete kpd;
			return -1;
		}

		_wallets.insert(rc = generateHandle(), b);
		if (emptyPass) {
			_passwords[wallet] = "";
		} else {
			_passwords[wallet] = p;
		}
		_handles[appid].append(rc);

		delete kpd; // don't refactor this!!  Argh I hate KPassDlg

		if (brandNew) {
			createFolder(rc, KWallet::Wallet::PasswordFolder());
			createFolder(rc, KWallet::Wallet::FormDataFolder());
		}

		b->ref();
		if (_closeIdle && _timeouts) {
			_timeouts->addTimer(rc, _idleTime);
		}
		QByteArray data;
		QDataStream ds(data, IO_WriteOnly);
		ds << wallet;
		if (brandNew) {
			emitDCOPSignal("walletCreated(QString)", data);
		}
		emitDCOPSignal("walletOpened(QString)", data);
		if (_wallets.count() == 1 && _launchManager) {
			KApplication::startServiceByDesktopName("kwalletmanager-kwalletd");
		}
	} else {
		if (!_handles[appid].contains(rc) && _openPrompt && !isAuthorizedApp(appid, wallet, w)) {
			return -1;
		}
		_handles[appid].append(rc);
		_wallets.find(rc)->ref();
	}

	return rc;
}


bool KWalletD::isAuthorizedApp(const QCString& appid, const QString& wallet, WId w) {
	int response = 0;

	QCString thisApp;
	if (appid.isEmpty()) {
		thisApp = "KDE System";
	} else {
		thisApp = appid;
	}

	if (!implicitAllow(wallet, thisApp)) {
		KBetterThanKDialogBase *dialog = new KBetterThanKDialogBase;
		if (appid.isEmpty()) {
			dialog->setLabel(i18n("<qt>KDE has requested access to the open wallet '<b>%1</b>'.").arg(QStyleSheet::escape(wallet)));
		} else {
			dialog->setLabel(i18n("<qt>The application '<b>%1</b>' has requested access to the open wallet '<b>%2</b>'.").arg(QStyleSheet::escape(QString(appid))).arg(QStyleSheet::escape(wallet)));
		}
		setupDialog( dialog, w, appid, false );
		response = dialog->exec();
		delete dialog;
	}

	if (response == 0 || response == 1) {
		if (response == 1) {
			KConfig cfg("kwalletrc");
			cfg.setGroup("Auto Allow");
			QStringList apps = cfg.readListEntry(wallet);
			if (!apps.contains(thisApp)) {
				apps += thisApp;
				_implicitAllowMap[wallet] += thisApp;
				cfg.writeEntry(wallet, apps);
				cfg.sync();
			}
		}
	} else if (response == 3) {
		KConfig cfg("kwalletrc");
		cfg.setGroup("Auto Deny");
		QStringList apps = cfg.readListEntry(wallet);
		if (!apps.contains(thisApp)) {
			apps += thisApp;
			_implicitDenyMap[wallet] += thisApp;
			cfg.writeEntry(wallet, apps);
			cfg.sync();
		}
		return false;
	} else {
		return false;
	}
	return true;
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

	xact->appid = appid;
	xact->client = callingDcopClient();
	xact->wallet = wallet;
	xact->wId = wId;
	xact->tType = KWalletTransaction::ChangePassword;

	_transactions.append(xact);

	QTimer::singleShot(0, this, SLOT(processTransactions()));
	checkActiveDialog();
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
		handle = doTransactionOpen(appid, wallet, wId,false);
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
	kpd->setPrompt(i18n("<qt>Please choose a new password for the wallet '<b>%1</b>'.").arg(QStyleSheet::escape(wallet)));
	kpd->setCaption(i18n("KDE Wallet Service"));
	kpd->setAllowEmptyPasswords(true);
	setupDialog( kpd, wId, appid, false );
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
		assert(_passwords.contains(wallet));
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
	if (handle == 0) {
		return false;
	}

	KWallet::Backend *rc = _wallets.find(handle);

	if (rc == 0 && ++_failed > 5) {
		_failed = 0;
		QTimer::singleShot(0, this, SLOT(notifyFailures()));
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


QMap<QString,QByteArray> KWalletD::readMapList(int handle, const QString& folder, const QString& key) {
	KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		QPtrList<KWallet::Entry> e = b->readEntryList(key);
		QMap<QString, QByteArray> rc;
		QPtrListIterator<KWallet::Entry> it(e);
		KWallet::Entry *entry;
		while ((entry = it.current())) {
			if (entry->type() == KWallet::Wallet::Map) {
				rc.insert(entry->key(), entry->map());
			}
			++it;
		}
		return rc;
	}

	return QMap<QString, QByteArray>();
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


QMap<QString, QByteArray> KWalletD::readEntryList(int handle, const QString& folder, const QString& key) {
	KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		QPtrList<KWallet::Entry> e = b->readEntryList(key);
		QMap<QString, QByteArray> rc;
		QPtrListIterator<KWallet::Entry> it(e);
		KWallet::Entry *entry;
		while ((entry = it.current())) {
			rc.insert(entry->key(), entry->value());
			++it;
		}
		return rc;
	}

	return QMap<QString, QByteArray>();
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


QMap<QString, QString> KWalletD::readPasswordList(int handle, const QString& folder, const QString& key) {
	KWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		QPtrList<KWallet::Entry> e = b->readEntryList(key);
		QMap<QString, QString> rc;
		QPtrListIterator<KWallet::Entry> it(e);
		KWallet::Entry *entry;
		while ((entry = it.current())) {
			if (entry->type() == KWallet::Wallet::Password) {
				rc.insert(entry->key(), entry->password());
			}
			++it;
		}
		return rc;
	}

	return QMap<QString, QString>();
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
		for (QValueList<int>::Iterator i = l.begin(); i != l.end(); ++i) {
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
	if (handle == 0) {
		return 0L;
	}

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
		_failed = 0;
		QTimer::singleShot(0, this, SLOT(notifyFailures()));
	}

	return 0L;
}


void KWalletD::notifyFailures() {
	if (!_showingFailureNotify) {
		_showingFailureNotify = true;
		KMessageBox::information(0, i18n("There have been repeated failed attempts to gain access to a wallet. An application may be misbehaving."), i18n("KDE Wallet Service"));
		_showingFailureNotify = false;
	}
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
	_launchManager = cfg.readBoolEntry("Launch Manager", false);
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

	// Update the implicit allow stuff
	_implicitDenyMap.clear();
	cfg.setGroup("Auto Deny");
	entries = cfg.entryMap("Auto Deny").keys();
	for (QStringList::Iterator i = entries.begin(); i != entries.end(); ++i) {
		_implicitDenyMap[*i] = cfg.readListEntry(*i);
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


bool KWalletD::implicitDeny(const QString& wallet, const QCString& app) {
	return _implicitDenyMap[wallet].contains(QString::fromLocal8Bit(app));
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


QString KWalletD::networkWallet() {
	return KWallet::Wallet::NetworkWallet();
}


QString KWalletD::localWallet() {
	return KWallet::Wallet::LocalWallet();
}


#include "kwalletd.moc"
