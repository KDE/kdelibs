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
#include <q3intdict.h>
#include <qstring.h>
#include <qwidget.h>
#include "kwalletbackend.h"
#include <Q3PtrList>
#include <time.h>
#include <stdlib.h>

class KDirWatch;
class KTimeout;

// @Private
class KWalletTransaction;

class KWalletD : public KDEDModule {
	Q_OBJECT
	K_DCOP
	public:
		KWalletD(const QByteArray &name);
		virtual ~KWalletD();

	k_dcop:
		// Is the wallet enabled?  If not, all open() calls fail.
		virtual bool isEnabled() const;

		// Open and unlock the wallet
		virtual int open(const QString& wallet, uint wId);

		// Open and unlock the wallet with this path
		virtual int openPath(const QString& path, uint wId);

		// Asynchronous open - must give the object to return the handle
		// to.
		virtual void openAsynchronous(const QString& wallet, const QByteArray& returnObject, uint wId);

		// Close and lock the wallet
		// If force = true, will close it for all users.  Behave.  This
		// can break applications, and is generally intended for use by
		// the wallet manager app only.
		virtual int close(const QString& wallet, bool force);
		virtual int close(int handle, bool force);

		// Save to disk but leave open
		virtual ASYNC sync(int handle);

		// Physically deletes the wallet from disk.
		virtual int deleteWallet(const QString& wallet);

		// Returns true if the wallet is open
		virtual bool isOpen(const QString& wallet);
		virtual bool isOpen(int handle);

		// List the users of this wallet
		virtual QStringList users(const QString& wallet) const;

		// Change the password of this wallet
		virtual void changePassword(const QString& wallet, uint wId);

		// A list of all wallets
		virtual QStringList wallets() const;

		// A list of all folders in this wallet
		virtual QStringList folderList(int handle);

		// Does this wallet have this folder?
		virtual bool hasFolder(int handle, const QString& folder);

		// Create this folder
		virtual bool createFolder(int handle, const QString& folder);

		// Remove this folder
		virtual bool removeFolder(int handle, const QString& folder);

		// List of entries in this folder
		virtual QStringList entryList(int handle, const QString& folder);

		// Read an entry.  If the entry does not exist, it just
		// returns an empty result.  It is your responsibility to check
		// hasEntry() first.
		virtual QByteArray readEntry(int handle, const QString& folder, const QString& key);
		virtual QByteArray readMap(int handle, const QString& folder, const QString& key);
		virtual QString readPassword(int handle, const QString& folder, const QString& key);
		virtual QMap<QString, QByteArray> readEntryList(int handle, const QString& folder, const QString& key);
		virtual QMap<QString, QByteArray> readMapList(int handle, const QString& folder, const QString& key);
		virtual QMap<QString, QString> readPasswordList(int handle, const QString& folder, const QString& key);

		// Rename an entry.  rc=0 on success.
		virtual int renameEntry(int handle, const QString& folder, const QString& oldName, const QString& newName);

		// Write an entry.  rc=0 on success.
		virtual int writeEntry(int handle, const QString& folder, const QString& key, const QByteArray& value, int entryType);
		virtual int writeEntry(int handle, const QString& folder, const QString& key, const QByteArray& value);
		virtual int writeMap(int handle, const QString& folder, const QString& key, const QByteArray& value);
		virtual int writePassword(int handle, const QString& folder, const QString& key, const QString& value);

		// Does the entry exist?
		virtual bool hasEntry(int handle, const QString& folder, const QString& key);

		// What type is the entry?
		virtual int entryType(int handle, const QString& folder, const QString& key);

		// Remove an entry.  rc=0 on success.
		virtual int removeEntry(int handle, const QString& folder, const QString& key);

		// Disconnect an app from a wallet
		virtual bool disconnectApplication(const QString& wallet, const QByteArray& application);

		virtual void reconfigure();

		// Determine
		virtual bool folderDoesNotExist(const QString& wallet, const QString& folder);
		virtual bool keyDoesNotExist(const QString& wallet, const QString& folder, const QString& key);

		virtual void closeAllWallets();

		virtual QString networkWallet();

		virtual QString localWallet();

	private slots:
		void slotAppUnregistered(const QByteArray& app);
		void emitWalletListDirty();
		void timedOut(int);
		void notifyFailures();
		void processTransactions();

	private:
		int internalOpen(const QByteArray& appid, const QString& wallet, bool isPath = false, WId w = 0);
		bool isAuthorizedApp(const QByteArray& appid, const QString& wallet, WId w);
		// This also validates the handle.  May return NULL.
		KWallet::Backend* getWallet(const QByteArray& appid, int handle);
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
		bool implicitAllow(const QString& wallet, const QByteArray& app);
		bool implicitDeny(const QString& wallet, const QByteArray& app);
		QByteArray friendlyDCOPPeerName();

		void doTransactionChangePassword(const QByteArray& appid, const QString& wallet, uint wId);
		int doTransactionOpen(const QByteArray& appid, const QString& wallet, uint wId);

		Q3IntDict<KWallet::Backend> _wallets;
		QMap<QByteArray,QList<int> > _handles;
		QMap<QString,QByteArray> _passwords;
		KDirWatch *_dw;
		int _failed;

		bool _leaveOpen, _closeIdle, _launchManager, _enabled;
	       	bool _openPrompt, _firstUse, _showingFailureNotify;
		int _idleTime;
		QMap<QString,QStringList> _implicitAllowMap, _implicitDenyMap;
		KTimeout *_timeouts;

		Q3PtrList<KWalletTransaction> _transactions;
};


#endif
