/*
   This file is part of the KDE libraries

   Copyright (c) 2001 Waldo Bastian <bastian@kde.org>

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

#include "kdedmodule.h"

#include "kdebug.h"
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>



class KDEDModulePrivate
{
public:
  QString moduleName;
};

KDEDModule::KDEDModule(QObject* parent)
    : QObject(parent), d(new KDEDModulePrivate)
{
}

KDEDModule::~KDEDModule()
{
   emit moduleDeleted(this);
   delete d;
}

void KDEDModule::setModuleName( const QString& name )
{
   QString realPath = d->moduleName = name;
   realPath.prepend("/modules/");
   // ExportSignals not used since it triggers a warning at this point
   if (!QDBusConnection::sessionBus().registerObject(
           realPath,
           this,
           QDBusConnection::ExportScriptableContents | QDBusConnection::ExportAdaptors))
        {
        // Happens for khotkeys but the module works. Need some time to
        // investigate.
        kDebug() << "registerObject() returned false for %s" << d->moduleName;
        }

}

QString KDEDModule::moduleName() const
{
   return d->moduleName;
}

#include "kdedmodule.moc"
