/* This file is part of the KDE libraries
   Copyright (C) 2010 Téo Mrnjavac <teo@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kaboutapplicationpersonlistmodel_p.h"

#include <attica/providermanager.h>

#include <kdecore/io/kdebug.h>

namespace KDEPrivate
{

KAboutApplicationPersonListModel::KAboutApplicationPersonListModel( const QList< KAboutPerson > &personList,
                                                                    QObject *parent )
    : QAbstractListModel( parent )
    , m_personList( personList )
{


}

int
KAboutApplicationPersonListModel::rowCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent )
    return m_personList.count();
}

QVariant
KAboutApplicationPersonListModel::data( const QModelIndex &index, int role ) const
{
    if( !index.isValid() )
    {
        kDebug()<<"ERROR: invalid index";
        return QVariant();
    }
    if( index.row() >= rowCount() )
    {
        kDebug()<<"ERROR: index out of bounds";
        return QVariant();
    }
    if( role == Qt::DisplayRole )
    {
        return m_personList.at( index.row() ).name();
    }
    else
    {
        return QVariant();
    }

}

} //namespace KDEPrivate
