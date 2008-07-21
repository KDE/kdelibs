/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
 *  Copyright (C) 2000 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kdedadaptor.h"
#include "kded.h"
#include <kglobal.h>
#include <kconfig.h>
#include <QCoreApplication>

KdedAdaptor::KdedAdaptor(QObject *parent)
   : QDBusAbstractAdaptor(parent)
{
}

bool KdedAdaptor::isModuleAutoloaded(const QString &module)
{
   return Kded::self()->isModuleAutoloaded(module);
}

bool KdedAdaptor::isModuleLoadedOnDemand(const QString &module)
{
   return Kded::self()->isModuleLoadedOnDemand(module);
}

bool KdedAdaptor::loadModule(const QString &module)
{
   return Kded::self()->loadModule(module, false) != 0;
}

bool KdedAdaptor::unloadModule(const QString &module)
{
   return Kded::self()->unloadModule(module);
}

void KdedAdaptor::registerWindowId(qlonglong windowId, const QDBusMessage &msg)
{
   Kded::self()->registerWindowId(windowId, msg.service());
}

void KdedAdaptor::setModuleAutoloading(const QString &module, bool autoload)
{
        return Kded::self()->setModuleAutoloading(module, autoload);
}
void KdedAdaptor::unregisterWindowId(qlonglong windowId, const QDBusMessage &msg)
{
   Kded::self()->unregisterWindowId(windowId, msg.service());
}

QStringList KdedAdaptor::loadedModules()
{
   return Kded::self()->loadedModules();
}

void KdedAdaptor::reconfigure()
{
   KGlobal::config()->reparseConfiguration();
   Kded::self()->initModules();
   Kded::self()->loadSecondPhase();
}

void KdedAdaptor::loadSecondPhase()
{
   Kded::self()->loadSecondPhase();
}

void KdedAdaptor::quit()
{
   QCoreApplication::instance()->quit();
}

#include "kdedadaptor.moc"
