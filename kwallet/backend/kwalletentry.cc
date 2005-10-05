/* This file is part of the KDE project
 *
 * Copyright (C) 2001-2003 George Staikos <staikos@kde.org>
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */ 

#include "kwalletentry.h"


using namespace KWallet;


Entry::Entry() {
}

Entry::~Entry() {
	_value.fill(0);
}

const QString& Entry::key() const {
	return _key;
}


const QByteArray& Entry::value() const {
	return _value;
}


QString Entry::password() const {
QString x;
	QDataStream qds(_value);
	qds >> x;
	return x;
}


void Entry::setValue(const QByteArray& val) {
	// do a direct copy from one into the other without
	// temporary variables
	_value.fill(0);
	_value.duplicate(val);
}


void Entry::setValue(const QString& val) {
	_value.fill(0);
	QDataStream qds(&_value, QIODevice::WriteOnly);
	qds << val;
}


void Entry::setKey(const QString& key) {
	_key = key;
}


Wallet::EntryType Entry::type() const {
	return _type;
}


void Entry::setType(Wallet::EntryType type) {
	_type = type;
}


void Entry::copy(const Entry* x) {
	_type = x->_type;
	_key = x->_key;
	_value.fill(0);
	_value.duplicate(x->_value);
}


