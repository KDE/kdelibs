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

Wallet::Wallet()
: QObject(0L), d(0), _dcopClient(new DCOPClient) {
	_open = false;
	_dcopClient->attach();
	_dcopRef = new DCOPRef("kded", "kwalletd");
	_dcopRef->setDCOPClient(_dcopClient);
}


Wallet::~Wallet() {
	if (_open) {
		// FIXME
	}

	delete _dcopRef;
	_dcopRef = 0L;
	delete _dcopClient;
	_dcopClient = 0L;
}


Wallet *Wallet::openWallet(const QString& name) {
DCOPReply r = DCOPRef("kded", "kwalletd").call("open", name);
	if (r.isValid()) {
		int drc ;
		r.get(drc);
		if (drc != -1) {
			// FIXME
		}
	}
return 0;
}


#if 0
int Wallet::close(const QByteArray& password) {
	if (!_open) {
		return -1;
	}

	DCOPReply r = _kwalletdRef->call("close", _name, password);
	if (r.isValid()) {
		int drc;
		r.get(drc);
		if (drc == 0) {
			_open = false;
		}
		return drc;
	}
	return -99;
}


const QString& Wallet::walletName() const {
	return _name;
}


bool Wallet::isOpen() {
	DCOPReply r = _kwalletdRef->call("isOpen", _name);
	if (r.isValid()) {
		bool drc;
		r.get(drc);
		_open = drc;
		return drc;
	}
	return false;
}


#endif

#include "kwallet.moc"
