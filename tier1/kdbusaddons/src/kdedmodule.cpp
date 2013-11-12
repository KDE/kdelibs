//! vim: ts=3 sw=3
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

#include <QDebug>
#include <QTimer>
#include <QDBusConnection>
#include <QDBusObjectPath>

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
   d->moduleName = name;
   QDBusObjectPath realPath( QString::fromLatin1("/modules/") + d->moduleName);

   if (realPath.path().isEmpty())
      {
      qWarning() << "The kded module name" << name << "is invalid!";
      return;
      }


   QDBusConnection::RegisterOptions regOptions;

   if (this->metaObject()->indexOfClassInfo("D-Bus Interface")!=-1)
      {
      // 1. There are kded modules that don't have a D-Bus interface.
      // 2. qt 4.4.3 crashes when trying to emit signals on class without
      //    Q_CLASSINFO("D-Bus Interface", "<your interface>") but
      //    ExportSignal set.
      // We try to solve that for now with just registering Properties and
      // Adaptors. But we should investigate where the sense is in registering
      // the module at all. Just for autoload? Is there a better solution?
      regOptions = QDBusConnection::ExportScriptableContents | QDBusConnection::ExportAdaptors;
      }
   else
      {
      // Full functional module. Register everything.
      regOptions = QDBusConnection::ExportScriptableSlots
                     | QDBusConnection::ExportScriptableProperties
                     | QDBusConnection::ExportAdaptors;
      qDebug() << "Registration of kded module" << d->moduleName << "without D-Bus interface.";
      }

   if (!QDBusConnection::sessionBus().registerObject(realPath.path(), this, regOptions))
      {
      // Happens for khotkeys but the module works. Need some time to investigate.
      qDebug() << "registerObject() returned false for" << d->moduleName;
      }
   else
      {
      //qDebug() << "registerObject() successful for" << d->moduleName;
      emit moduleRegistered(realPath);
      }

}

QString KDEDModule::moduleName() const
{
   return d->moduleName;
}

