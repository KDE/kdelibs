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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kwalletd.h"

#include <assert.h>


extern "C" {
   KDEDModule *create_kwalletd(const QCString &name) {
	   return new KWalletD(name);
   }

   //void *__kde_do_unload;
}


KWalletD::KWalletD(const QCString &name) : KDEDModule(name)
{
}
  

KWalletD::~KWalletD()
{
	// Open wallets get closed without being saved of course.
	for (QMap<QString,KWallet::Backend*>::iterator it = _wallets.begin();
							it != _wallets.end();
									++it) {
		delete it.data();
	}
		
}


int KWalletD::open(const QString& wallet, const QByteArray& password) {
	if (_wallets.contains(wallet)) {
		assert(_wallets[wallet]->isOpen());
		_useCount[wallet]++;
		return 0;
	}

	_wallets[wallet] = new KWallet::Backend(wallet);

	int rc = _wallets[wallet]->open(password);
	if (rc != 0) {
		delete _wallets[wallet];
		_wallets.remove(wallet);
	} else {
		_useCount[wallet] = 1;
	}

	return rc;
}


int KWalletD::close(const QString& wallet, const QByteArray& password) {
	if (!_wallets.contains(wallet))
		return -99;

	if (!_wallets[wallet]->isOpen())
		return -98;

	int rc = _wallets[wallet]->close(password);
	_useCount[wallet]--;

	if (_useCount[wallet] == 0) {
		delete _wallets[wallet];
		_wallets.remove(wallet);
		_useCount.remove(wallet);
	}
	return rc;
}


bool KWalletD::isOpen(const QString& wallet) const {
	if (!_wallets.contains(wallet))
		return false;

	return _wallets[wallet]->isOpen();
}


#include "kwalletd.moc"

