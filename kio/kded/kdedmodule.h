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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/
#ifndef __KDEDMODULE_H__
#define __KDEDMODULE_H__

#include <qobject.h>
#include <dcopobject.h>
#include <ksharedptr.h>

class KDEDModulePrivate;

/**
 * The base class for KDED modules.
 *
 * In KDE 2.x, KDED modules are realized as shared
 * libraries that are loaded on-demand into kded at runtime.
 *
 * To write a config module, you have to create a library
 * that contains at least one factory function like this:
 *
 * <pre>
 *   extern "C" {
 *     KDEDModule *create_xyz(QCString *name)
 *     {
 *       return new XYZ(name);
 *     }
 *   }
 * </pre>
 *
 * See kkdelibs/kio/kded/HOWTO for more detailed documentation.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */

class KDEDModule : public QObject, public DCOPObject
{
  Q_OBJECT

public:
  
  /**
   * Create a DCOPObject named @p name
   */
  KDEDModule(const QCString &name);
  
  virtual ~KDEDModule();
  
  /**
   * Called whenever the last referenced object gets dereferenced.
   */
  virtual void idle() { };

  /**
   * Insert @p obj indexed with @p app and @p key. The
   * object will be automatically deleted when the application
   * @p app unregisters with DCOP.
   *
   * Any previous object inserted with the same values for @p app 
   * and @p key will be removed.
   */
  void insert(const QCString &app, const QCString &key, KShared *obj);

  /**
   * Lookup object indexed with @p app and @p key
   */
  KShared *find(const QCString &app, const QCString &key);
  
  /**
   * remove object indexed with @p app and @p key.
   * The object will be deleted when it is no more referenced.
   */
  void remove(const QCString &app, const QCString &key);

private:
  KDEDModulePrivate *d;
};

#endif
