/* This file is part of the KDE project
 *
 * Copyright (C) 2002 George Staikos <staikos@kde.org>
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

#include <assert.h>


using namespace KWallet;

Wallet::Wallet(const QString& name) : _name(name), _dcopClient(new DCOPClient) {
	_open = false;
	_dcopClient->attach();
}


Wallet::~Wallet() {
	if (_open) {
		// FIXME: Discard changes
	}

	delete _dcopClient;
}


int Wallet::open(const QByteArray& password) {
	if (!_dcopClient->isAttached())
		return -100;

	if (_open) {
		return -1;
	}

	QByteArray data, retval;
	QCString rettype;
	QDataStream arg(data, IO_WriteOnly);
	arg << _name;
	arg << password;
	bool rc = _dcopClient->call("kded", "kwalletd",
				"open(const QString&, const QByteArray&)",
				data, rettype, retval, true);

	if (rc && rettype == "int") {
		QDataStream retStream(retval, IO_ReadOnly);
		int drc;
		retStream >> drc;
		if (drc == 0) {
			_open = true;
		}
		return drc;
	}
	return -99;
}

	
int Wallet::close(const QByteArray& password) {
	if (!_dcopClient->isAttached())
		return -100;

	if (!_open) {
		return -1;
	}

	QByteArray data, retval;
	QCString rettype;
	QDataStream arg(data, IO_WriteOnly);
	arg << _name;
	arg << password;
	bool rc = _dcopClient->call("kded", "kwalletd",
				"close(const QString&, const QByteArray&)",
				data, rettype, retval, true);

	if (rc && rettype == "int") {
		QDataStream retStream(retval, IO_ReadOnly);
		int drc;
		retStream >> drc;
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
	if (!_dcopClient->isAttached())
		return false;
	QByteArray data, retval;
	QCString rettype;
	QDataStream arg(data, IO_WriteOnly);
	arg << _name;
	bool rc = _dcopClient->call("kded", "kwalletd",
				"isOpen(const QString&)",
				data, rettype, retval, true);

	if (rc && rettype == "bool") {
		QDataStream retStream(retval, IO_ReadOnly);
		bool drc;
		retStream >> drc;
		_open = drc;
		return drc;
	}
	return false;
}


bool Wallet::changeWallet(const QString& name) {
	if (_open)
		return false;

	_name = name;

	return true;
}


const QPtrList<Entry>& Wallet::getEntriesByType(const QString& type) const {
	if (!_dcopClient->isAttached())
		return QPtrList<Entry>();
	Q_UNUSED(type);
return QPtrList<Entry>();
}


const QStringList Wallet::getTypeList() const {
	if (!_dcopClient->isAttached())
		return QStringList();
return QStringList();
}


