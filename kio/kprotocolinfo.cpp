/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kprotocolinfo.h"
#include "kprotocolinfofactory.h"

#include <kstddirs.h>
#include <kglobal.h>
#include <kapp.h>
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
  m_listing = config.readListEntry( "listing" );
  m_supportsListing = ( m_listing.count() > 0 );
  m_defaultMimetype = config.readEntry( "defaultMimetype" );
  m_mimetypesExcludedFromFastMode = config.readListEntry( "mimetypesExcludedFromFastMode" );
  m_patternsExcludedFromFastMode = config.readListEntry( "patternsExcludedFromFastMode" );
  m_icon = config.readEntry( "Icon", "mime_empty" );

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
          i_supportsMoving;
   _str >> m_name >> m_exec >> m_listing >> m_defaultMimetype
        >> m_mimetypesExcludedFromFastMode 
        >> m_patternsExcludedFromFastMode
        >> m_icon
        >> i_inputType >> i_outputType
        >> i_isSourceProtocol >> i_isHelperProtocol
        >> i_supportsListing >> i_supportsReading
        >> i_supportsWriting >> i_supportsMakeDir
        >> i_supportsDeleting >> i_supportsLinking
        >> i_supportsMoving;
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
          i_supportsMoving;

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


   _str << m_name << m_exec << m_listing << m_defaultMimetype
        << m_mimetypesExcludedFromFastMode 
        << m_patternsExcludedFromFastMode
        << m_icon
        << i_inputType << i_outputType
        << i_isSourceProtocol << i_isHelperProtocol
        << i_supportsListing << i_supportsReading
        << i_supportsWriting << i_supportsMakeDir
        << i_supportsDeleting << i_supportsLinking
        << i_supportsMoving;
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
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return prot->m_isSourceProtocol;
}

bool KProtocolInfo::isFilterProtocol( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return !prot->m_isSourceProtocol;
}

bool KProtocolInfo::isHelperProtocol( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

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
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return prot->m_supportsListing;
}

QStringList KProtocolInfo::listing( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return QStringList();
  }

  return prot->m_listing;
}

bool KProtocolInfo::supportsReading( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return prot->m_supportsReading;
}

bool KProtocolInfo::supportsWriting( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return prot->m_supportsWriting;
}

bool KProtocolInfo::supportsMakeDir( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return prot->m_supportsMakeDir;
}

bool KProtocolInfo::supportsDeleting( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return prot->m_supportsDeleting;
}

bool KProtocolInfo::supportsLinking( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return prot->m_supportsLinking;
}

bool KProtocolInfo::supportsMoving( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return prot->m_supportsMoving;
}

QString KProtocolInfo::icon( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return QString::fromLatin1("mime_empty");
  }

  return prot->m_icon;
}

QString KProtocolInfo::defaultMimetype( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return QString::null;
  }

  return prot->m_defaultMimetype;
}

bool KProtocolInfo::mimetypeFastMode( const QString& _protocol, const QString & _mimetype )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  // return true if the exclude-list doesn't contain this mimetype
  return !(prot->m_mimetypesExcludedFromFastMode.contains(_mimetype));
}

bool KProtocolInfo::patternFastMode( const QString& _protocol, const QString & _filename )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  // return true if the exclude-list doesn't contain this mimetype
  const QStringList & pat = prot->m_patternsExcludedFromFastMode;
  for ( QStringList::ConstIterator pit = pat.begin(); pit != pat.end(); ++pit )
    if ( KStringHandler::matchFilename( _filename, *pit ) )
      return false; // in the list -> EXCLUDED

  return true; // not in the list -> ok
}

QString KProtocolInfo::exec( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return QString::null;
  }

  return prot->m_exec;
}

KProtocolInfo::Type KProtocolInfo::inputType( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return T_NONE;
  }

  return prot->m_inputType;
}

KProtocolInfo::Type KProtocolInfo::outputType( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return T_NONE;
  }

  return prot->m_outputType;
}

