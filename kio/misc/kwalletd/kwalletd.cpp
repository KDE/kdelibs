/*
   This file is part of the KDE libraries

   Copyright (c) 2002 George Staikos <staikos@kde.org>

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

#include <kapplication.h>
#include <dcopclient.h>
#include "kwalletd.h"

#include <assert.h>


extern "C" {
   KDEDModule *create_kwalletd(const QCString &name) {
	   return new KWalletD(name);
   }

   //void *__kde_do_unload;
}


/*

   Design issues:

   - How do we track when a client disappears?

   - If we lose a client, do we prompt to close the wallet?
     (perhaps at shutdown time)

   - Should we have a flush() which closes the wallet then reopens?

   - Should we allow a wallet to be locked?


*/

KWalletD::KWalletD(const QCString &name)
: KDEDModule(name) {
	srand(time(0));
	KApplication::dcopClient()->setNotifications(true);
	connect(KApplication::dcopClient(),
		SIGNAL(applicationRemoved(const QCString&)),
		this,
		SLOT(slotAppUnregistered(const QCString&)));
}
  

KWalletD::~KWalletD() {
	// Open wallets get closed without being saved of course.
	for (QIntDictIterator<KWallet::Backend> it(_wallets);
						it.current();
							++it) {
		emitDCOPSignal("walletClosed(int)", it.currentKey());
		delete it.current();
		// FIXME: removeme later
		_wallets.replace(it.currentKey(), 0L);
	}
	_wallets.clear();
}


int KWalletD::open(const QString& wallet) {
}


int KWalletD::close(const QString& wallet, bool force) {
}


int KWalletD::close(int handle, bool force) {
}


bool KWalletD::isOpen(const QString& wallet) const {
}


bool KWalletD::isOpen(int handle) const {
}


QStringList KWalletD::wallets() const {
}


QStringList KWalletD::folderList(int handle) {
}


bool KWalletD::hasFolder(int handle, const QString& f) {
}


bool KWalletD::removeFolder(int handle, const QString& f) {
}


QByteArray KWalletD::readEntry(int handle, const QString& folder, const QString& key) {
}


QString KWalletD::readPassword(int handle, const QString& folder, const QString& key) {
}


int KWalletD::writeEntry(int handle, const QString& folder, const QString& key, const QByteArray& value) {
}


int KWalletD::writePassword(int handle, const QString& folder, const QString& key, const QString& value) {
}


bool KWalletD::hasEntry(int handle, const QString& folder, const QString& key) {
}


int KWalletD::removeEntry(int handle, const QString& folder, const QString& key) {
}


void KWalletD::slotAppUnregistered(const QCString& app) {
}


#include "kwalletd.moc"
