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
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#ifndef KCMODULEINFO_H
#define KCMODULEINFO_H

#include <kservice.h>

class QPixmap;
class QString;
class QStringList;

/**
 * A class that provides information about a Control Module
 * 
 * @internal
 * @author Matthias Hoelzer-Kluepfel <mhk@kde.org> 
 * @author Matthias Elter <elter@kde.org>
 * @author Daniel Molkentin <molkentin@kde.org>
 * @since 3.2
 *
 */
class KCModuleInfo
{

public:

  /**
   * Constructs a KCModuleInfo, note that you will have to take care about the deletion
   * yourself!
   */
  KCModuleInfo(const QString& desktopFile, const QString& baseGroup /* = QString::fromLatin1("settings")*/);

  /**
   * Construct a KCModuleInfo from a KService object.
   */
  KCModuleInfo( KService::Ptr moduleInfo );

  KCModuleInfo( const KCModuleInfo &rhs );
  KCModuleInfo &operator=( const KCModuleInfo &rhs );
  ~KCModuleInfo();

  /**
   * @return the filename of the .desktop file that describes the KCM
   */
  QString fileName() const { return _fileName; };

  /**
   * @return the list of groups this KCM belongs to.
   */
  const QStringList &groups() const { return _groups; };

  /**
   * @return the keywords associated with this KCM.
   */
  const QStringList &keywords() const { return _keywords; };

  // changed from name() to avoid abiguity with QObject::name() on multiple inheritance
  /**
   * @return the module's (translated) name
   */
  QString moduleName() const { return _name; };

  /**
   * @return The module's (translated) path names. This gives you the path in a
   * TreeView in KDialogBase where the module belongs to.
   */
  QStringList moduleNames() const;

  /**
   * @return a @ref KSharedPtr to @ref KService created from the modules .desktop file
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
   * @return the KCD parent
   */
  const QStringList & parentComponents() const;

  /**
   * @return whether the module might require root permissions
   */
  bool needsRootPrivileges() const;

  /**
   * @deprecated
   * @return the isHiddenByDefault attribute.
   */
  bool isHiddenByDefault() const;

  QCString moduleId() const;

protected:

  void setGroups(const QStringList &groups) { _groups = groups; };
  void setKeywords(const QStringList &k) { _keywords = k; };
  void setName(const QString &name) { _name = name; };
  void setComment(const QString &comment) { _comment = comment; };
  void setIcon(const QString &icon) { _icon = icon; };
  void setLibrary(const QString &lib) { _lib = lib; };
  void setHandle(const QString &handle) { _handle = handle; };
  void setParentComponents(const QStringList &parentcomponents) { _parentcomponents = parentcomponents; }
  void setNeedsRootPrivileges(bool needsRootPrivileges) { _needsRootPrivileges = needsRootPrivileges; };
  void setIsHiddenByDefault(bool isHiddenByDefault) { _isHiddenByDefault = isHiddenByDefault; };
  void setDocPath(const QString &p) { _doc = p; };
  void loadAll();

private:

  // when adding members, don't forget to take care about them in the assignment operator
  QStringList _groups, _keywords, _parentcomponents;
  QString     _name, _icon, _lib, _handle, _fileName, _doc, _comment;
  bool        _needsRootPrivileges : 1; 
  bool        _isHiddenByDefault : 1;
  bool        _allLoaded : 1;

  KService::Ptr _service;

  class KCModuleInfoPrivate;
  KCModuleInfoPrivate *d;
  
};

#endif // KCMODULEINFO_H

// vim: ts=2 sw=2 et
