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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifdef MAKE_KDECORE_LIB //needed for proper linkage (win32)
#undef KIO_EXPORT
#define KIO_EXPORT KDE_EXPORT
#endif

#include "kprotocolinfo.h"
#include "kprotocolinfofactory.h"

#include <kstandarddirs.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kdebug.h>
#include <ksimpleconfig.h>
#include <kconfig.h>
#include <kstringhandler.h>

class KProtocolInfo::KProtocolInfoPrivate
{
public:
  QString docPath;
  QString protClass;
  KProtocolInfo::ExtraFieldList extraFields;
  bool showPreviews;
  bool canRenameFromFile;
  bool canRenameToFile;
  bool canDeleteRecursive;
  bool fileNameUsedForCopying; // true if using UDS_NAME, false if using KURL::fileName() [default]
  KURL::URIMode uriMode;
  QStringList capabilities;
  QString proxyProtocol;
};

//
// Internal functions:
//
KProtocolInfo::KProtocolInfo(const QString &path)
 : KSycocaEntry(path)
{
  d = new KProtocolInfoPrivate;
  QString fullPath = locate("services", path);

  KSimpleConfig config( fullPath, true );
  config.setGroup( "Protocol" );

  m_name = config.readEntry( "protocol" );
  m_exec = config.readPathEntry( "exec" );
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
  d->canRenameFromFile = config.readBoolEntry( "renameFromFile", false );
  d->canRenameToFile = config.readBoolEntry( "renameToFile", false );
  d->canDeleteRecursive = config.readBoolEntry( "deleteRecursive", false );
  d->fileNameUsedForCopying = config.readEntry( "fileNameUsedForCopying", "FromURL" ) == "Name";

  m_listing = config.readListEntry( "listing" );
  // Many .protocol files say "Listing=false" when they really mean "Listing=" (i.e. unsupported)
  if ( m_listing.count() == 1 && m_listing.first() == "false" )
    m_listing.clear();
  m_supportsListing = ( m_listing.count() > 0 );
  m_defaultMimetype = config.readEntry( "defaultMimetype" );
  m_determineMimetypeFromExtension = config.readBoolEntry( "determineMimetypeFromExtension", true );
  m_icon = config.readEntry( "Icon", "unknown" );
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

  d->docPath = config.readPathEntry( "DocPath" );
  d->protClass = config.readEntry( "Class" ).lower();
  if (d->protClass[0] != ':')
     d->protClass.prepend(':');

  QStringList extraNames = config.readListEntry( "ExtraNames" );
  QStringList extraTypes = config.readListEntry( "ExtraTypes" );
  QStringList::Iterator it = extraNames.begin();
  QStringList::Iterator typeit = extraTypes.begin();
  for( ; it != extraNames.end() && typeit != extraTypes.end(); ++it, ++typeit ) {
      d->extraFields.append( ExtraField( *it, *typeit ) );
  }

  d->showPreviews = config.readBoolEntry( "ShowPreviews", d->protClass == ":local" );

  tmp = config.readEntry( "URIMode", QString::null ).lower();
  if (tmp == "rawuri")
     d->uriMode = KURL::RawURI;
  else if (tmp == "mailto")
     d->uriMode = KURL::Mailto;
  else if (tmp == "url")
     d->uriMode = KURL::URL;
  else
     d->uriMode = KURL::Auto;

  d->capabilities = config.readListEntry( "Capabilities" );
  d->proxyProtocol = config.readEntry( "ProxiedBy" );
}

KProtocolInfo::KProtocolInfo( QDataStream& _str, int offset) :
	KSycocaEntry( _str, offset)
{
   d = new KProtocolInfoPrivate;
   load( _str );
}

KProtocolInfo::~KProtocolInfo()
{
   delete d;
}

void
KProtocolInfo::load( QDataStream& _str)
{
   // You may add new fields at the end. Make sure to update the version
   // number in ksycoca.h
   Q_INT32 i_inputType, i_outputType;
   Q_INT8 i_isSourceProtocol, i_isHelperProtocol,
          i_supportsListing, i_supportsReading,
          i_supportsWriting, i_supportsMakeDir,
          i_supportsDeleting, i_supportsLinking,
          i_supportsMoving, i_determineMimetypeFromExtension,
          i_canCopyFromFile, i_canCopyToFile, i_showPreviews,
          i_uriMode, i_canRenameFromFile, i_canRenameToFile,
          i_canDeleteRecursive, i_fileNameUsedForCopying;

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
        >> m_config >> m_maxSlaves >> d->docPath >> d->protClass
        >> d->extraFields >> i_showPreviews >> i_uriMode
        >> d->capabilities >> d->proxyProtocol
        >> i_canRenameFromFile >> i_canRenameToFile
        >> i_canDeleteRecursive >> i_fileNameUsedForCopying;

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
   d->canRenameFromFile = (i_canRenameFromFile != 0);
   d->canRenameToFile = (i_canRenameToFile != 0);
   d->canDeleteRecursive = (i_canDeleteRecursive != 0);
   d->fileNameUsedForCopying = (i_fileNameUsedForCopying != 0);
   m_determineMimetypeFromExtension = (i_determineMimetypeFromExtension != 0);
   d->showPreviews = (i_showPreviews != 0);
   d->uriMode = (KURL::URIMode) i_uriMode;
}

void
KProtocolInfo::save( QDataStream& _str)
{
   KSycocaEntry::save( _str );

   // You may add new fields at the end. Make sure to update the version
   // number in ksycoca.h
   Q_INT32 i_inputType, i_outputType;
   Q_INT8 i_isSourceProtocol, i_isHelperProtocol,
          i_supportsListing, i_supportsReading,
          i_supportsWriting, i_supportsMakeDir,
          i_supportsDeleting, i_supportsLinking,
          i_supportsMoving, i_determineMimetypeFromExtension,
          i_canCopyFromFile, i_canCopyToFile, i_showPreviews,
          i_uriMode, i_canRenameFromFile, i_canRenameToFile,
          i_canDeleteRecursive, i_fileNameUsedForCopying;

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
   i_canRenameFromFile = d->canRenameFromFile ? 1 : 0;
   i_canRenameToFile = d->canRenameToFile ? 1 : 0;
   i_canDeleteRecursive = d->canDeleteRecursive ? 1 : 0;
   i_fileNameUsedForCopying = d->fileNameUsedForCopying ? 1 : 0;
   i_determineMimetypeFromExtension = m_determineMimetypeFromExtension ? 1 : 0;
   i_showPreviews = d->showPreviews ? 1 : 0;
   i_uriMode = d->uriMode;

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
        << m_config << m_maxSlaves << d->docPath << d->protClass
        << d->extraFields << i_showPreviews << i_uriMode
        << d->capabilities << d->proxyProtocol
        << i_canRenameFromFile << i_canRenameToFile
        << i_canDeleteRecursive << i_fileNameUsedForCopying;
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
    return QString::fromLatin1("unknown");

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

KProtocolInfo::ExtraFieldList KProtocolInfo::extraFields( const KURL &url )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url.protocol());
  if ( !prot )
    return ExtraFieldList();

  return prot->d->extraFields;
}

QString KProtocolInfo::docPath( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return QString::null;

  return prot->d->docPath;
}

QString KProtocolInfo::protocolClass( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return QString::null;

  return prot->d->protClass;
}

bool KProtocolInfo::showFilePreview( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return false;

  return prot->d->showPreviews;
}

KURL::URIMode KProtocolInfo::uriParseMode( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return KURL::Auto;

  return prot->d->uriMode;
}

QStringList KProtocolInfo::capabilities( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return QStringList();

  return prot->d->capabilities;
}

QString KProtocolInfo::proxiedBy( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return QString::null;

  return prot->d->proxyProtocol;
}

bool KProtocolInfo::canRenameFromFile() const
{
  return d->canRenameFromFile;
}

bool KProtocolInfo::canRenameToFile() const
{
  return d->canRenameToFile;
}

bool KProtocolInfo::canDeleteRecursive() const
{
  return d->canDeleteRecursive;
}

KProtocolInfo::FileNameUsedForCopying KProtocolInfo::fileNameUsedForCopying() const
{
  return d->fileNameUsedForCopying ? Name : FromURL;
}

QDataStream& operator>>( QDataStream& s, KProtocolInfo::ExtraField& field )  {
  s >> field.name;
  s >> field.type;
  return s;
}

QDataStream& operator<<( QDataStream& s, const KProtocolInfo::ExtraField& field )  {
  s << field.name;
  s << field.type;
  return s;
}

// KURL based static functions are implemented in ../kio/kio/kprotocolinfo.cpp

void KProtocolInfo::virtual_hook( int id, void* data )
{ KSycocaEntry::virtual_hook( id, data ); }

