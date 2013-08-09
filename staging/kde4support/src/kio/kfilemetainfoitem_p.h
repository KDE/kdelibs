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
#ifndef KFILEMETAINFOITEM_P_H
#define KFILEMETAINFOITEM_P_H

#include <config-kde4support.h>

#ifndef KDE_NO_DEPRECATED
#include <predicateproperties.h>
#else
#if ! KIO_NO_NEPOMUK
#include <nepomuk/types/property.h>
#endif
#endif

#include <QtCore/QHash>
#include <QtCore/QSharedData>

class KFileWritePlugin;

class KFileMetaInfoItemPrivate : public QSharedData {
public:
    KFileMetaInfoItemPrivate() : writer(0) {}
#ifndef KDE_NO_DEPRECATED
    PredicateProperties pp;
#else
#if ! KIO_NO_NEPOMUK
    Nepomuk::Types::Property pp;
#endif
#endif
    QVariant value;
    KFileWritePlugin* writer;
    bool embedded;
    bool modified;
};

#endif
