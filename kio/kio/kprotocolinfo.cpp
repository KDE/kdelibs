/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kprotocolinfo.h"
#include "kprotocolinfofactory.h"

#include <kstandarddirs.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kdebug.h>
#include <ksimpleconfig.h>
#include <kconfig.h>
#include <kstringhandler.h>

#include <stdio.h> // WAAB remove me

//
// Internal functions:
//
KProtocolInfo::KProtocolInfo(const QString &path)
 : KSycocaEntry(path)
{
  QString fullPath = locate("services", path);

  KSimpleConfig config( fullPath, true );
  config.setGroup( "Protocol" );

  m_name = config.readEntry( "protocol" );
  m_exec = config.readEntry( "exec" );
  m_isSourceProtocol = config.readBoolEntry( "source", true );
  m_isHelperProtocol = config.readBoolEntry( "helper", false );
  m_supportsReading = config.readBoolEntry( "reading", false );
  m_supportsWriting = config.readBoolEntry( "writing", false );
  m_supportsMakeDir = config.readBoolEntry( "makedir", false );
  m_supportsDeleting = config.readBoolEntry( "deleting", false );
  m_supportsLinking = config.readBoolEntry( "linking", false );
  m_supportsMoving = config.readBoolEntry( "moving", false );
  m_canCopyFromFile = config.readBoolEntry( "copyFromFile", false );
  m_canCopyToFile = config.readBoolEntry( "copyToFile", false );
  m_listing = config.readListEntry( "listing" );
  // Many .protocol files say "Listing=false" when they really mean "Listing=" (i.e. unsupported)
  if ( m_listing.count() == 1 && m_listing.first() == "false" )
    m_listing.clear();
  m_supportsListing = ( m_listing.count() > 0 );
  m_defaultMimetype = config.readEntry( "defaultMimetype" );
  m_determineMimetypeFromExtension = config.readBoolEntry( "determineMimetypeFromExtension", true );
  m_icon = config.readEntry( "Icon", "mime_empty" );
  m_config = config.readEntry( "config", m_name );
  m_maxSlaves = config.readNumEntry( "maxInstances", 1);

  QString tmp = config.readEntry( "input" );
  if ( tmp == "filesystem" )
    m_inputType = KProtocolInfo::T_FILESYSTEM;
  else if ( tmp == "stream" )
    m_inputType = KProtocolInfo::T_STREAM;
  else
    m_inputType = KProtocolInfo::T_NONE;

  tmp = config.readEntry( "output" );
  if ( tmp == "filesystem" )
    m_outputType = KProtocolInfo::T_FILESYSTEM;
  else if ( tmp == "stream" )
    m_outputType = KProtocolInfo::T_STREAM;
  else
    m_outputType = KProtocolInfo::T_NONE;
}

KProtocolInfo::KProtocolInfo( QDataStream& _str, int offset) :
	KSycocaEntry( _str, offset)
{
   load( _str );
}

KProtocolInfo::~KProtocolInfo()
{
}

void
KProtocolInfo::load( QDataStream& _str)
{
   Q_INT32 i_inputType, i_outputType;
   Q_INT8 i_isSourceProtocol, i_isHelperProtocol,
          i_supportsListing, i_supportsReading,
          i_supportsWriting, i_supportsMakeDir,
          i_supportsDeleting, i_supportsLinking,
          i_supportsMoving, i_determineMimetypeFromExtension,
          i_canCopyFromFile, i_canCopyToFile;
   _str >> m_name >> m_exec >> m_listing >> m_defaultMimetype
        >> i_determineMimetypeFromExtension 
        >> m_icon
        >> i_inputType >> i_outputType
        >> i_isSourceProtocol >> i_isHelperProtocol
        >> i_supportsListing >> i_supportsReading
        >> i_supportsWriting >> i_supportsMakeDir
        >> i_supportsDeleting >> i_supportsLinking
        >> i_supportsMoving 
        >> i_canCopyFromFile >> i_canCopyToFile
        >> m_config >> m_maxSlaves;
   m_inputType = (Type) i_inputType;
   m_outputType = (Type) i_outputType;
   m_isSourceProtocol = (i_isSourceProtocol != 0);
   m_isHelperProtocol = (i_isHelperProtocol != 0);
   m_supportsListing = (i_supportsListing != 0);
   m_supportsReading = (i_supportsReading != 0);
   m_supportsWriting = (i_supportsWriting != 0);
   m_supportsMakeDir = (i_supportsMakeDir != 0);
   m_supportsDeleting = (i_supportsDeleting != 0);
   m_supportsLinking = (i_supportsLinking != 0);
   m_supportsMoving = (i_supportsMoving != 0);
   m_canCopyFromFile = (i_canCopyFromFile != 0);
   m_canCopyToFile = (i_canCopyToFile != 0);
   m_determineMimetypeFromExtension = (i_determineMimetypeFromExtension != 0);
}

void
KProtocolInfo::save( QDataStream& _str)
{
   KSycocaEntry::save( _str );

   Q_INT32 i_inputType, i_outputType;
   Q_INT8 i_isSourceProtocol, i_isHelperProtocol,
          i_supportsListing, i_supportsReading,
          i_supportsWriting, i_supportsMakeDir,
          i_supportsDeleting, i_supportsLinking,
          i_supportsMoving, i_determineMimetypeFromExtension,
          i_canCopyFromFile, i_canCopyToFile;

   i_inputType = (Q_INT32) m_inputType;
   i_outputType = (Q_INT32) m_outputType;
   i_isSourceProtocol = m_isSourceProtocol ? 1 : 0;
   i_isHelperProtocol = m_isHelperProtocol ? 1 : 0;
   i_supportsListing = m_supportsListing ? 1 : 0;
   i_supportsReading = m_supportsReading ? 1 : 0;
   i_supportsWriting = m_supportsWriting ? 1 : 0;
   i_supportsMakeDir = m_supportsMakeDir ? 1 : 0;
   i_supportsDeleting = m_supportsDeleting ? 1 : 0;
   i_supportsLinking = m_supportsLinking ? 1 : 0;
   i_supportsMoving = m_supportsMoving ? 1 : 0;
   i_canCopyFromFile = m_canCopyFromFile ? 1 : 0;
   i_canCopyToFile = m_canCopyToFile ? 1 : 0;
   i_determineMimetypeFromExtension = m_determineMimetypeFromExtension ? 1 : 0;


   _str << m_name << m_exec << m_listing << m_defaultMimetype
        << i_determineMimetypeFromExtension 
        << m_icon
        << i_inputType << i_outputType
        << i_isSourceProtocol << i_isHelperProtocol
        << i_supportsListing << i_supportsReading
        << i_supportsWriting << i_supportsMakeDir
        << i_supportsDeleting << i_supportsLinking
        << i_supportsMoving
        << i_canCopyFromFile << i_canCopyToFile
        << m_config << m_maxSlaves;
}


//
// Static functions:
//

QStringList KProtocolInfo::protocols()
{
  return KProtocolInfoFactory::self()->protocols();
}

bool KProtocolInfo::isSourceProtocol( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return false;

  return prot->m_isSourceProtocol;
}

bool KProtocolInfo::isFilterProtocol( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return false;

  return !prot->m_isSourceProtocol;
}

bool KProtocolInfo::isHelperProtocol( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return false;

  return prot->m_isHelperProtocol;
}

bool KProtocolInfo::isKnownProtocol( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  return ( prot != 0);
}

bool KProtocolInfo::supportsListing( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return false;

  return prot->m_supportsListing;
}

QStringList KProtocolInfo::listing( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return QStringList();

  return prot->m_listing;
}

bool KProtocolInfo::supportsReading( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return false;

  return prot->m_supportsReading;
}

bool KProtocolInfo::supportsWriting( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return false;

  return prot->m_supportsWriting;
}

bool KProtocolInfo::supportsMakeDir( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return false;

  return prot->m_supportsMakeDir;
}

bool KProtocolInfo::supportsDeleting( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return false;

  return prot->m_supportsDeleting;
}

bool KProtocolInfo::supportsLinking( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return false;

  return prot->m_supportsLinking;
}

bool KProtocolInfo::supportsMoving( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return false;

  return prot->m_supportsMoving;
}

bool KProtocolInfo::canCopyFromFile( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return false;

  return prot->m_canCopyFromFile;
}


bool KProtocolInfo::canCopyToFile( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return false;

  return prot->m_canCopyToFile;
}

QString KProtocolInfo::icon( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return QString::fromLatin1("mime_empty");

  return prot->m_icon;
}

QString KProtocolInfo::config( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return QString::null;

  return QString("kio_%1rc").arg(prot->m_config);
}

int KProtocolInfo::maxSlaves( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return 1;

  return prot->m_maxSlaves;
}

QString KProtocolInfo::defaultMimetype( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return QString::null;

  return prot->m_defaultMimetype;
}

bool KProtocolInfo::determineMimetypeFromExtension( const QString &_protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol( _protocol );
  if ( !prot )
    return true;

  return prot->m_determineMimetypeFromExtension;
}

QString KProtocolInfo::exec( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return QString::null;

  return prot->m_exec;
}

KProtocolInfo::Type KProtocolInfo::inputType( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return T_NONE;

  return prot->m_inputType;
}

KProtocolInfo::Type KProtocolInfo::outputType( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return T_NONE;

  return prot->m_outputType;
}


KProtocolInfo::Type KProtocolInfo::inputType( const KURL &url )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url);
  if ( !prot )
    return T_NONE;

  return prot->m_inputType;
}

KProtocolInfo::Type KProtocolInfo::outputType( const KURL &url )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url);
  if ( !prot )
    return T_NONE;

  return prot->m_outputType;
}


bool KProtocolInfo::isSourceProtocol( const KURL &url )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_isSourceProtocol;
}

bool KProtocolInfo::isFilterProtocol( const KURL &url )
{
  // We use url.protocol() to bypass any proxy settings.
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url.protocol());
  if ( !prot )
    return false;

  return !prot->m_isSourceProtocol;
}

bool KProtocolInfo::isHelperProtocol( const KURL &url )
{
  // We use url.protocol() to bypass any proxy settings.
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url.protocol());
  if ( !prot )
    return false;

  return prot->m_isHelperProtocol;
}

bool KProtocolInfo::isKnownProtocol( const KURL &url )
{
  // We use url.protocol() to bypass any proxy settings.
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url.protocol());
  return ( prot != 0);
}

bool KProtocolInfo::supportsListing( const KURL &url )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsListing;
}

QStringList KProtocolInfo::listing( const KURL &url )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url);
  if ( !prot )
    return QStringList();

  return prot->m_listing;
}

bool KProtocolInfo::supportsReading( const KURL &url )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsReading;
}

bool KProtocolInfo::supportsWriting( const KURL &url )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsWriting;
}

bool KProtocolInfo::supportsMakeDir( const KURL &url )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsMakeDir;
}

bool KProtocolInfo::supportsDeleting( const KURL &url )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsDeleting;
}

bool KProtocolInfo::supportsLinking( const KURL &url )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsLinking;
}

bool KProtocolInfo::supportsMoving( const KURL &url )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsMoving;
}

bool KProtocolInfo::canCopyFromFile( const KURL &url )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_canCopyFromFile;
}


bool KProtocolInfo::canCopyToFile( const KURL &url )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_canCopyToFile;
}

QString KProtocolInfo::defaultMimetype( const KURL &url )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url);
  if ( !prot )
    return QString::null;

  return prot->m_defaultMimetype;
}

void KProtocolInfo::virtual_hook( int id, void* data )
{ KSycocaEntry::virtual_hook( id, data ); }

