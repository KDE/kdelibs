/* This file is part of the KDE libraries

   Copyright (c) 2007 Jos van den Oever <jos@vandenoever.info>
                 2010 Sebastian Trueg <trueg@kde.org>

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

#include "kfilemetainfoitem.h"
#include "kfilemetainfoitem_p.h"
#include <config-kde4support.h>

#ifdef HAVE_NEPOMUK
#include <config-nepomuk.h>
#endif

KFileMetaInfoItem::KFileMetaInfoItem() : d(new KFileMetaInfoItemPrivate()) {
}

KFileMetaInfoItem::KFileMetaInfoItem(const KFileMetaInfoItem& item) : d(item.d) {
}
KFileMetaInfoItem::KFileMetaInfoItem(const QString& pp,
                                     const QVariant& v, KFileWritePlugin* w, bool e)
    : d(new KFileMetaInfoItemPrivate()) {
#ifndef KDE_NO_DEPRECATED
    d->pp = pp;
#else
#if ! KIO_NO_NEPOMUK
    d->pp = QUrl(pp);
#endif
#endif
    d->value = v;
    d->writer = w;
    d->embedded = e;
    d->modified = false;
}
KFileMetaInfoItem::~KFileMetaInfoItem() {
}
const KFileMetaInfoItem&
KFileMetaInfoItem::operator=(const KFileMetaInfoItem& item) {
    d = item.d;
    return item;
}
const QString&
KFileMetaInfoItem::name() const {
#ifndef KDE_NO_DEPRECATED
    return d->pp.name();
#else
    #if ! KIO_NO_NEPOMUK
        return d->pp.name();
    #else
        return QString::null;
    #endif
#endif	
}
const QVariant&
KFileMetaInfoItem::value() const {
    return d->value;
}
bool
KFileMetaInfoItem::setValue(const QVariant& value) {
    bool changed = d->value != value;
    d->value = value;
    d->modified |= changed;
    return changed;
}
bool
KFileMetaInfoItem::addValue(const QVariant& value) {
    QVariant& v = d->value;
    if (v.type() == QVariant::List) {
        QVariantList vl = v.toList();
        vl.append(value);
        d->value = vl;
    }
    return false;
}
bool
KFileMetaInfoItem::isModified() const {
    return d->modified;
}
bool
KFileMetaInfoItem::isRemoved() const {
    return d->modified && d->value.isNull();
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
#ifndef KDE_NO_DEPRECATED
const PredicateProperties&
KFileMetaInfoItem::properties() const {
    return d->pp;
}
#endif
bool
KFileMetaInfoItem::isEditable() const {
    return d->writer != 0;
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
