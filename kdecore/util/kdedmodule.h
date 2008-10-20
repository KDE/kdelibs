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
#ifndef __KDEDMODULE_H__
#define __KDEDMODULE_H__

#include <kdecore_export.h>

#include <QtCore/QObject>
#include <QtCore/QByteArray>

class KDEDModulePrivate;
class Kded;

class QDBusObjectPath;

/**
 * \class KDEDModule kdedmodule.h <KDEDModule>
 * 
 * The base class for KDED modules.
 *
 * KDED modules are realized as shared
 * libraries that are loaded on-demand into kded at runtime.
 *
 * See kdelibs/kded/HOWTO for documentation about writing kded modules.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
class KDECORE_EXPORT KDEDModule: public QObject
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.KDEDModule")

  friend class Kded;
public:

  /**
   * Constructor
   */
  explicit KDEDModule(QObject* parent = 0);

  virtual ~KDEDModule();

  /**
   * @internal called by kded after loading a module
   * The module name is set from the path of the desktop file, and is
   * used to register the module to dbus.
   */
  void setModuleName( const QString& name );

  QString moduleName() const;

Q_SIGNALS:
  /**
   * Emitted when the module is being deleted.
   */
  void moduleDeleted(KDEDModule *);

  /**
   * Emitted when a mainwindow registers itself.
   */
  void windowRegistered(qlonglong windowId);

  /**
   * Emitted when a mainwindow unregisters itself.
   */
  void windowUnregistered(qlonglong windowId);

  /**
   * Emitted after the module is registered successfully with dbus
   *
   * @since 4.2
   */
  void moduleRegistered(const QDBusObjectPath &path);

private:
  KDEDModulePrivate* const d;
};

#endif
