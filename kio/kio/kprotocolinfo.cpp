/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kprotocolinfo.h"
#include "kprotocolinfofactory.h"
#include "kprotocolmanager.h"

#ifdef Q_CC_MSVC
#include "../../kdecore/kprotocolinfo_kdecore.cpp"
#endif

// Most of this class is implemented in kdecore/kprotocolinfo_kdecore.cpp
// This file only contains a few static class-functions that depend on
// KProtocolManager
KProtocolInfo::Ptr KProtocolInfo::findProtocol(const KURL &url)
{
   QString protocol = url.protocol();

   if ( !KProtocolInfo::proxiedBy( protocol ).isEmpty() )
   {
      QString dummy;
      protocol = KProtocolManager::slaveProtocol(url, dummy);
   }

   return KProtocolInfoFactory::self()->findProtocol(protocol);
}


KProtocolInfo::Type KProtocolInfo::inputType( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return T_NONE;

  return prot->m_inputType;
}

KProtocolInfo::Type KProtocolInfo::outputType( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return T_NONE;

  return prot->m_outputType;
}


bool KProtocolInfo::isSourceProtocol( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_isSourceProtocol;
}

bool KProtocolInfo::isFilterProtocol( const KURL &url )
{
  return isFilterProtocol (url.protocol());
}

bool KProtocolInfo::isFilterProtocol( const QString &protocol )
{
  // We call the findProtocol (const QString&) to bypass any proxy settings.
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(protocol);
  if ( !prot )
    return false;

  return !prot->m_isSourceProtocol;
}

bool KProtocolInfo::isHelperProtocol( const KURL &url )
{
  return isHelperProtocol (url.protocol());
}

bool KProtocolInfo::isHelperProtocol( const QString &protocol )
{
  // We call the findProtocol (const QString&) to bypass any proxy settings.
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(protocol);
  if ( !prot )
    return false;

  return prot->m_isHelperProtocol;
}

bool KProtocolInfo::isKnownProtocol( const KURL &url )
{
  return isKnownProtocol (url.protocol());
}

bool KProtocolInfo::isKnownProtocol( const QString &protocol )
{
  // We call the findProtocol (const QString&) to bypass any proxy settings.
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(protocol);
  return prot;
}

bool KProtocolInfo::supportsListing( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsListing;
}

QStringList KProtocolInfo::listing( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return QStringList();

  return prot->m_listing;
}

bool KProtocolInfo::supportsReading( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsReading;
}

bool KProtocolInfo::supportsWriting( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsWriting;
}

bool KProtocolInfo::supportsMakeDir( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsMakeDir;
}

bool KProtocolInfo::supportsDeleting( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsDeleting;
}

bool KProtocolInfo::supportsLinking( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsLinking;
}

bool KProtocolInfo::supportsMoving( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsMoving;
}

bool KProtocolInfo::canCopyFromFile( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_canCopyFromFile;
}


bool KProtocolInfo::canCopyToFile( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_canCopyToFile;
}

bool KProtocolInfo::canRenameFromFile( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->canRenameFromFile();
}


bool KProtocolInfo::canRenameToFile( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->canRenameToFile();
}

bool KProtocolInfo::canDeleteRecursive( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->canDeleteRecursive();
}

KProtocolInfo::FileNameUsedForCopying KProtocolInfo::fileNameUsedForCopying( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return FromURL;

  return prot->fileNameUsedForCopying();
}

QString KProtocolInfo::defaultMimetype( const KURL &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return QString();

  return prot->m_defaultMimetype;
}

