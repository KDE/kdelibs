/* This file is part of the KDE project
   Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "browserinterface.h"

#include <QtCore/QMetaEnum>
#include <QtCore/QStringList>

#include <config.h>

using namespace KParts;

BrowserInterface::BrowserInterface( QObject *parent )
    : QObject( parent )
{
}

BrowserInterface::~BrowserInterface()
{
}

void BrowserInterface::callMethod( const char *name, const QVariant &argument )
{
    switch ( argument.type() )
    {
        case QVariant::Invalid:
            break;
        case QVariant::String:
            QMetaObject::invokeMethod( this, name,
                                       Q_ARG( QString, argument.toString() ) );
            break;
        case QVariant::StringList:
        {
	    QStringList strLst = argument.toStringList();
            QMetaObject::invokeMethod( this, name,
                                       Q_ARG( QStringList *, &strLst ) );
            break;
        }
        case QVariant::Int:
            QMetaObject::invokeMethod( this, name,
                                       Q_ARG( int, argument.toInt() ) );
            break;
        case QVariant::UInt:
        {
	    unsigned int i = argument.toUInt();
            QMetaObject::invokeMethod( this, name,
                                       Q_ARG( unsigned int *, &i ) );
            break;
        }
        case QVariant::Bool:
            QMetaObject::invokeMethod( this, name,
                                       Q_ARG( bool, argument.toBool() ) );
            break;
        default:
            break;
    }
}

#include "browserinterface.moc"
