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
#include "kprotocolinfo_p.h"
#include "kprotocolinfofactory.h"

#include <kstandarddirs.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kstringhandler.h>
#include <kconfiggroup.h>

//
// Internal functions:
//
KProtocolInfo::KProtocolInfo(const QString &path)
 : KSycocaEntry(*new KProtocolInfoPrivate(path, this))
{
    Q_D(KProtocolInfo);
  QString fullPath = KStandardDirs::locate("services", path);

  KConfig sconfig( fullPath );
  KConfigGroup config(&sconfig, "Protocol" );

  m_name = config.readEntry( "protocol" );
  m_exec = config.readPathEntry( "exec", QString() );
  m_isSourceProtocol = config.readEntry( "source", true );
  m_isHelperProtocol = config.readEntry( "helper", false );
  m_supportsReading = config.readEntry( "reading", false );
  m_supportsWriting = config.readEntry( "writing", false );
  m_supportsMakeDir = config.readEntry( "makedir", false );
  m_supportsDeleting = config.readEntry( "deleting", false );
  m_supportsLinking = config.readEntry( "linking", false );
  m_supportsMoving = config.readEntry( "moving", false );
  m_supportsOpening = config.readEntry( "opening", false );
  m_canCopyFromFile = config.readEntry( "copyFromFile", false );
  m_canCopyToFile = config.readEntry( "copyToFile", false );
  d->canRenameFromFile = config.readEntry( "renameFromFile", false );
  d->canRenameToFile = config.readEntry( "renameToFile", false );
  d->canDeleteRecursive = config.readEntry( "deleteRecursive", false );
  d->fileNameUsedForCopying = config.readEntry( "fileNameUsedForCopying", "FromURL" ) == "Name";

  m_listing = config.readEntry( "listing", QStringList() );
  // Many .protocol files say "Listing=false" when they really mean "Listing=" (i.e. unsupported)
  if ( m_listing.count() == 1 && m_listing.first() == "false" )
    m_listing.clear();
  m_supportsListing = ( m_listing.count() > 0 );
  m_defaultMimetype = config.readEntry( "defaultMimetype" );
  m_determineMimetypeFromExtension = config.readEntry( "determineMimetypeFromExtension", true );
  d->archiveMimetype = config.readEntry("archiveMimetype", QStringList());
  m_icon = config.readEntry( "Icon" );
  m_config = config.readEntry( "config", m_name );
  m_maxSlaves = config.readEntry( "maxInstances", 1);
  d->maxSlavesPerHost = config.readEntry( "maxInstancesPerHost", 0);

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

  d->docPath = config.readPathEntry( "X-DocPath", QString() );
  if (d->docPath.isEmpty())
    d->docPath = config.readPathEntry( "DocPath", QString() );
  d->protClass = config.readEntry( "Class" ).toLower();
  if (d->protClass[0] != ':')
     d->protClass.prepend(QLatin1Char(':'));

  const QStringList extraNames = config.readEntry( "ExtraNames", QStringList() );
  const QStringList extraTypes = config.readEntry( "ExtraTypes", QStringList() );
  QStringList::const_iterator it = extraNames.begin();
  QStringList::const_iterator typeit = extraTypes.begin();
  for( ; it != extraNames.end() && typeit != extraTypes.end(); ++it, ++typeit ) {
      QVariant::Type type = QVariant::nameToType( (*typeit).toLatin1() );
      // currently QVariant::Type and ExtraField::Type use the same subset of values, so we can just cast.
      d->extraFields.append( ExtraField( *it, static_cast<ExtraField::Type>(type) ) );
  }

  d->showPreviews = config.readEntry( "ShowPreviews", d->protClass == ":local" );

  d->capabilities = config.readEntry( "Capabilities", QStringList() );
  d->proxyProtocol = config.readEntry( "ProxiedBy" );
}

KProtocolInfo::KProtocolInfo( QDataStream& _str, int offset) :
	KSycocaEntry(*new KProtocolInfoPrivate( _str, offset, this) )
{
   load( _str );
}

KProtocolInfo::~KProtocolInfo()
{
}

void
KProtocolInfo::load( QDataStream& _str)
{
    Q_D(KProtocolInfo);
   // You may add new fields at the end. Make sure to update the version
   // number in ksycoca.h
   qint32 i_inputType, i_outputType;
   qint8 i_isSourceProtocol, i_isHelperProtocol,
          i_supportsListing, i_supportsReading,
          i_supportsWriting, i_supportsMakeDir,
          i_supportsDeleting, i_supportsLinking,
          i_supportsMoving, i_supportsOpening,
          i_determineMimetypeFromExtension,
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
        >> i_supportsMoving >> i_supportsOpening
        >> i_canCopyFromFile >> i_canCopyToFile
        >> m_config >> m_maxSlaves >> d->docPath >> d->protClass
        >> d->extraFields >> i_showPreviews >> i_uriMode
        >> d->capabilities >> d->proxyProtocol
        >> i_canRenameFromFile >> i_canRenameToFile
        >> i_canDeleteRecursive >> i_fileNameUsedForCopying
        >> d->archiveMimetype >> d->maxSlavesPerHost;

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
   m_supportsOpening = (i_supportsOpening != 0);
   m_canCopyFromFile = (i_canCopyFromFile != 0);
   m_canCopyToFile = (i_canCopyToFile != 0);
   d->canRenameFromFile = (i_canRenameFromFile != 0);
   d->canRenameToFile = (i_canRenameToFile != 0);
   d->canDeleteRecursive = (i_canDeleteRecursive != 0);
   d->fileNameUsedForCopying = (i_fileNameUsedForCopying != 0);
   m_determineMimetypeFromExtension = (i_determineMimetypeFromExtension != 0);
   d->showPreviews = (i_showPreviews != 0);
}

void
KProtocolInfoPrivate::save( QDataStream& _str)
{
   KSycocaEntryPrivate::save( _str );

   // You may add new fields at the end. Make sure to update the version
   // number in ksycoca.h
   qint32 i_inputType, i_outputType;
   qint8 i_isSourceProtocol, i_isHelperProtocol,
          i_supportsListing, i_supportsReading,
          i_supportsWriting, i_supportsMakeDir,
          i_supportsDeleting, i_supportsLinking,
          i_supportsMoving, i_supportsOpening,
          i_determineMimetypeFromExtension,
          i_canCopyFromFile, i_canCopyToFile, i_showPreviews,
          i_uriMode, i_canRenameFromFile, i_canRenameToFile,
          i_canDeleteRecursive, i_fileNameUsedForCopying;

   i_inputType = (qint32) q->m_inputType;
   i_outputType = (qint32) q->m_outputType;
   i_isSourceProtocol = q->m_isSourceProtocol ? 1 : 0;
   i_isHelperProtocol = q->m_isHelperProtocol ? 1 : 0;
   i_supportsListing = q->m_supportsListing ? 1 : 0;
   i_supportsReading = q->m_supportsReading ? 1 : 0;
   i_supportsWriting = q->m_supportsWriting ? 1 : 0;
   i_supportsMakeDir = q->m_supportsMakeDir ? 1 : 0;
   i_supportsDeleting = q->m_supportsDeleting ? 1 : 0;
   i_supportsLinking = q->m_supportsLinking ? 1 : 0;
   i_supportsMoving = q->m_supportsMoving ? 1 : 0;
   i_supportsOpening = q->m_supportsOpening ? 1 : 0;
   i_canCopyFromFile = q->m_canCopyFromFile ? 1 : 0;
   i_canCopyToFile = q->m_canCopyToFile ? 1 : 0;
   i_canRenameFromFile = canRenameFromFile ? 1 : 0;
   i_canRenameToFile = canRenameToFile ? 1 : 0;
   i_canDeleteRecursive = canDeleteRecursive ? 1 : 0;
   i_fileNameUsedForCopying = fileNameUsedForCopying ? 1 : 0;
   i_determineMimetypeFromExtension = q->m_determineMimetypeFromExtension ? 1 : 0;
   i_showPreviews = showPreviews ? 1 : 0;
  i_uriMode = 0;

   _str << q->m_name << q->m_exec << q->m_listing << q->m_defaultMimetype
        << i_determineMimetypeFromExtension
        << q->m_icon
        << i_inputType << i_outputType
        << i_isSourceProtocol << i_isHelperProtocol
        << i_supportsListing << i_supportsReading
        << i_supportsWriting << i_supportsMakeDir
        << i_supportsDeleting << i_supportsLinking
        << i_supportsMoving << i_supportsOpening
        << i_canCopyFromFile << i_canCopyToFile
        << q->m_config << q->m_maxSlaves << docPath << protClass
        << extraFields << i_showPreviews << i_uriMode
        << capabilities << proxyProtocol
        << i_canRenameFromFile << i_canRenameToFile
        << i_canDeleteRecursive << i_fileNameUsedForCopying
        << archiveMimetype << maxSlavesPerHost;
}


//
// Static functions:
//

QStringList KProtocolInfo::protocols()
{
  return KProtocolInfoFactory::self()->protocols();
}

bool KProtocolInfo::isFilterProtocol( const QString& _protocol )
{
  // We call the findProtocol directly (not via KProtocolManager) to bypass any proxy settings.
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return false;

  return !prot->m_isSourceProtocol;
}

QString KProtocolInfo::icon( const QString& _protocol )
{
  // We call the findProtocol directly (not via KProtocolManager) to bypass any proxy settings.
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return QString();

  return prot->m_icon;
}

QString KProtocolInfo::config( const QString& _protocol )
{
  // We call the findProtocol directly (not via KProtocolManager) to bypass any proxy settings.
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return QString();

  return QString("kio_%1rc").arg(prot->m_config);
}

int KProtocolInfo::maxSlaves( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return 1;

  return prot->m_maxSlaves;
}

int KProtocolInfo::maxSlavesPerHost( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return 0;

  return prot->d_func()->maxSlavesPerHost;
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
    return QString();

  return prot->m_exec;
}

KProtocolInfo::ExtraFieldList KProtocolInfo::extraFields( const KUrl &url )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(url.protocol());
  if ( !prot )
    return ExtraFieldList();

  return prot->d_func()->extraFields;
}

QString KProtocolInfo::docPath( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return QString();

  return prot->d_func()->docPath;
}

QString KProtocolInfo::protocolClass( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return QString();

  return prot->d_func()->protClass;
}

bool KProtocolInfo::showFilePreview( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return false;

  return prot->d_func()->showPreviews;
}

QStringList KProtocolInfo::capabilities( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return QStringList();

  return prot->d_func()->capabilities;
}

QString KProtocolInfo::proxiedBy( const QString& _protocol )
{
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
  if ( !prot )
    return QString();

  return prot->d_func()->proxyProtocol;
}

QString KProtocolInfo::defaultMimeType() const
{
    return m_defaultMimetype;
}


QStringList KProtocolInfo::archiveMimeTypes() const
{
    Q_D(const KProtocolInfo);
    return d->archiveMimetype;
}

bool KProtocolInfo::supportsListing() const
{
    return m_supportsListing;
}

bool KProtocolInfo::canRenameFromFile() const
{
    Q_D(const KProtocolInfo);
  return d->canRenameFromFile;
}

bool KProtocolInfo::canRenameToFile() const
{
    Q_D(const KProtocolInfo);
  return d->canRenameToFile;
}

bool KProtocolInfo::canDeleteRecursive() const
{
    Q_D(const KProtocolInfo);
  return d->canDeleteRecursive;
}

KProtocolInfo::FileNameUsedForCopying KProtocolInfo::fileNameUsedForCopying() const
{
    Q_D(const KProtocolInfo);
  return d->fileNameUsedForCopying ? Name : FromUrl;
}

bool KProtocolInfo::isFilterProtocol( const KUrl &url )
{
  return isFilterProtocol (url.protocol());
}

bool KProtocolInfo::isHelperProtocol( const KUrl &url )
{
  return isHelperProtocol (url.protocol());
}

bool KProtocolInfo::isHelperProtocol( const QString &protocol )
{
  // We call the findProtocol directly (not via KProtocolManager) to bypass any proxy settings.
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(protocol);
  if ( !prot )
    return false;

  return prot->m_isHelperProtocol;
}

bool KProtocolInfo::isKnownProtocol( const KUrl &url )
{
  return isKnownProtocol (url.protocol());
}

bool KProtocolInfo::isKnownProtocol( const QString &protocol )
{
  // We call the findProtocol (const QString&) to bypass any proxy settings.
  KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(protocol);
  return prot;
}

QDataStream& operator>>( QDataStream& s, KProtocolInfo::ExtraField& field )  {
  s >> field.name;
  int type;
  s >> type;
  field.type = static_cast<KProtocolInfo::ExtraField::Type>( type );
  return s;
}

QDataStream& operator<<( QDataStream& s, const KProtocolInfo::ExtraField& field )  {
  s << field.name;
  s << static_cast<int>( field.type );
  return s;
}
