/* -*- C++ -*-

   This file implements the SMIVModel class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: SMIVModel.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "SMIVModel.h"
#include "SMIVItem.h"

#include <QtCore/QStringList>

#include <DebuggingAids.h>


SMIVModel::SMIVModel( QObject* parent )
    : QAbstractListModel( parent )
{
}

SMIVModel::~SMIVModel()
{
}

void SMIVModel::insert ( const SMIVItem* item )
{
    beginInsertRows( QModelIndex(),  m_data.count(), m_data.count() + 1 );
    m_data.append ( item );
    endInsertRows();
}

QVariant SMIVModel::headerData ( int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if (orientation == Qt::Horizontal)
    {
        return QString("Column %1").arg(section);
    } else {
        return QString("Image %1").arg(section);
    }
}

int SMIVModel::rowCount ( const QModelIndex& ) const
{
    return m_data.count();
}

QVariant SMIVModel::data ( const QModelIndex & index, int role ) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (index.row() < 0 || index.row() >= m_data.size())
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        return m_data.at(index.row())->name();
    } else {
        return QVariant();
    }
}

const SMIVItem* SMIVModel::data( int index ) const
{
    Q_ASSERT( index >=0 && index <m_data.size() );
    return m_data[index];
}

#include "SMIVModel.moc"
