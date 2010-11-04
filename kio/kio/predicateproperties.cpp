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

#include "predicateproperties.h"

#include <QtCore/QStringList>

using namespace std;

class PredicateProperties::Private : public QSharedData {
public:
    static const QString nullString;
    static const QStringList nullStringList;
#ifndef KDE_NO_DEPRECATED
    static const PredicateProperties nullPP;
#endif
    PredicateProperties parent;
    QString key;
    QString name;
    QVariant::Type type;
    uint attributes;
};
const QString PredicateProperties::Private::nullString;
const QStringList PredicateProperties::Private::nullStringList;
#ifndef KDE_NO_DEPRECATED
const PredicateProperties PredicateProperties::Private::nullPP;
#endif

#ifndef KDE_NO_DEPRECATED
PredicateProperties::PredicateProperties(const QString& predicate) {
    if (!predicate.isEmpty()) {
        d = new Private();
        d->key = predicate;
    }
}
#endif
PredicateProperties::PredicateProperties(const PredicateProperties& pp)
        : d(pp.d) {
}
PredicateProperties::~PredicateProperties() {
}
const PredicateProperties&
PredicateProperties::operator=(const PredicateProperties& pp) {
    d = pp.d;
    return pp;
}
const QString&
PredicateProperties::name() const {
    if ( d == 0) return Private::nullString;
    return ( d->name.isEmpty()) ? d->key : d->name;
}

const QStringList&
PredicateProperties::suggestedValues() const {
    return Private::nullStringList;
}

uint
PredicateProperties::minCardinality() const {
    return 0;
}

uint
PredicateProperties::maxCardinality() const {
    return 0;
}

uint
PredicateProperties::attributes() const {
    return ( d) ? d->attributes :0;
}
QVariant::Type
PredicateProperties::type() const {
    return ( d) ? d->type :QVariant::Invalid;
}
QValidator*
PredicateProperties::createValidator() const {
    return 0;
}
const PredicateProperties&
PredicateProperties::parent() const {
#ifndef KDE_NO_DEPRECATED
    return ( d) ? d->parent :Private::nullPP;
#else
    return d->parent;
#endif
}
bool
PredicateProperties::isValid() const {
    return d;
}
