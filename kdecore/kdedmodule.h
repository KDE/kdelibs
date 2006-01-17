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

#include <qobject.h>
#include <dcopobject.h>
//#include <ksharedptr.h>
#include <QByteArray>

class KDEDModulePrivate;
class Kded;

/**
 * The base class for KDED modules.
 *
 * KDED modules are realized as shared
 * libraries that are loaded on-demand into kded at runtime.
 *
 * To write a config module, you have to create a library
 * that contains at least one factory function like this:
 *
 * \code
 *   extern "C" {
 *     KDE_EXPORT KDEDModule *create_xyz(DCOPCString *name)
 *     {
 *       return new XYZ(name);
 *     }
 *   }
 * \endcode
 *
 * See kdelibs/kded/HOWTO for more detailed documentation.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */

class KDECORE_EXPORT KDEDModule : public QObject, public DCOPObject
{
  Q_OBJECT
  K_DCOP

  friend class Kded;
public:

  /**
   * Create a DCOPObject named @p name
   */
  KDEDModule(const DCOPCString &name);

  virtual ~KDEDModule();

#if 0 // does anyone really use this? I found no kdedmodule using it.
     // and since KShared is deprecated, this would have to be ported to something else.
     // For instance a KDEDObject base class (doing nothing, just an interface),
     // and saying that the kdedmodule gets ownership; no need for kshared[ptr] here, right?

     // Same for setIdleTimeout/resetIdle - no hit in lxr.kde.org
     // Grep for KDED_OBJECTS if you re-enable the feature.

  /**
   * Specifies the idle timeout in seconds. The default is 0.
   *
   * This will call the idle slot @p secs seconds after the last
   * reference was removed.
   */
  void setIdleTimeout(int secs);

  /**
   * Reset the idle timeout counter.
   *
   * (re)starts the timeout counter if no objects are being referenced.
   */
  void resetIdle();


  /**
   * Insert @p obj indexed with @p app and @p key. The
   * object will be automatically deleted when the application
   * @p app unregisters with DCOP.
   *
   * Any previous object inserted with the same values for @p app
   * and @p key will be removed.
   */
  void insert(const DCOPCString &app, const DCOPCString &key, KShared *obj);

  /**
   * Lookup object indexed with @p app and @p key
   */
  KShared *find(const DCOPCString &app, const DCOPCString &key);

  /**
   * remove object indexed with @p app and @p key.
   * The object will be deleted when it is no more referenced.
   */
  void remove(const DCOPCString &app, const DCOPCString &key);

  /**
   * remove all objects indexed with @p app.
   * The objects will be deleted when they are no more referenced.
   */
  void removeAll(const DCOPCString &app);
#endif

#if 0 // does anyone use this?
  /**
   * Returns whether a certain mainwindow has registered itself with KDED
   */
  bool isWindowRegistered(long windowId) const;
#endif

#if 0
public Q_SLOTS:
  /**
   * Called whenever the last referenced object gets dereferenced.
   *
   * See also setIdleTimeout()
   *
   * You may delete the module from this slot.
   */
  virtual void idle() { }
#endif

Q_SIGNALS:
  /**
   * Emitted when the module is being deleted.
   */
  void moduleDeleted(KDEDModule *);

  /**
   * Emitted when a mainwindow registers itself.
   */
  void windowRegistered(long windowId);

  /**
   * Emitted when a mainwindow unregisters itself.
   */
  void windowUnregistered(long windowId);

private:
  KDEDModulePrivate* const d;
};

#endif
