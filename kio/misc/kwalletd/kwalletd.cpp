// -*- indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*-
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

#include "kwalletd.h"

#include "kbetterthankdialog.h"
#include "kwalletwizard.h"
#include "ktimeout.h"

#include <kuniqueapplication.h>
#include <ktoolinvocation.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpassworddialog.h>
#include <knewpassworddialog.h>
#include <kstandarddirs.h>
#include <kwalletentry.h>
#include <kwindowsystem.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <QtCore/QDir>
#include <QtGui/QTextDocument> // Qt::escape
#include <QtCore/QRegExp>
#include <QtCore/QTimer>

#include <assert.h>

#include "kwalletdadaptor.h"
#include "kwalletsynctimer.h"

class KWalletTransaction {
	public:
		KWalletTransaction() {
			tType = Unknown;
		}

		~KWalletTransaction() {
		}

		enum Type { Unknown, Open, ChangePassword, OpenFail };
		QDBusMessage msg;
		Type tType;
		QString appid;
		qlonglong wId;
		QString wallet;
		bool modal;
};

KWalletD::KWalletD()
	: QObject(0), _failed(0) {
	srand(time(0));
	_showingFailureNotify = false;
	_timeouts = new KTimeout();
	_closeIdle = false;
	_idleTime = 0;
	connect(_timeouts, SIGNAL(timedOut(int)), this, SLOT(timedOut(int)));

	(void)new KWalletDAdaptor(this);
	// register services
	QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.kwalletd"));
	QDBusConnection::sessionBus().registerObject(QLatin1String("/modules/kwalletd"), this);
	
#ifdef Q_WS_X11
	screensaver = new QDBusInterface("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver");
#endif

	reconfigure();
	KGlobal::dirs()->addResourceType("kwallet", 0, "share/apps/kwallet");
		connect(QDBusConnection::sessionBus().interface(), SIGNAL(serviceUnregistered(QString)),
				SLOT(slotServiceUnregistered(QString)));
	_dw = new KDirWatch(this );
		_dw->setObjectName( "KWallet Directory Watcher" );
	_dw->addDir(KGlobal::dirs()->saveLocation("kwallet"));
	_dw->startScan(true);
	connect(_dw, SIGNAL(dirty(const QString&)), this, SLOT(emitWalletListDirty()));
}


KWalletD::~KWalletD() {
	delete _timeouts;
	_timeouts = 0;
#ifdef Q_WS_X11
	delete screensaver;
	screensaver = 0;
#endif
	closeAllWallets();
	qDeleteAll(_synctimers);
	qDeleteAll(_transactions);
}


int KWalletD::generateHandle() {
	int rc;

	// ASSUMPTION: RAND_MAX is fairly large.
	do {
		rc = rand();
	} while (_wallets.contains(rc) || rc == 0);

	return rc;
}

QPair<int, KWallet::Backend*> KWalletD::findWallet(const QString& walletName) const
{
	Wallets::const_iterator it = _wallets.begin();
	const Wallets::const_iterator end = _wallets.end();
	for (; it != end; ++it) {
		if (it.value()->walletName() == walletName) {
			return qMakePair(it.key(), it.value());
		}
	}
    return qMakePair(-1, static_cast<KWallet::Backend*>(0));
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
		int res;

		assert(xact->tType != KWalletTransaction::Unknown);

		switch (xact->tType) {
			case KWalletTransaction::Open:
				res = doTransactionOpen(xact->appid, xact->wallet, xact->wId, xact->modal);

				// multiple requests from the same client
				// should not produce multiple password
				// dialogs on a failure
				if (res < 0) {
					QList<KWalletTransaction *>::iterator it = _transactions.begin();
					Q_ASSERT(*it == xact);
					++it;
					for (; it != _transactions.end(); ++it) {
						KWalletTransaction *x = *it;
						if (xact->appid == x->appid && x->tType == KWalletTransaction::Open
							&& x->wallet == xact->wallet && x->wId == xact->wId) {
							x->tType = KWalletTransaction::OpenFail;
						}
					}
				}
				break;
			case KWalletTransaction::OpenFail:
				res = -1;
				break;
			case KWalletTransaction::ChangePassword:
				doTransactionChangePassword(xact->appid, xact->wallet, xact->wId);
				// fall through - no return
			default:
				_transactions.removeAll(xact);
				continue;
		}

		if (xact->tType != KWalletTransaction::ChangePassword) {
                    QDBusConnection::sessionBus().send(xact->msg.createReply(res));
		}
		_transactions.removeAll(xact);
	}

	processing = false;
}

#if 0
void KWalletD::openAsynchronous(const QString& wallet, const QByteArray& returnObject, uint wId, const QString& appid) {
	DCOPClient *dc = callingDcopClient();
	if (!dc) {
		return;
	}

	if (!_enabled ||
		!QRegExp("^[A-Za-z0-9]+[A-Za-z0-9\\s\\-_]*$").exactMatch(wallet)) {
		DCOPRef(appid, returnObject).send("walletOpenResult", -1);
		return;
	}

	KWalletTransaction *xact = new KWalletTransaction;

	xact->appid = appid;
	xact->wallet = wallet;
	xact->wId = wId;
	xact->modal = false;
	xact->tType = KWalletTransaction::Open;
	xact->returnObject = returnObject;
	_transactions.append(xact);

	DCOPRef(appid, returnObject).send("walletOpenResult", 0);

	QTimer::singleShot(0, this, SLOT(processTransactions()));
}
#endif

int KWalletD::openPath(const QString& path, qlonglong wId, const QString& appid) {
	if (!_enabled) { // guard
		return -1;
	}

	// FIXME: setup transaction
	int rc = internalOpen(appid, path, true, (WId)wId, false);
	return rc;
}


int KWalletD::open(const QString& wallet, qlonglong wId, const QString& appid, const QDBusMessage &msg) {
	if (!_enabled) { // guard
		return -1;
	}

	if (!QRegExp("^[A-Za-z0-9]+[A-Za-z0-9\\s\\-_]*$").exactMatch(wallet)) {
		return -1;
	}

	KWalletTransaction *xact = new KWalletTransaction;
	_transactions.append(xact);

	msg.setDelayedReply(true);
	xact->msg = msg;
	xact->appid = appid;
	xact->wallet = wallet;
	xact->wId = wId;
	xact->modal = true; // mark dialogs as modal, the app has blocking wait
	xact->tType = KWalletTransaction::Open;
	QTimer::singleShot(0, this, SLOT(processTransactions()));
	checkActiveDialog();
	return 0; // process later
}

// Sets up a dialog that will be shown by kwallet.
void KWalletD::setupDialog( QWidget* dialog, WId wId, const QString& appid, bool modal ) {
#ifdef Q_WS_X11
	if( wId != 0 )
		KWindowSystem::setMainWindow( dialog, wId ); // correct, set dialog parent
	else {
#endif
		if( appid.isEmpty())
			kWarning() << "Using kwallet without parent window!";
		else
			kWarning() << "Application '" << appid << "' using kwallet without parent window!";
		// allow dialog activation even if it interrupts, better than trying hacks
		// with keeping the dialog on top or on all desktops
		kapp->updateUserTimestamp();
#ifdef Q_WS_X11
	}
	if( modal )
		KWindowSystem::setState( dialog->winId(), NET::Modal );
	else
		KWindowSystem::clearState( dialog->winId(), NET::Modal );
#endif
	activeDialog = dialog;
}

// If there's a dialog already open and another application tries some operation that'd lead to
// opening a dialog, that application will be blocked by this dialog. A proper solution would
// be to set the second application's window also as a parent for the active dialog, so that
// KWin properly handles focus changes and so on, but there's currently no support for multiple
// dialog parents. Hopefully to be done in KDE4, for now just use all kinds of bad hacks to make
//  sure the user doesn't overlook the active dialog.
void KWalletD::checkActiveDialog() {
	if( !activeDialog || activeDialog->isHidden())
		return;
	kapp->updateUserTimestamp();
#ifdef Q_WS_X11
	KWindowSystem::setState( activeDialog->winId(), NET::KeepAbove );
	KWindowSystem::setOnAllDesktops( activeDialog->winId(), true );
	KWindowSystem::forceActiveWindow( activeDialog->winId());
#endif
}


int KWalletD::doTransactionOpen(const QString& appid, const QString& wallet, qlonglong wId, bool modal) {
	if (_firstUse && !wallets().contains(KWallet::Wallet::LocalWallet())) {
		// First use wizard
		KWalletWizard *wiz = new KWalletWizard(0);
		wiz->setWindowTitle(i18n("KDE Wallet Service"));
		setupDialog( wiz, (WId)wId, appid, modal );
		int rc = wiz->exec();
		if (rc == QDialog::Accepted) {
			bool useWallet = wiz->field("useWallet").toBool();
			KConfig kwalletrc("kwalletrc");
			KConfigGroup cfg(&kwalletrc, "Wallet");
			cfg.writeEntry("First Use", false);
			cfg.writeEntry("Enabled", useWallet);
			cfg.writeEntry("Close When Idle", wiz->field("closeWhenIdle").toBool());
			cfg.writeEntry("Use One Wallet", !wiz->field("networkWallet").toBool());
			cfg.sync();
			reconfigure();

			if (!useWallet) {
				delete wiz;
				return -1;
			}

			// Create the wallet
			KWallet::Backend *b = new KWallet::Backend(KWallet::Wallet::LocalWallet());
			QString pass = wiz->field("pass1").toString();
			QByteArray p(pass.toUtf8(), pass.length());
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
		KConfig kwalletrc("kwalletrc");
		KConfigGroup cfg(&kwalletrc, "Wallet");
		_firstUse = false;
		cfg.writeEntry("First Use", false);
	}

	int rc = internalOpen(appid, wallet, false, WId(wId), modal);
	return rc;
}


int KWalletD::internalOpen(const QString& appid, const QString& wallet, bool isPath, WId w, bool modal) {
	bool brandNew = false;

	QString thisApp;
	if (appid.isEmpty()) {
		thisApp = "KDE System";
	} else {
		thisApp = appid;
	}

	if (implicitDeny(wallet, thisApp)) {
		return -1;
	}

	const QPair<int, KWallet::Backend*> walletInfo = findWallet(wallet);
	int rc = walletInfo.first;
	if (rc == -1) {
		if (_wallets.count() > 20) {
			kDebug() << "Too many wallets open.";
			return -1;
		}

		KWallet::Backend *b = new KWallet::Backend(wallet, isPath);
		QString password;
		bool emptyPass = false;
		if ((isPath && QFile::exists(wallet)) || (!isPath && KWallet::Backend::exists(wallet))) {
			int pwless = b->open(QByteArray());
			if (0 != pwless || !b->isOpen()) {
				if (pwless == 0) {
					// release, start anew
					delete b;
					b = new KWallet::Backend(wallet, isPath);
				}
				KPasswordDialog *kpd = new KPasswordDialog();
				if (appid.isEmpty()) {
					kpd->setPrompt(i18n("<qt>KDE has requested to open the wallet '<b>%1</b>'. Please enter the password for this wallet below.</qt>", Qt::escape(wallet)));
				} else {
					kpd->setPrompt(i18n("<qt>The application '<b>%1</b>' has requested to open the wallet '<b>%2</b>'. Please enter the password for this wallet below.</qt>", Qt::escape(appid), Qt::escape(wallet)));
				}
				brandNew = false;
				// don't use KStdGuiItem::open() here which has trailing ellipsis!
				kpd->setButtonGuiItem(KDialog::Ok,KGuiItem( i18n( "&Open" ), "document-open"));
				kpd->setCaption(i18n("KDE Wallet Service"));
				while (!b->isOpen()) {
					setupDialog( kpd, w, appid, modal );
					if (kpd->exec() == KDialog::Accepted) {
						password = kpd->password();
						int rc = b->open(password.toUtf8());
						if (!b->isOpen()) {
							kpd->setPrompt(i18n("<qt>Error opening the wallet '<b>%1</b>'. Please try again.<br />(Error code %2: %3)</qt>", Qt::escape(wallet), rc, KWallet::Backend::openRCToString(rc)));
						}
					} else {
						break;
					}
				}
				delete kpd;
			} else {
				emptyPass = true;
			}
		} else {
			KNewPasswordDialog *kpd = new KNewPasswordDialog();
			if (wallet == KWallet::Wallet::LocalWallet() ||
						 wallet == KWallet::Wallet::NetworkWallet())
			{
				// Auto create these wallets.
				if (appid.isEmpty()) {
					kpd->setPrompt(i18n("KDE has requested to open the wallet. This is used to store sensitive data in a secure fashion. Please enter a password to use with this wallet or click cancel to deny the application's request."));
				} else {
					kpd->setPrompt(i18n("<qt>The application '<b>%1</b>' has requested to open the KDE wallet. This is used to store sensitive data in a secure fashion. Please enter a password to use with this wallet or click cancel to deny the application's request.</qt>", Qt::escape(appid)));
				}
			} else {
				if (appid.length() == 0) {
					kpd->setPrompt(i18n("<qt>KDE has requested to create a new wallet named '<b>%1</b>'. Please choose a password for this wallet, or cancel to deny the application's request.</qt>", Qt::escape(wallet)));
				} else {
					kpd->setPrompt(i18n("<qt>The application '<b>%1</b>' has requested to create a new wallet named '<b>%2</b>'. Please choose a password for this wallet, or cancel to deny the application's request.</qt>", Qt::escape(appid), Qt::escape(wallet)));
				}
			}
			brandNew = true;
			kpd->setCaption(i18n("KDE Wallet Service"));
			kpd->setButtonGuiItem(KDialog::Ok,KGuiItem(i18n("C&reate"),"document-new"));
			while (!b->isOpen()) {
				setupDialog( kpd, w, appid, modal );
				if (kpd->exec() == KDialog::Accepted) {
					password = kpd->password();
					int rc = b->open(password.toUtf8());
					if (!b->isOpen()) {
						kpd->setPrompt(i18n("<qt>Error opening the wallet '<b>%1</b>'. Please try again.<br />(Error code %2: %3)</qt>", Qt::escape(wallet), rc, KWallet::Backend::openRCToString(rc)));
					}
				} else {
					break;
				}
			}
			delete kpd;
		}



		if (!emptyPass && (password.isNull() || !b->isOpen())) {
			delete b;
			return -1;
		}

		if (emptyPass && _openPrompt && !isAuthorizedApp(appid, wallet, w)) {
			delete b;
			return -1;
		}

		_wallets.insert(rc = generateHandle(), b);
		if (emptyPass) {
			_passwords[wallet] = "";
		} else {
			_passwords[wallet] = password.toUtf8();
		}
		_handles[appid].append(rc);
		_synctimers[wallet] = new KWalletSyncTimer(this, wallet);
		connect(_synctimers[wallet], SIGNAL(timeoutSync(const QString&)), this, SLOT(doTransactionSync(const QString&)));

		if (brandNew) {
			createFolder(rc, KWallet::Wallet::PasswordFolder(), appid);
			createFolder(rc, KWallet::Wallet::FormDataFolder(), appid);
		}

		b->ref();
		if (_closeIdle && _timeouts) {
			_timeouts->addTimer(rc, _idleTime);
		}
		if (brandNew)
			emit walletCreated(wallet);
		emit walletOpened(wallet);
		if (_wallets.count() == 1 && _launchManager) {
			KToolInvocation::startServiceByDesktopName("kwalletmanager-kwalletd");
		}
	} else {
		if (!_handles[appid].contains(rc) && _openPrompt && !isAuthorizedApp(appid, wallet, w)) {
			return -1;
		}
		_handles[appid].append(rc);
		_wallets.value(rc)->ref();
	}

	return rc;
}


bool KWalletD::isAuthorizedApp(const QString& appid, const QString& wallet, WId w) {
	int response = 0;

	QString thisApp;
	if (appid.isEmpty()) {
		thisApp = "KDE System";
	} else {
		thisApp = appid;
	}

	if (!implicitAllow(wallet, thisApp)) {
		KConfigGroup cfg = KSharedConfig::openConfig("kwalletrc")->group("Auto Allow");
		if (!cfg.isEntryImmutable(wallet)) {
		    KBetterThanKDialog *dialog = new KBetterThanKDialog;
		    if (appid.isEmpty()) {
			dialog->setLabel(i18n("<qt>KDE has requested access to the open wallet '<b>%1</b>'.</qt>", Qt::escape(wallet)));
		    } else {
			dialog->setLabel(i18n("<qt>The application '<b>%1</b>' has requested access to the open wallet '<b>%2</b>'.</qt>", Qt::escape(QString(appid)), Qt::escape(wallet)));
		    }
		    setupDialog( dialog, w, appid, false );
		    response = dialog->exec();
		    delete dialog;
		}
	}

	if (response == 0 || response == 1) {
		if (response == 1) {
			KConfigGroup cfg = KSharedConfig::openConfig("kwalletrc")->group("Auto Allow");
			QStringList apps = cfg.readEntry(wallet, QStringList());
			if (!apps.contains(thisApp)) {
				if (cfg.isEntryImmutable(wallet)) {
					return false;
				}
				apps += thisApp;
				_implicitAllowMap[wallet] += thisApp;
				cfg.writeEntry(wallet, apps);
				cfg.sync();
			}
		}
	} else if (response == 3) {
		KConfigGroup cfg = KSharedConfig::openConfig("kwalletrc")->group("Auto Deny");
		QStringList apps = cfg.readEntry(wallet, QStringList());
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
		emit walletDeleted(wallet);
		return 0;
	}

	return -1;
}


void KWalletD::changePassword(const QString& wallet, qlonglong wId, const QString& appid, const QDBusMessage& msg) {
	KWalletTransaction *xact = new KWalletTransaction;

	//msg.setDelayedReply(true);
	xact->msg = msg;
	xact->appid = appid;
	xact->wallet = wallet;
	xact->wId = wId;
	xact->modal = false;
	xact->tType = KWalletTransaction::ChangePassword;

	_transactions.append(xact);

	QTimer::singleShot(0, this, SLOT(processTransactions()));
	checkActiveDialog();
	checkActiveDialog();
}

void KWalletD::initiateSync(const QString& wallet) {
	assert(_synctimers.contains(wallet));
	// start or restart the timer.
	_synctimers[wallet]->start();
}

void KWalletD::doTransactionChangePassword(const QString& appid, const QString& wallet, qlonglong wId) {

	const QPair<int, KWallet::Backend*> walletInfo = findWallet(wallet);
	int handle = walletInfo.first;
	KWallet::Backend* w = walletInfo.second;

	bool reclose = false;
	if (!w) {
		handle = doTransactionOpen(appid, wallet, wId, false);
		if (-1 == handle) {
			KMessageBox::sorryWId((WId)wId, i18n("Unable to open wallet. The wallet must be opened in order to change the password."), i18n("KDE Wallet Service"));
			return;
		}

		w = _wallets.value(handle);
		reclose = true;
	}

	assert(w);

	KNewPasswordDialog *kpd = new KNewPasswordDialog();
	kpd->setPrompt(i18n("<qt>Please choose a new password for the wallet '<b>%1</b>'.</qt>", Qt::escape(wallet)));
	kpd->setCaption(i18n("KDE Wallet Service"));
	kpd->setAllowEmptyPasswords(true);
	setupDialog( kpd, (WId)wId, appid, false );
	if (kpd->exec() == KDialog::Accepted) {
		QString p = kpd->password();
		if (!p.isNull()) {
			_passwords[wallet] = p.toUtf8();
			int rc = w->close(p.toUtf8());
			if (rc < 0) {
				KMessageBox::sorryWId((WId)wId, i18n("Error re-encrypting the wallet. Password was not changed."), i18n("KDE Wallet Service"));
				reclose = true;
			} else {
				rc = w->open(p.toUtf8());
				if (rc < 0) {
					KMessageBox::sorryWId((WId)wId, i18n("Error reopening the wallet. Data may be lost."), i18n("KDE Wallet Service"));
					reclose = true;
				}
			}
		}
	}

	delete kpd;

	if (reclose) {
		close(handle, true, appid);
	}
}


int KWalletD::close(const QString& wallet, bool force) {
	const QPair<int, KWallet::Backend*> walletInfo = findWallet(wallet);
	int handle = walletInfo.first;
	KWallet::Backend* w = walletInfo.second;

	return closeWallet(w, handle, force);
}


int KWalletD::closeWallet(KWallet::Backend *w, int handle, bool force) {
	if (w) {
		const QString& wallet = w->walletName();
		assert(_passwords.contains(wallet));
		assert(_synctimers.contains(wallet));
		if (w->refCount() == 0 || force) {
			invalidateHandle(handle);
			if (_closeIdle && _timeouts) {
				_timeouts->removeTimer(handle);
			}
			_wallets.remove(handle);
			delete _synctimers.take(wallet);
			if (_passwords.contains(wallet)) {
				w->close(QByteArray(_passwords[wallet].data(), _passwords[wallet].length()));
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


int KWalletD::close(int handle, bool force, const QString& appid) {
	KWallet::Backend *w = _wallets.value(handle);
	bool contains = false;

	if (w) { // the handle is valid
		if (_handles.contains(appid)) { // we know this app
			if (_handles[appid].contains(handle)) {
				// the app owns this handle
				_handles[appid].removeAt(_handles[appid].indexOf(handle));
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
			if (_synctimers.contains(w->walletName())) {
				delete _synctimers.take(w->walletName());
			}
			if (force) {
				invalidateHandle(handle);
			}
			if (_passwords.contains(w->walletName())) {
				w->close(QByteArray(_passwords[w->walletName()].data(), _passwords[w->walletName()].length()));
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


bool KWalletD::isOpen(const QString& wallet) {
	const QPair<int, KWallet::Backend*> walletInfo = findWallet(wallet);
	return walletInfo.second != 0;
}


bool KWalletD::isOpen(int handle) {
	if (handle == 0) {
		return false;
	}

	KWallet::Backend *rc = _wallets.value(handle);

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

	foreach (const QFileInfo &fi, dir.entryInfoList()) {
		QString fn = fi.fileName();
		if (fn.endsWith(".kwl")) {
			fn.truncate(fn.length()-4);
		}
		rc += fn;
	}
	return rc;
}


void KWalletD::sync(int handle, const QString& appid) {
	KWallet::Backend *b;
	
	// get the wallet and check if we have a password for it (safety measure)
	if ((b = getWallet(appid, handle)) && _passwords.contains(b->walletName())) {
		QByteArray p;
		QString wallet = b->walletName();
		p = QByteArray(_passwords[wallet].data(), _passwords[wallet].length());
		b->sync(p);
		p.fill(0);
	}
}

void KWalletD::doTransactionSync(const QString& wallet) {
	if (_synctimers.contains(wallet)) {
		_synctimers[wallet]->stop();
	}
	const QPair<int, KWallet::Backend*> walletInfo = findWallet(wallet);
	
	// check if we have a password for this wallet. discard if not.
	if (walletInfo.second && _passwords.contains(wallet)) {
		QByteArray p = QByteArray(_passwords[wallet].data(), _passwords[wallet].length());
		walletInfo.second->sync(p);
		p.fill(0);
	}
}


QStringList KWalletD::folderList(int handle, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		return b->folderList();
	}

	return QStringList();
}


bool KWalletD::hasFolder(int handle, const QString& f, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		return b->hasFolder(f);
	}

	return false;
}


bool KWalletD::removeFolder(int handle, const QString& f, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		bool rc = b->removeFolder(f);
		initiateSync(b->walletName());
		emit folderListUpdated(b->walletName());
		return rc;
	}

	return false;
}


bool KWalletD::createFolder(int handle, const QString& f, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		bool rc = b->createFolder(f);
		initiateSync(b->walletName());
		emit folderListUpdated(b->walletName());
		return rc;
	}

	return false;
}


QByteArray KWalletD::readMap(int handle, const QString& folder, const QString& key, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		b->setFolder(folder);
		KWallet::Entry *e = b->readEntry(key);
		if (e && e->type() == KWallet::Wallet::Map) {
			return e->map();
		}
	}

	return QByteArray();
}


QVariantMap KWalletD::readMapList(int handle, const QString& folder, const QString& key, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		b->setFolder(folder);
		QVariantMap rc;
		foreach (KWallet::Entry *entry, b->readEntryList(key)) {
			if (entry->type() == KWallet::Wallet::Map) {
				rc.insert(entry->key(), entry->map());
			}
		}
		return rc;
	}

	return QVariantMap();
}


QByteArray KWalletD::readEntry(int handle, const QString& folder, const QString& key, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		b->setFolder(folder);
		KWallet::Entry *e = b->readEntry(key);
		if (e) {
			return e->value();
		}
	}

	return QByteArray();
}


QVariantMap KWalletD::readEntryList(int handle, const QString& folder, const QString& key, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		b->setFolder(folder);
		QVariantMap rc;
		foreach (KWallet::Entry *entry, b->readEntryList(key)) {
			rc.insert(entry->key(), entry->value());
		}
		return rc;
	}

	return QVariantMap();
}


QStringList KWalletD::entryList(int handle, const QString& folder, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		b->setFolder(folder);
		return b->entryList();
	}

	return QStringList();
}


QString KWalletD::readPassword(int handle, const QString& folder, const QString& key, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		b->setFolder(folder);
		KWallet::Entry *e = b->readEntry(key);
		if (e && e->type() == KWallet::Wallet::Password) {
			return e->password();
		}
	}

	return QString();
}


QVariantMap KWalletD::readPasswordList(int handle, const QString& folder, const QString& key, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		b->setFolder(folder);
		QVariantMap rc;
		foreach (KWallet::Entry *entry, b->readEntryList(key)) {
			if (entry->type() == KWallet::Wallet::Password) {
				rc.insert(entry->key(), entry->password());
			}
		}
		return rc;
	}

	return QVariantMap();
}


int KWalletD::writeMap(int handle, const QString& folder, const QString& key, const QByteArray& value, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		b->setFolder(folder);
		KWallet::Entry e;
		e.setKey(key);
		e.setValue(value);
		e.setType(KWallet::Wallet::Map);
		b->writeEntry(&e);
		initiateSync(b->walletName());
		emitFolderUpdated(b->walletName(), folder);
		return 0;
	}

	return -1;
}


int KWalletD::writeEntry(int handle, const QString& folder, const QString& key, const QByteArray& value, int entryType, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		b->setFolder(folder);
		KWallet::Entry e;
		e.setKey(key);
		e.setValue(value);
		e.setType(KWallet::Wallet::EntryType(entryType));
		b->writeEntry(&e);
		initiateSync(b->walletName());
		emitFolderUpdated(b->walletName(), folder);
		return 0;
	}

	return -1;
}


int KWalletD::writeEntry(int handle, const QString& folder, const QString& key, const QByteArray& value, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		b->setFolder(folder);
		KWallet::Entry e;
		e.setKey(key);
		e.setValue(value);
		e.setType(KWallet::Wallet::Stream);
		b->writeEntry(&e);
		initiateSync(b->walletName());
		emitFolderUpdated(b->walletName(), folder);
		return 0;
	}

	return -1;
}


int KWalletD::writePassword(int handle, const QString& folder, const QString& key, const QString& value, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		b->setFolder(folder);
		KWallet::Entry e;
		e.setKey(key);
		e.setValue(value);
		e.setType(KWallet::Wallet::Password);
		b->writeEntry(&e);
		initiateSync(b->walletName());
		emitFolderUpdated(b->walletName(), folder);
		return 0;
	}

	return -1;
}


int KWalletD::entryType(int handle, const QString& folder, const QString& key, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
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


bool KWalletD::hasEntry(int handle, const QString& folder, const QString& key, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		if (!b->hasFolder(folder)) {
			return false;
		}
		b->setFolder(folder);
		return b->hasEntry(key);
	}

	return false;
}


int KWalletD::removeEntry(int handle, const QString& folder, const QString& key, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		if (!b->hasFolder(folder)) {
			return 0;
		}
		b->setFolder(folder);
		bool rc = b->removeEntry(key);
		initiateSync(b->walletName());
		emitFolderUpdated(b->walletName(), folder);
		return rc ? 0 : -3;
	}

	return -1;
}


void KWalletD::slotServiceUnregistered(const QString& app) {
	if (_handles.contains(app)) {
		QList<int> l = _handles[app];
		for (QList<int>::Iterator i = l.begin(); i != l.end(); ++i) {
			_handles[app].removeAll(*i);
			KWallet::Backend *w = _wallets.value(*i);
			if (w && !_leaveOpen && 0 == w->deref()) {
				close(w->walletName(), true);
			}
		}
		_handles.remove(app);
	}
}


void KWalletD::invalidateHandle(int handle) {
	for (QHash<QString,QList<int> >::Iterator i = _handles.begin();
							i != _handles.end();
									++i) {
		i.value().removeAll(handle);
	}
}


KWallet::Backend *KWalletD::getWallet(const QString& appid, int handle) {
	if (handle == 0) {
		return 0L;
	}

	KWallet::Backend *w = _wallets.value(handle);

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
	emit walletClosed(handle);
	emit walletClosed(wallet);
	if (_wallets.isEmpty()) {
		emit allWalletsClosed();
	}
}


int KWalletD::renameEntry(int handle, const QString& folder, const QString& oldName, const QString& newName, const QString& appid) {
	KWallet::Backend *b;

	if ((b = getWallet(appid, handle))) {
		b->setFolder(folder);
		int rc = b->renameEntry(oldName, newName);
		initiateSync(b->walletName());
		emitFolderUpdated(b->walletName(), folder);
		return rc;
	}

	return -1;
}


QStringList KWalletD::users(const QString& wallet) const {
	QStringList rc;

	const QPair<int, KWallet::Backend*> walletInfo = findWallet(wallet);
	int handle = walletInfo.first;
	KWallet::Backend* w = walletInfo.second;

	if (w) {
		for (QHash<QString,QList<int> >::ConstIterator hit = _handles.begin(); hit != _handles.end(); ++hit) {
			if (hit.value().contains(handle)) {
				rc.append(hit.key());
			}
		}
	}

	return rc;
}


bool KWalletD::disconnectApplication(const QString& wallet, const QString& application) {
	const QPair<int, KWallet::Backend*> walletInfo = findWallet(wallet);
	int handle = walletInfo.first;
	KWallet::Backend* backend = walletInfo.second;

	if (handle != -1 && _handles[application].contains(handle)) {
		_handles[application].removeAll(handle);

		if (_handles[application].isEmpty()) {
			_handles.remove(application);
		}

		if (backend->deref() == 0) {
			close(backend->walletName(), true);
		}

		emit applicationDisconnected(wallet, application);
		return true;
	}

	return false;
}


void KWalletD::emitFolderUpdated(const QString& wallet, const QString& folder) {
	emit folderUpdated(wallet, folder);
}


void KWalletD::emitWalletListDirty() {
	emit walletListDirty();
}


void KWalletD::reconfigure() {
	KConfig cfg("kwalletrc");
	KConfigGroup walletGroup(&cfg, "Wallet");
	_firstUse = walletGroup.readEntry("First Use", true);
	_enabled = walletGroup.readEntry("Enabled", true);
	_launchManager = walletGroup.readEntry("Launch Manager", true);
	_leaveOpen = walletGroup.readEntry("Leave Open", false);
	bool idleSave = _closeIdle;
	_closeIdle = walletGroup.readEntry("Close When Idle", false);
	_openPrompt = walletGroup.readEntry("Prompt on Open", true);
	int timeSave = _idleTime;
	// in minutes!
	_idleTime = walletGroup.readEntry("Idle Timeout", 10) * 60 * 1000;
#ifdef Q_WS_X11
	if ( screensaver->isValid() ) {
		if (walletGroup.readEntry("Close on Screensaver", false)) {
			connect(screensaver, SIGNAL(ActiveChanged(bool)), SLOT(screenSaverChanged(bool)));
		} else {
			screensaver->disconnect(SIGNAL(ActiveChanged(bool)), this, SLOT(screenSaverChanged(bool)));
		}
	}
#endif
	// Handle idle changes
	if (_closeIdle) {
		if (_idleTime != timeSave) { // Timer length changed
			Wallets::const_iterator it = _wallets.begin();
			const Wallets::const_iterator end = _wallets.end();
			for (; it != end; ++it) {
				_timeouts->resetTimer(it.key(), _idleTime);
			}
		}

		if (!idleSave) { // add timers for all the wallets
			Wallets::const_iterator it = _wallets.begin();
			const Wallets::const_iterator end = _wallets.end();
			for (; it != end; ++it) {
				_timeouts->addTimer(it.key(), _idleTime);
			}
		}
	} else {
		_timeouts->clear();
	}

	// Update the implicit allow stuff
	_implicitAllowMap.clear();
	const KConfigGroup autoAllowGroup(&cfg, "Auto Allow");
	QStringList entries = autoAllowGroup.entryMap().keys();
	for (QStringList::Iterator i = entries.begin(); i != entries.end(); ++i) {
		_implicitAllowMap[*i] = autoAllowGroup.readEntry(*i, QStringList());
	}

	// Update the implicit allow stuff
	_implicitDenyMap.clear();
	const KConfigGroup autoDenyGroup(&cfg, "Auto Deny");
	entries = autoDenyGroup.entryMap().keys();
	for (QStringList::Iterator i = entries.begin(); i != entries.end(); ++i) {
		_implicitDenyMap[*i] = autoDenyGroup.readEntry(*i, QStringList());
	}

	// Update if wallet was enabled/disabled
	if (!_enabled) { // close all wallets
		while (!_wallets.isEmpty()) {
			Wallets::const_iterator it = _wallets.begin();
			closeWallet(it.value(), it.key(), true);
		}
		KUniqueApplication::exit(0);
	}
}


bool KWalletD::isEnabled() const {
	return _enabled;
}


bool KWalletD::folderDoesNotExist(const QString& wallet, const QString& folder) {
	if (!wallets().contains(wallet)) {
		return true;
	}

	const QPair<int, KWallet::Backend*> walletInfo = findWallet(wallet);
	if (walletInfo.second) {
		return walletInfo.second->folderDoesNotExist(folder);
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

	const QPair<int, KWallet::Backend*> walletInfo = findWallet(wallet);
	if (walletInfo.second) {
		return walletInfo.second->entryDoesNotExist(folder, key);
	}

	KWallet::Backend *b = new KWallet::Backend(wallet);
	b->open(QByteArray());
	bool rc = b->entryDoesNotExist(folder, key);
	delete b;
	return rc;
}


bool KWalletD::implicitAllow(const QString& wallet, const QString& app) {
	return _implicitAllowMap[wallet].contains(app);
}


bool KWalletD::implicitDeny(const QString& wallet, const QString& app) {
	return _implicitDenyMap[wallet].contains(app);
}


void KWalletD::timedOut(int id) {
	KWallet::Backend *w = _wallets.value(id);
	if (w) {
		closeWallet(w, id, true);
	}
}


void KWalletD::closeAllWallets() {
	Wallets walletsCopy = _wallets;

	Wallets::const_iterator it = walletsCopy.begin();
	const Wallets::const_iterator end = walletsCopy.end();
	for (; it != end; ++it) {
		closeWallet(it.value(), it.key(), true);
	}

	walletsCopy.clear();

	// All of this should be basically noop.  Let's just be safe.
	_wallets.clear();

	for (QMap<QString,QByteArray>::Iterator it = _passwords.begin();
						it != _passwords.end();
						++it) {
		it.value().fill(0);
	}
	_passwords.clear();
}


QString KWalletD::networkWallet() {
	return KWallet::Wallet::NetworkWallet();
}


QString KWalletD::localWallet() {
	return KWallet::Wallet::LocalWallet();
}

void KWalletD::screenSaverChanged(bool s)
{
	if (s)
		closeAllWallets();
}

#include "kwalletd.moc"
#include "kwalletdadaptor.moc"
