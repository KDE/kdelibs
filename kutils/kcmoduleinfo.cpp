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


#include "kcmoduleinfo.h"

#include <kdesktopfile.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>

KCModuleInfo::KCModuleInfo(const QString& desktopFile, const QString& baseGroup)
  : _fileName(desktopFile), d(0L)
{
  _allLoaded = false;

  //kdDebug(1208) << "desktopFile = " << desktopFile << endl;
  _service = KService::serviceByDesktopPath(desktopFile);
  Q_ASSERT(_service != 0L);

  // set the modules simple attributes
  setName(_service->name());
  setComment(_service->comment());
  setIcon(_service->icon());

  // library and factory
  setLibrary(_service->library());

  // get the keyword list
  setKeywords(_service->keywords());

  // try to find out the modules groups
  QString group = desktopFile;

  int pos = group.find(baseGroup);
  if (pos >= 0)
     group = group.mid(pos+baseGroup.length());
  pos = group.findRev('/');
  if (pos >= 0)
     group = group.left(pos);
  else
     group = QString::null;

  QStringList groups = QStringList::split('/', group);
  setGroups(groups);
}

KCModuleInfo::KCModuleInfo( KService::Ptr moduleInfo )
  : _fileName( KGlobal::dirs()->findResource( "services", moduleInfo->desktopEntryPath() ) )
{
  kdDebug() << k_funcinfo << _fileName << endl;
  _allLoaded = false;

  _service = moduleInfo;
  Q_ASSERT(_service != 0L);

  // set the modules simple attributes
  setName(_service->name());
  setComment(_service->comment());
  setIcon(_service->icon());

  // library and factory
  setLibrary(_service->library());

  // get the keyword list
  setKeywords(_service->keywords());

  // get the groups list
  KDesktopFile desktop(_fileName);
  setGroups( desktop.readListEntry( "X-KDE-Groups" ) );
}

KCModuleInfo::KCModuleInfo( const KCModuleInfo &rhs )
    : d( 0 )
{
    ( *this ) = rhs;
}

// this re-implementation exists to ensure that other code always calls
// our re-implementation, so in case we add data to the d pointer in the future 
// we can be sure that we get called when we are copied.
KCModuleInfo &KCModuleInfo::operator=( const KCModuleInfo &rhs )
{
    _groups = rhs._groups;
    _keywords = rhs._keywords;
    _name = rhs._name;
    _icon = rhs._icon;
    _lib = rhs._lib;
    _handle = rhs._handle;
    _fileName = rhs._fileName;
    _doc = rhs._doc;
    _comment = rhs._comment;
    _parentcomponents = rhs._parentcomponents;
    _needsRootPrivileges = rhs._needsRootPrivileges;
    _isHiddenByDefault = rhs._isHiddenByDefault;
    _allLoaded = rhs._allLoaded;
    _service = rhs._service;

    // d pointer ... once used.

    return *this;
}

bool KCModuleInfo::operator==( const KCModuleInfo & rhs ) const
{
  return ( ( _name == rhs._name ) && ( _lib == rhs._lib ) && ( _fileName == rhs._fileName ) );
}

bool KCModuleInfo::operator!=( const KCModuleInfo & rhs ) const
{
  return ! operator==( rhs );
}

KCModuleInfo::~KCModuleInfo() { }

void
KCModuleInfo::loadAll() 
{
  _allLoaded = true;

  KDesktopFile desktop(_fileName);

  // library and factory
  setHandle(desktop.readEntry("X-KDE-FactoryName"));

  // KCD parent
  setParentComponents(desktop.readListEntry("X-KDE-ParentComponents"));

  // does the module need super user privileges?
  setNeedsRootPrivileges(desktop.readBoolEntry("X-KDE-RootOnly", false));

  // does the module need to be shown to root only?
  // Deprecated !
  setIsHiddenByDefault(desktop.readBoolEntry("X-KDE-IsHiddenByDefault", false));

  // get the documentation path
  setDocPath(desktop.readPathEntry("DocPath"));
}

QStringList
KCModuleInfo::moduleNames() const
{
  QStringList sl = _groups;
  sl += _name;
  return sl;
}

QCString 
KCModuleInfo::moduleId() const
{
  if (!_allLoaded) const_cast<KCModuleInfo*>(this)->loadAll();

  QString res;

  QStringList::ConstIterator it;
  for (it = _groups.begin(); it != _groups.end(); ++it)
    res.append(*it+"-");
  res.append(moduleName());

  return res.ascii();
}

QString
KCModuleInfo::docPath() const
{
  if (!_allLoaded) 
    const_cast<KCModuleInfo*>(this)->loadAll();

  return _doc;
}

QString
KCModuleInfo::handle() const
{
  if (!_allLoaded) 
    const_cast<KCModuleInfo*>(this)->loadAll();

  if (_handle.isEmpty())
     return _lib;

  return _handle;
}

const QStringList &
KCModuleInfo::parentComponents() const
{
  if( !_allLoaded )
    const_cast<KCModuleInfo*>( this )->loadAll();

  return _parentcomponents;
}

bool
KCModuleInfo::needsRootPrivileges() const
{
  if (!_allLoaded) 
    const_cast<KCModuleInfo*>(this)->loadAll();

  return _needsRootPrivileges;
}

bool
KCModuleInfo::isHiddenByDefault() const
{
  if (!_allLoaded)
    const_cast<KCModuleInfo*>(this)->loadAll();

  return _isHiddenByDefault;
}

// vim: ts=2 sw=2 et
