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

#ifndef KABOUT_APPLICATION_PERSON_LIST_MODEL_H
#define KABOUT_APPLICATION_PERSON_LIST_MODEL_H

#include <QtCore/QAbstractListModel>

#include <kdecore/kernel/kaboutdata.h>

namespace KDEPrivate
{

class KAboutApplicationPersonListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    KAboutApplicationPersonListModel( const QList< KAboutPerson > &personList, QObject *parent = 0 );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data( const QModelIndex &index, int role ) const;

private:
    QList< KAboutPerson > m_personList;
};

} //namespace KDEPrivate

#endif // KABOUT_APPLICATION_PERSON_LIST_MODEL_H
