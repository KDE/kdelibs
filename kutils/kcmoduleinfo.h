/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
  Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>
  Copyright (c) 2003 Matthias Kretz <kretz@kde.org>

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

#include <kservice.h>

class QPixmap;
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
 * @since 3.2
 *
 */
class KUTILS_EXPORT KCModuleInfo
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
   * @since 3.4
   */
  KCModuleInfo();

  /**
   * Assignment operator
   */
  KCModuleInfo &operator=( const KCModuleInfo &rhs );

  /**
   * Equal operator
   *
   * @return true if @p rhs equals itself
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
  QString fileName() const { return _fileName; };

  /**
   * @return the keywords associated with this KCM.
   */
  const QStringList &keywords() const { return _keywords; };

  /**
   * Returns the module's factory name, if it's set. If not, the library
   * name is returned.
   * @returns the module's factory name
   * @since 3.4
   */
  QString factoryName() const;

  /**
   * @return the module\'s (translated) name
   */
  QString moduleName() const { return _name; };
  // changed from name() to avoid ambiguity with QObject::name() on multiple inheritance

  /**
   * @return a KSharedPtr to KService created from the modules .desktop file
   */
  KService::Ptr service() const { return _service; };

  /**
   * @return the module's (translated) comment field
   */
  QString comment() const { return _comment; };

  /**
   * @return the module's icon name
   */
  QString icon() const { return _icon; };

  /**
   * @return the path of the module's documentation
   */
  QString docPath() const;

  /**
   * @return the library name
   */
  QString library() const { return _lib; };

  /**
   * @return a handle (usually the contents of the FactoryName field)
   */
  QString handle() const;

  /**
   * @return the weight of the module which determines the order of the pages in
   * the KCMultiDialog. It's set by the X-KDE-Weight field.
   */
  int weight() const;

  /**
   * @return whether the module might require root permissions
   */
  bool needsRootPrivileges() const;

  /**
   * @deprecated
   * @return the isHiddenByDefault attribute.
   */
  bool isHiddenByDefault() const KDE_DEPRECATED;


  /**
   * @returns true if the module should be conditionally
   * loaded.
   * @since 3.4
   */
  bool needsTest() const;


protected:

  /**
   * Sets the object's keywords.
   * @param keyword the new keywords
   */
  void setKeywords(const QStringList &keyword) { _keywords = keyword; };

  /**
   * Sets the object's name.
   * @param name the new name
   */
  void setName(const QString &name) { _name = name; };

  /**
   * Sets the object's name.
   * @param comment the new comment
   */
  void setComment(const QString &comment) { _comment = comment; };

  /**
   * Sets the object's icon.
   * @param icon the name of the new icon
   */
  void setIcon(const QString &icon) { _icon = icon; };

  /**
   * Set the object's library
   * @param lib the name of the new library without any extensions or prefixs.
   */
  void setLibrary(const QString &lib) { _lib = lib; };

  /**
   * Sets the factory name
   * @param handle The new factory name
   */
  void setHandle(const QString &handle) { _handle = handle; };

  /**
   * Sets the object's weight property which determines in what
   * order modules will be displayed. Default is 100.
   *
   * @param weight the new weight
   */
  void setWeight(int weight) { _weight = weight; };


  /**
   * Sets if the module should be tested for loading.
   * @param val the value to set
   * @since 3.4
   */
  void setNeedsTest( bool val );

  /**
   * Toggles whether the represented module needs root privileges.
   * Use with caution.
   * @param needsRootPrivileges if module needs root privilges
   */
  void setNeedsRootPrivileges(bool needsRootPrivileges)
  { _needsRootPrivileges = needsRootPrivileges; };

  /**
   * @deprecated
   */
  void setIsHiddenByDefault(bool isHiddenByDefault)
  { _isHiddenByDefault = isHiddenByDefault; };

  /**
   * Sets the object's documentation path
   * @param p the new documentation path
   */
  void setDocPath(const QString &p) { _doc = p; };

  /**
   * Reads the service entries specific for KCModule from the desktop file.
   * The usual desktop entries are read in init.
   */
  void loadAll();

private:

  /**
   * Reads the service entries. Called by the constructors.
   */
  void init(KService::Ptr s);

private:

  // KDE4 These needs to be moved to KCModuleInfoPrivate
  QStringList _keywords;
  QString     _name, _icon, _lib, _handle, _fileName, _doc, _comment;
  bool        _needsRootPrivileges : 1;
  bool        _isHiddenByDefault : 1;
  bool        _allLoaded : 1;
  int         _weight;

  KService::Ptr _service;

  class KCModuleInfoPrivate;
  KCModuleInfoPrivate *d;

};

#endif // KCMODULEINFO_H

// vim: ts=2 sw=2 et
