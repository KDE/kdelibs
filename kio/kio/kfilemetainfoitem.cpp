/* This file is part of the KDE libraries

   Copyright (c) 2007 Jos van den Oever <jos@vandenoever.info>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
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

#include "kfilemetainfo_p.h"
#include <QDebug>

const QVariant KFileMetaInfoItemPrivate::null;

KFileMetaInfoItem::KFileMetaInfoItem() :p(new KFileMetaInfoItemPrivate()) {
}

KFileMetaInfoItem::KFileMetaInfoItem(const KFileMetaInfoItem& item) :p(item.p) {
}
KFileMetaInfoItem::KFileMetaInfoItem(const PredicateProperties& pp,
            const QVariant& v, KFileWritePlugin* w, bool e)
        :p(new KFileMetaInfoItemPrivate()) {
    p->pp = pp;
    p->value = v;
    p->writer = w;
    p->embedded = e;
    p->modified = false;
}
KFileMetaInfoItem::~KFileMetaInfoItem() {
}
const KFileMetaInfoItem&
KFileMetaInfoItem::operator=(const KFileMetaInfoItem& item) {
    p = item.p;
    return item;
}
const QString&
KFileMetaInfoItem::name() const {
    return p->pp.name();
}
const QVariant&
KFileMetaInfoItem::value() const {
    return p->value;
}
bool
KFileMetaInfoItem::setValue(const QVariant& value) {
    bool changed = p->value != value;
    p->value = value;
    p->modified |= changed;
    return changed;
}
bool
KFileMetaInfoItem::addValue(const QVariant& value) {
    QVariant& v = p->value;
    if (v.type() == QVariant::List) {
        QVariantList vl = v.toList();
        vl.append(value);
        p->value = vl;
    }
    return false;
}
bool
KFileMetaInfoItem::isModified() const {
    return p->modified;
}
bool
KFileMetaInfoItem::isRemoved() const {
    return p->modified && p->value.isNull();
}
bool
KFileMetaInfoItem::isValid() const {
    return true;
}
bool
KFileMetaInfoItem::isSkipped() const {
    // ########## TODO implement (vandenoever)
    return false;
}
const PredicateProperties&
KFileMetaInfoItem::properties() const {
    return p->pp;
}
bool
KFileMetaInfoItem::isEditable() const {
    return p->writer != NULL;
}
QString
KFileMetaInfoItem::suffix() const {
    // ########## TODO implement (vandenoever)
    return QString();
}

QString
KFileMetaInfoItem::prefix() const {
    // ########## TODO implement (vandenoever)
    return QString();
}
