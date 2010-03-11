/* This file is part of the KDE Project
   Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>

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

#include "nepomukservice.h"
#include "resourcemanager.h"

#include <QtCore/QTimer>


class Nepomuk::Service::Private
{
public:
};


Nepomuk::Service::Service( QObject* parent, bool delayedInitialization )
    : QObject( parent ),
      d( new Private() )
{
    if ( !delayedInitialization ) {
        setServiceInitialized( true );
    }
}


Nepomuk::Service::~Service()
{
    delete d;
}


Soprano::Model* Nepomuk::Service::mainModel()
{
    return ResourceManager::instance()->mainModel();
}


void Nepomuk::Service::setServiceInitialized( bool success )
{
    // the parent will always be a control class in the service stub
    QMetaObject::invokeMethod( parent(),
                               "setServiceInitialized",
                               Qt::QueuedConnection,  // needs to be queued to give the service time to register with DBus
                               Q_ARG(bool, success) );
}

#include "nepomukservice.moc"
