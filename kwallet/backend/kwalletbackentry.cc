/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
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

#include "kwalletbackentry.h"





KWalletBackEntry::KWalletBackEntry() {
	_dirty = false;
}


KWalletBackEntry::~KWalletBackEntry() {
}


bool KWalletBackEntry::isDirty() const {
	return _dirty;
}


const QStringList& KWalletBackEntry::key() const {
	return _key;
}


const QByteArray& KWalletBackEntry::value() const {
	return _value;
}


void KWalletBackEntry::setValue(QByteArray& val) {
	// do a direct copy from one into the other without
	// temporary variables
}


void KWalletBackEntry::setKey(QStringList key) {
	_key = key;
}


void KWalletBackEntry::addKey(QString key) {
	_key << key;
}


