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

#include "kwalletentry.h"


using namespace KWallet;


Entry::Entry() {
	_dirty = false;
}


Entry::Entry(const QString &type,
	     const Key &key, 
	     const Value &value)  :
	_type(type), 
	_dirty(false),
	_key(key), 
	_value(value) {
}


Entry::~Entry() {
	_value.fill(0);
}


void Entry::clearDirty() {
	_dirty = false;
}


bool Entry::isDirty() const {
	return _dirty;
}


const Key& Entry::key() const {
	return _key;
}


const Value& Entry::value() const {
	return _value;
}


void Entry::setValue(const Value& val) {
	// do a direct copy from one into the other without
	// temporary variables
	_value.fill(0);
	_value.duplicate(val);
	_dirty = true;
}


void Entry::setKey(const Key& key) {
	_key = key;
	_dirty = true;
}


void Entry::addKey(NVPair& key) {
	_key.insert(key);
	_dirty = true;
}


void Entry::addKey(const QString& name, const QString& value) {
	_key[name] = value;
	_dirty = true;
}


QString Entry::type() const {
	return _type;
}


QPtrList<Entry> Entry::getEntries(const QString& type, 
				  EntryFactory factory,
				  const QString& keyName,
				  const QString& keyValue) {
	// TODO: get list of entries (using the backend)
	return QPtrList<Entry>();
}

