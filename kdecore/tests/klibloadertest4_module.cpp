/* This file is part of the KDE libraries
    Copyright (c) 2005 David Faure <faure@kde.org>

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

#include <kpluginfactory.h> // K_PLUGIN_FACTORY
#include "klibloadertest4_module.h"

K_PLUGIN_FACTORY(KLibLoaderTestFactory,
                 registerPlugin<KLibLoaderTestObject>();
                 )
K_EXPORT_PLUGIN(KLibLoaderTestFactory("klibloader4testfactory"))

KLibLoaderTestObject::KLibLoaderTestObject( QObject* parent, const QList<QVariant>& )
    : QObject( parent )
{
    qDebug( "KLibLoaderTestObject created with KPluginFactory" );
}

KLibLoaderTestObject::~KLibLoaderTestObject()
{
    qDebug( "KLibLoaderTestObject created with KPluginFactory, now deleted" );
}
