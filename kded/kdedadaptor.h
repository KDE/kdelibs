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

#ifndef KDED_KDEDADAPTOR_H
#define KDED_KDEDADAPTOR_H

#include <QtDBus/QtDBus>

class KdedAdaptor: public QDBusAbstractAdaptor
{
   Q_OBJECT
   Q_CLASSINFO("D-Bus Interface", "org.kde.kded")
public:
   KdedAdaptor(QObject *parent);

public Q_SLOTS:
   bool loadModule(const QString &obj);
   QStringList loadedModules();
   bool unloadModule(const QString &obj);
   //bool isWindowRegistered(qlonglong windowId) const;
   void registerWindowId(qlonglong windowId, const QDBusMessage&);
   void unregisterWindowId(qlonglong windowId, const QDBusMessage&);
   void reconfigure();
   void loadSecondPhase();
   void quit();

   /**
    * Check if module @a module has @c X-KDE-Kded-autoload=True.
    */
   bool isModuleAutoloaded(const QString &module);

   /**
    * Check if module @a module has @c X-KDE-Kded-load-on-demand=True.
    */
   bool isModuleLoadedOnDemand(const QString &module);

   /**
    * Set @c X-KDE-Kded-autoload to @a autoload for module @a module.
    */
   void setModuleAutoloading(const QString &module, bool autoload);
};

#endif
