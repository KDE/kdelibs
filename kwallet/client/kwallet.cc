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

#include <assert.h>


using namespace KWallet;

Wallet::Wallet(const QString& name) : _name(name) {
	_open = false;
}


Wallet::~Wallet() {
	if (_open) {
		// FIXME: Discard changes
	}
}


int Wallet::open(const QByteArray& password) {
	Q_UNUSED(password);
	return 0;
}

	
int Wallet::close(const QByteArray& password) {
	Q_UNUSED(password);
	return 0;
}


const QString& Wallet::walletName() const {
	return _name;
}


bool Wallet::isOpen() const {
	return _open;
}


bool Wallet::changeWallet(const QString& name) {
	Q_UNUSED(name);
	if (_open)
		return false;

	return true;
}


const QPtrList<Entry>& Wallet::getEntriesByType(const QString& type) const {
	Q_UNUSED(type);
return QPtrList<Entry>();
}


const QStringList Wallet::getTypeList() const {
return QStringList();
}


