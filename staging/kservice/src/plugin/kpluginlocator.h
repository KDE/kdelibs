/*
   Copyright 2013 Sebastian Kügler <sebas@kde.org>

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

#ifndef KPLUGINLOCATOR_H
#define KPLUGINLOCATOR_H

#include <kservice.h>
#include <kservice_export.h>

#include <QtCore/QLibrary>

class KPluginLocatorPrivate;

class KPluginFactory;

/**
 * \class KPluginLocator kpluginlocator.h <KPluginLocator>
 *

 */
class KSERVICE_EXPORT KPluginLocator
{
public:
    static QStringList locatePlugin(const QString &name);

    static KService::List query( const QString& servicetype,
                          const QString& constraint = QString() );
};

#endif
