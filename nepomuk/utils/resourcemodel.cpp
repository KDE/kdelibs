/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2007-2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "resourcemodel.h"

#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtCore/QMimeData>

#include "kurl.h"
#include "kdebug.h"
#include "kcategorizedsortfilterproxymodel.h"
#include "kicon.h"
#include "klocale.h"

#include "resource.h"
#include "class.h"
#include "variant.h"

#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/NAO>


class Nepomuk::Utils::ResourceModel::Private
{
public:
};


Nepomuk::Utils::ResourceModel::ResourceModel( QObject* parent )
    : QAbstractItemModel( parent ),
      d( new Private() )
{
}


Nepomuk::Utils::ResourceModel::~ResourceModel()
{
    delete d;
}


QModelIndex Nepomuk::Utils::ResourceModel::parent( const QModelIndex& child ) const
{
    Q_UNUSED(child);
    return QModelIndex();
}


int Nepomuk::Utils::ResourceModel::columnCount( const QModelIndex& parent ) const
{
    Q_UNUSED(parent);
    return 1;
}


QVariant Nepomuk::Utils::ResourceModel::data( const QModelIndex& index, int role ) const
{
    Nepomuk::Resource res = resourceForIndex( index );

    if( res.isValid() ) {
        switch( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return res.genericLabel();

        case Qt::DecorationRole: {
            QString iconName = res.genericIcon();
            if( !iconName.isEmpty() ) {
                return KIcon( iconName );
            }
            else {
                QIcon icon = Types::Class(res.resourceType()).icon();
                if( !icon.isNull() )
                    return icon;
                else
                    return QVariant();
            }
        }

        case Qt::ToolTipRole:
            return KUrl( res.resourceUri() ).prettyUrl();

        case KCategorizedSortFilterProxyModel::CategorySortRole:
            // FIXME: sort files before other stuff and so on

        case KCategorizedSortFilterProxyModel::CategoryDisplayRole: {
            Q_ASSERT( !res.resourceType().isEmpty() );
            Nepomuk::Types::Class c( res.resourceType() );
            QString cat = c.label();
            if ( cat.isEmpty() ) {
                cat = c.name();
            }
            if ( c.uri() == Soprano::Vocabulary::RDFS::Resource() || cat.isEmpty() ) {
                cat = i18n( "Miscellaneous" );
            }

            return cat;
        }

        case ResourceRole:
            return QVariant::fromValue( res );

        case ResourceCreationDate:
            return res.property( Soprano::Vocabulary::NAO::created() ).toDateTime();
        }
    }
    return QVariant();
}


Qt::ItemFlags Nepomuk::Utils::ResourceModel::flags( const QModelIndex& index ) const
{
    if ( index.isValid() ) {
        return QAbstractItemModel::flags( index ) | Qt::ItemIsDragEnabled;
    }
    else {
        return QAbstractItemModel::flags( index );
    }
}


QMimeData* Nepomuk::Utils::ResourceModel::mimeData( const QModelIndexList& indexes ) const
{
    KUrl::List uris;
    foreach ( const QModelIndex& index, indexes ) {
        if (index.isValid()) {
            uris << index.data( ResourceRole ).value<Resource>().resourceUri();
        }
    }

    QMimeData* mimeData = new QMimeData();
    uris.populateMimeData( mimeData );

    QByteArray data;
    QDataStream s( &data, QIODevice::WriteOnly );
    s << uris;
    mimeData->setData( mimeTypes().first(), data );

    return mimeData;
}


QStringList Nepomuk::Utils::ResourceModel::mimeTypes() const
{
    return( QStringList()
            << QLatin1String( "application/x-nepomuk-resource-uri" )
            << KUrl::List::mimeDataTypes() );
}

#include "resourcemodel.moc"
