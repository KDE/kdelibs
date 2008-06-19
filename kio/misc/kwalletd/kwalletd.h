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
#ifndef _KWALLETD_H_
#define _KWALLETD_H_

#include <kdedmodule.h>
#include <QtCore/QString>
#include <QtGui/QImage>
#include <QtCore/QHash>
#include "kwalletbackend.h"
#include <QtCore/QPointer>
#include <time.h>
#include <stdlib.h>
#include <QtDBus/QtDBus>

class KDirWatch;
class KTimeout;

// @Private
class KWalletTransaction;
class KWalletSyncTimer;

class KWalletD : public QObject {
	Q_OBJECT
	
	public:
		KWalletD();
		virtual ~KWalletD();
	
	public Q_SLOTS:
		// Is the wallet enabled?  If not, all open() calls fail.
		bool isEnabled() const;

		// Open and unlock the wallet
		int open(const QString& wallet, qlonglong wId, const QString& appid, const QDBusMessage& msg);

		// Open and unlock the wallet with this path
		int openPath(const QString& path, qlonglong wId, const QString& appid);

		// Asynchronous open - must give the object to return the handle
		// to.
		// disabled -thiago
		//virtual void openAsynchronous(const QString& wallet, const QByteArray& returnObject, uint wId, const QString& appid);

		// Close and lock the wallet
		// If force = true, will close it for all users.  Behave.  This
		// can break applications, and is generally intended for use by
		// the wallet manager app only.
		int close(const QString& wallet, bool force);
		int close(int handle, bool force, const QString& appid);

		// Save to disk but leave open
		Q_NOREPLY void sync(int handle, const QString& appid);

		// Physically deletes the wallet from disk.
		int deleteWallet(const QString& wallet);

		// Returns true if the wallet is open
		bool isOpen(const QString& wallet);
		bool isOpen(int handle);

		// List the users of this wallet
		QStringList users(const QString& wallet) const;

		// Change the password of this wallet
		void changePassword(const QString& wallet, qlonglong wId, const QString& appid, const QDBusMessage& msg);

		// A list of all wallets
		QStringList wallets() const;

		// A list of all folders in this wallet
		QStringList folderList(int handle, const QString& appid);

		// Does this wallet have this folder?
		bool hasFolder(int handle, const QString& folder, const QString& appid);

		// Create this folder
		bool createFolder(int handle, const QString& folder, const QString& appid);

		// Remove this folder
		bool removeFolder(int handle, const QString& folder, const QString& appid);

		// List of entries in this folder
		QStringList entryList(int handle, const QString& folder, const QString& appid);

		// Read an entry.  If the entry does not exist, it just
		// returns an empty result.	 It is your responsibility to check
		// hasEntry() first.
		QByteArray readEntry(int handle, const QString& folder, const QString& key, const QString& appid);
		QByteArray readMap(int handle, const QString& folder, const QString& key, const QString& appid);
		QString readPassword(int handle, const QString& folder, const QString& key, const QString& appid);
		QVariantMap readEntryList(int handle, const QString& folder, const QString& key, const QString& appid);
		QVariantMap readMapList(int handle, const QString& folder, const QString& key, const QString& appid);
		QVariantMap readPasswordList(int handle, const QString& folder, const QString& key, const QString& appid);

		// Rename an entry.	 rc=0 on success.
		int renameEntry(int handle, const QString& folder, const QString& oldName, const QString& newName, const QString& appid);

		// Write an entry.	rc=0 on success.
		int writeEntry(int handle, const QString& folder, const QString& key, const QByteArray& value, int entryType, const QString& appid);
		int writeEntry(int handle, const QString& folder, const QString& key, const QByteArray& value, const QString& appid);
		int writeMap(int handle, const QString& folder, const QString& key, const QByteArray& value, const QString& appid);
		int writePassword(int handle, const QString& folder, const QString& key, const QString& value, const QString& appid);

		// Does the entry exist?
		bool hasEntry(int handle, const QString& folder, const QString& key, const QString& appid);

		// What type is the entry?
		int entryType(int handle, const QString& folder, const QString& key, const QString& appid);

		// Remove an entry.	 rc=0 on success.
		int removeEntry(int handle, const QString& folder, const QString& key, const QString& appid);

		// Disconnect an app from a wallet
		bool disconnectApplication(const QString& wallet, const QString& application);

		void reconfigure();

		// Determine
		bool folderDoesNotExist(const QString& wallet, const QString& folder);
		bool keyDoesNotExist(const QString& wallet, const QString& folder, const QString& key);

		void closeAllWallets();

		QString networkWallet();

		QString localWallet();

		void screenSaverChanged(bool);

	Q_SIGNALS:
		void walletListDirty();
		void walletCreated(const QString& wallet);
		void walletOpened(const QString& wallet);
		void walletDeleted(const QString& wallet);
		void walletClosed(const QString& wallet);
		void walletClosed(int handle);
		void allWalletsClosed();
		void folderListUpdated(const QString& wallet);
		void folderUpdated(const QString&, const QString&);
		void applicationDisconnected(const QString& wallet, const QString& application);

	private Q_SLOTS:
		void slotServiceUnregistered(const QString& name);
		void emitWalletListDirty();
		void timedOut(int);
		void notifyFailures();
		void processTransactions();
		void doTransactionSync(const QString& wallet);

	private:
		int internalOpen(const QString& appid, const QString& wallet, bool isPath, WId w, bool modal);
		bool isAuthorizedApp(const QString& appid, const QString& wallet, WId w);
		// This also validates the handle.	May return NULL.
		KWallet::Backend* getWallet(const QString& appid, int handle);
		// Generate a new unique handle.
		int generateHandle();
		// Invalidate a handle (remove it from the QMap)
		void invalidateHandle(int handle);
		// Emit signals about closing wallets
		void doCloseSignals(int,const QString&);
		void emitFolderUpdated(const QString&, const QString&);
		// Internal - close this wallet.
		int closeWallet(KWallet::Backend *w, int handle, bool force);
		// Implicitly allow access for this application
		bool implicitAllow(const QString& wallet, const QString& app);
		bool implicitDeny(const QString& wallet, const QString& app);

		void doTransactionChangePassword(const QString& appid, const QString& wallet, qlonglong wId);
		int doTransactionOpen(const QString& appid, const QString& wallet, qlonglong wId, bool modal);
		void initiateSync(const QString& wallet);

		void setupDialog( QWidget* dialog, WId wId, const QString& appid, bool modal );
		void checkActiveDialog();

		QPair<int, KWallet::Backend*> findWallet(const QString& walletName) const;

		typedef QHash<int, KWallet::Backend *> Wallets;
		Wallets _wallets;
		QHash<QString,QList<int> > _handles;
		QMap<QString,QByteArray> _passwords;
		KDirWatch *_dw;
		int _failed;

		bool _leaveOpen, _closeIdle, _launchManager, _enabled;
		bool _openPrompt, _firstUse, _showingFailureNotify;
		int _idleTime;
		QMap<QString,QStringList> _implicitAllowMap, _implicitDenyMap;
		KTimeout *_timeouts;

		QList<KWalletTransaction*> _transactions;
		QMap<QString,KWalletSyncTimer*> _synctimers;
		QPointer< QWidget > activeDialog;
#ifdef Q_WS_X11
		QDBusInterface *screensaver;
#endif
};


#endif
