/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
  Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>
  Copyright (c) 2003,2006 Matthias Kretz <kretz@kde.org>

  This file is part of the KDE project

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2, as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KCMODULEINFO_H
#define KCMODULEINFO_H

#include <kutils_export.h>
#include <kservice.h>

class QString;
class QStringList;

/**
 * A class that provides information about a KCModule
 *
 * KCModuleInfo provides various technical information, such as icon, library
 * etc. about a KCModule.n
 * @note Any values set with the set* functions is not
 * written back with KCModuleInfo it only reads value from the desktop file.
 *
 * @internal
 * @author Matthias Hoelzer-Kluepfel <mhk@kde.org>
 * @author Matthias Elter <elter@kde.org>
 * @author Daniel Molkentin <molkentin@kde.org>
 *
 */
class KUTILS_EXPORT KCModuleInfo // krazy:exclude=dpointer (implicitly shared)
{

public:

  /**
   * Constructs a KCModuleInfo.
   * @note a KCModuleInfo object will have to be manually deleted, it is not
   * done automatically for you.
   * @param desktopFile the desktop file representing the module, or
   * the name of the module.
   */
  KCModuleInfo(const QString& desktopFile);

  /**
   * Same as above but takes a KService::Ptr as argument.
   *
   * @note @p moduleInfo must be a valid pointer.
   *
   * @param moduleInfo specifies the module
   */
  KCModuleInfo( KService::Ptr moduleInfo );


  /**
   * Same as above but takes a KCModuleInfo as argument.
   *
   * @param rhs specifies the module
   */
  KCModuleInfo( const KCModuleInfo &rhs );

  /**
   * Same as above but creates an empty KCModuleInfo.
   * You should not normally call this.
   */
  KCModuleInfo();

  /**
   * Assignment operator
   */
  KCModuleInfo &operator=( const KCModuleInfo &rhs );

    /**
     * Returns true if @p rhs describes the same KCModule as this object.
     */
  bool operator==( const KCModuleInfo &rhs ) const;

  /**
   * @return true if @p rhs is not equal itself
   */
  bool operator!=( const KCModuleInfo &rhs ) const;

  /**
   * Default destructor.
   */
  ~KCModuleInfo();

  /**
   * @return the filename of the .desktop file that describes the KCM
   */
  QString fileName() const;

  /**
   * @return the keywords associated with this KCM.
   */
  QStringList keywords() const;

  /**
   * @return the module\'s (translated) name
   */
  QString moduleName() const;

  /**
   * @return a KSharedPtr to KService created from the modules .desktop file
   */
  KService::Ptr service() const;

  /**
   * @return the module's (translated) comment field
   */
  QString comment() const;

  /**
   * @return the module's icon name
   */
  QString icon() const;

  /**
   * @return the path of the module's documentation
   */
  QString docPath() const;

  /**
   * @return the library name
   */
  QString library() const;

  /**
   * @return a handle (the contents of the X-KDE-FactoryName field if it exists,
   * else the same as the library name)
   */
  QString handle() const;

  /**
   * @return the weight of the module which determines the order of the pages in
   * the KCMultiDialog. It's set by the X-KDE-Weight field.
   */
  int weight() const;

private:
  class Private;
  Private * d;
};

#endif // KCMODULEINFO_H

// vim: ts=2 sw=2 et
