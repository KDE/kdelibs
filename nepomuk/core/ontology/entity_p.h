/* This file is part of the Nepomuk-KDE libraries
    Copyright (c) 2007-2009 Sebastian Trueg <trueg@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
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

#ifndef _NEPOMUK_ENTITY_PRIVATE_H_
#define _NEPOMUK_ENTITY_PRIVATE_H_

#include "entity.h"

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtGui/QIcon>
#include <QtCore/QSharedData>
#include <QtCore/QMutex>

namespace Nepomuk {
    namespace Types {
        class EntityPrivate : public QSharedData
        {
        public:
            EntityPrivate( const QUrl& uri = QUrl() );
            virtual ~EntityPrivate() {}

            QMutex mutex;

            QUrl uri;
            QString label;
            QString comment;
            QString l10nLabel;
            QString l10nComment;

            QIcon icon;

            // -1 - unknown
            // 0  - no
            // 1  - yes
            int available;
            int ancestorsAvailable;

            void init();
            void initAncestors();

            virtual bool addProperty( const QUrl& property, const Soprano::Node& value ) = 0;
            virtual bool addAncestorProperty( const QUrl& ancestorResource, const QUrl& property ) = 0;

            virtual void reset( bool recursive );

        protected:
            virtual bool load();
            virtual bool loadAncestors();
        };
    }
}



class Nepomuk::Entity::Private : public QSharedData
{
public:
    Private()
	: ontology( 0 ) {
    }

    const Ontology* ontology;

    QUrl uri;
    QString label;
    QString comment;
    QHash<QString, QString> l10nLabels;
    QHash<QString, QString> l10nComments;
};

#endif
