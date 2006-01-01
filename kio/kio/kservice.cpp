/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 - 2001 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 1999 - 2005 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include <config.h>

#include "kservice.h"
#include "kservice_p.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>

#include <qstring.h>
#include <qfile.h>
#include <qdir.h>
#include <qprogressdialog.h>

#include <kapplication.h>
#include <kauthorized.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <dcopclient.h>

#include "kservicefactory.h"
#include "kservicetypefactory.h"

class KService::Private
{
public:
  QStringList categories;
  QString menuId;
};

KService::KService( const QString & _name, const QString &_exec, const QString &_icon)
 : KSycocaEntry( QString()), d(new Private)
{
  m_bValid = true;
  m_bDeleted = false;
  m_strType = "Application";
  m_strName = _name;
  m_strExec = _exec;
  m_strIcon = _icon;
  m_bTerminal = false;
  m_bAllowAsDefault = true;
  m_initialPreference = 10;
}


KService::KService( const QString & _fullpath )
 : KSycocaEntry( _fullpath), d(new Private)
{
  KDesktopFile config( _fullpath );

  init(&config);
}

KService::KService( const KDesktopFile *config )
 : KSycocaEntry( config->fileName()), d(new Private)
{
  init(config);
}

void
KService::init( const KDesktopFile *config )
{
  m_bValid = true;

  bool absPath = !QDir::isRelativePath(entryPath());

  QMap<QString, QString> entryMap = config->entryMap(config->group());

  entryMap.remove("Encoding"); // reserved as part of Desktop Entry Standard
  entryMap.remove("Version");  // reserved as part of Desktop Entry Standard

  m_bDeleted = config->readEntry("Hidden", QVariant(false )).toBool();
  entryMap.remove("Hidden");
  if (m_bDeleted)
  {
    m_bValid = false;
    return;
  }

  m_strName = config->readEntry( "Name" );
  entryMap.remove("Name");
  if ( m_strName.isEmpty() )
  {
    if (config->readEntry( "Exec" ).isEmpty())
    {
      m_bValid = false;
      return;
    }
    // Try to make up a name.
    m_strName = entryPath();
    int i = m_strName.lastIndexOf('/');
    m_strName = m_strName.mid(i+1);
    i = m_strName.lastIndexOf('.');
    if (i != -1)
       m_strName = m_strName.left(i);
  }

  m_strType = config->readEntry( "Type" );
  entryMap.remove("Type");
  if ( m_strType.isEmpty() )
  {
    /*kdWarning(7012) << "The desktop entry file " << entryPath()
                    << " has no Type=... entry."
                    << " It should be \"Application\" or \"Service\"" << endl;
    m_bValid = false;
    return;*/
    m_strType = "Application";
  } else if ( m_strType != "Application" && m_strType != "Service" )
  {
    kdWarning(7012) << "The desktop entry file " << entryPath()
                    << " has Type=" << m_strType
                    << " instead of \"Application\" or \"Service\"" << endl;
    m_bValid = false;
    return;
  }

  // In case Try Exec is set, check if the application is available
  if (!config->tryExec()) {
      m_bDeleted = true;
      m_bValid = false;
      return;
  }

  QString resource = config->resource();

  if ( (m_strType == "Application") &&
       (!resource.isEmpty()) &&
       (resource != "apps") &&
       !absPath)
  {
    kdWarning(7012) << "The desktop entry file " << entryPath()
           << " has Type=" << m_strType << " but is located under \"" << resource
           << "\" instead of \"apps\"" << endl;
    m_bValid = false;
    return;
  }

  if ( (m_strType == "Service") &&
       (!resource.isEmpty()) &&
       (resource != "services") &&
       !absPath)
  {
    kdWarning(7012) << "The desktop entry file " << entryPath()
           << " has Type=" << m_strType << " but is located under \"" << resource
           << "\" instead of \"services\"" << endl;
    m_bValid = false;
    return;
  }

  QString _name = entryPath();
  int pos = _name.lastIndexOf('/');
  if (pos != -1)
     _name = _name.mid(pos+1);
  pos = _name.indexOf('.');
  if (pos != -1)
     _name = _name.left(pos);

  m_strExec = config->readPathEntry( "Exec" );
  entryMap.remove("Exec");

  m_strIcon = config->readEntry( "Icon" );
  entryMap.remove("Icon");
  m_bTerminal = (config->readEntry( "Terminal" ), QVariant(false)).toBool(); // should be a property IMHO
  entryMap.remove("Terminal");
  m_strTerminalOptions = config->readEntry( "TerminalOptions" ); // should be a property IMHO
  entryMap.remove("TerminalOptions");
  m_strPath = config->readPathEntry( "Path" );
  entryMap.remove("Path");
  m_strComment = config->readEntry( "Comment" );
  entryMap.remove("Comment");
  m_strGenName = config->readEntry( "GenericName" );
  entryMap.remove("GenericName");
  QString _untranslatedGenericName = config->readEntryUntranslated( "GenericName" );
  entryMap.insert("UntranslatedGenericName", _untranslatedGenericName);

  m_lstKeywords = config->readListEntry("Keywords");
  entryMap.remove("Keywords");
  d->categories = config->readListEntry("Categories", ';');
  entryMap.remove("Categories");
  m_strLibrary = config->readEntry( "X-KDE-Library" );
  entryMap.remove("X-KDE-Library");
  m_strInit = config->readEntry("X-KDE-Init" );
  entryMap.remove("X-KDE-Init");

  m_lstServiceTypes = config->readListEntry( "ServiceTypes" );
  entryMap.remove("ServiceTypes");
  m_lstServiceTypes += config->readListEntry( "MimeType", ';' ); // freedesktop.org standard
  entryMap.remove("MimeType");

  if ( m_strType == "Application" && !m_lstServiceTypes.contains("Application") )
    // Applications implement the service type "Application" ;-)
    m_lstServiceTypes += "Application";

  QString dcopServiceType = config->readEntry("X-DCOP-ServiceType").toLower();
  entryMap.remove("X-DCOP-ServiceType");
  if (dcopServiceType == "unique")
     m_DCOPServiceType = DCOP_Unique;
  else if (dcopServiceType == "multi")
     m_DCOPServiceType = DCOP_Multi;
  else if (dcopServiceType == "wait")
     m_DCOPServiceType = DCOP_Wait;
  else
     m_DCOPServiceType = DCOP_None;

  m_strDesktopEntryName = _name.toLower();

  m_bAllowAsDefault = config->readEntry("AllowDefault", QVariant(true )).toBool();
  entryMap.remove("AllowDefault");

  m_initialPreference = config->readEntry( "InitialPreference", QVariant(1 )).toInt();
  entryMap.remove("InitialPreference");

  // Store all additional entries in the property map.
  // A QMap<QString,QString> would be easier for this but we can't
  // break BC, so we have to store it in m_mapProps.
//  qWarning("Path = %s", entryPath().latin1());
  QMap<QString,QString>::ConstIterator it = entryMap.begin();
  for( ; it != entryMap.end();++it)
  {
//     qWarning("   Key = %s Data = %s", it.key().toLatin1().data(), it->toLatin1().data());
     m_mapProps.insert( it.key(), QVariant( *it));
  }
}

KService::KService( QDataStream& _str, int _offset ) : KSycocaEntry( _str, _offset ), d(new Private)
{
  load( _str );
}

KService::~KService()
{
  delete d;
}

QPixmap KService::pixmap( KIcon::Group _group, int _force_size, int _state, QString * _path ) const
{
  KIconLoader *iconLoader=KGlobal::iconLoader();
  if (!iconLoader->extraDesktopThemesAdded())
  {
      QPixmap _pixmap=iconLoader->loadIcon( m_strIcon, _group, _force_size, _state, _path, true );
      if (!_pixmap.isNull() ) return _pixmap;

      iconLoader->addExtraDesktopThemes();
  }

  return iconLoader->loadIcon( m_strIcon, _group, _force_size, _state, _path );
}

void KService::load( QDataStream& s )
{
  // dummies are here because of fields that were removed, to keep bin compat.
  // Feel free to re-use, but fields for Applications only (not generic services)
  // should rather be added to application.desktop
  qint8 def, term, dummy1, dummy2;
  qint8 dst, initpref;
  QString dummyStr1, dummyStr2;
  int dummyI1, dummyI2;
  quint32 dummyUI32;

  // WARNING: IN KDE 3.x THIS NEEDS TO REMAIN COMPATIBLE WITH KDE 2.x!
  // !! This data structure should remain binary compatible at all times !!
  // You may add new fields at the end. Make sure to update the version
  // number in ksycoca.h
  s >> m_strType >> m_strName >> m_strExec >> m_strIcon
    >> term >> m_strTerminalOptions
    >> m_strPath >> m_strComment >> m_lstServiceTypes >> def >> m_mapProps
    >> m_strLibrary >> dummyI1 >> dummyI2
    >> dst
    >> m_strDesktopEntryName
    >> dummy1 >> dummyStr1 >> initpref >> dummyStr2 >> dummy2
    >> m_lstKeywords >> m_strInit >> dummyUI32 >> m_strGenName
    >> d->categories >> d->menuId;

  m_bAllowAsDefault = def;
  m_bTerminal = term;
  m_DCOPServiceType = (DCOPServiceType_t) dst;
  m_initialPreference = initpref;

  m_bValid = true;
}

void KService::save( QDataStream& s )
{
  KSycocaEntry::save( s );
  qint8 def = m_bAllowAsDefault, initpref = m_initialPreference;
  qint8 term = m_bTerminal;
  qint8 dst = (qint8) m_DCOPServiceType;
  qint8 dummy1 = 0, dummy2 = 0; // see ::load
  QString dummyStr1, dummyStr2;
  int dummyI1 = 0, dummyI2 = 0;
  quint32 dummyUI32 = 0;

  // WARNING: IN KDE 3.x THIS NEEDS TO REMAIN COMPATIBLE WITH KDE 2.x!
  // !! This data structure should remain binary compatible at all times !!
  // You may add new fields at the end. Make sure to update the version
  // number in ksycoca.h
  s << m_strType << m_strName << m_strExec << m_strIcon
    << term << m_strTerminalOptions
    << m_strPath << m_strComment << m_lstServiceTypes << def << m_mapProps
    << m_strLibrary << dummyI1 << dummyI2
    << dst
    << m_strDesktopEntryName
    << dummy1 << dummyStr1 << initpref << dummyStr2 << dummy2
    << m_lstKeywords << m_strInit << dummyUI32 << m_strGenName
    << d->categories << d->menuId;
}

bool KService::hasServiceType( const QString& _servicetype ) const
{
  if (!m_bValid) return false; // safety test

  //kdDebug(7012) << "Testing " << m_strDesktopEntryName << " for " << _servicetype << endl;

  KMimeType::Ptr mimePtr = KMimeType::mimeType( _servicetype );
  if ( mimePtr && mimePtr == KMimeType::defaultMimeTypePtr() )
      mimePtr = 0;

  bool isNumber;
  // For each service type we are associated with, if it doesn't
  // match then we try its parent service types.
  QStringList::ConstIterator it = m_lstServiceTypes.begin();
  for( ; it != m_lstServiceTypes.end(); ++it )
  {
      it->toInt(&isNumber);
      if (isNumber)
         continue;
      //kdDebug(7012) << "    has " << (*it) << endl;
      KServiceType::Ptr ptr = KServiceType::serviceType( *it );
      if ( ptr && ptr->inherits( _servicetype ) )
          return true;

      // The mimetype inheritance ("is also") works the other way.
      // e.g. if we're looking for a handler for mimePtr==smb-workgroup
      // then a handler for inode/directory is ok.
      if ( mimePtr && mimePtr->is( *it ) )
          return true;
  }
  return false;
}

int KService::initialPreferenceForMimeType( const QString& mimeType ) const
{
  if (!m_bValid) return 0; // safety test

  bool isNumber;

  // For each service type we are associated with
  QStringList::ConstIterator it = m_lstServiceTypes.begin();
  for( ; it != m_lstServiceTypes.end(); ++it )
  {
      it->toInt(&isNumber);
      if (isNumber)
         continue;
      //kdDebug(7012) << "    has " << (*it) << endl;
      KServiceType::Ptr ptr = KServiceType::serviceType( *it );
      if ( !ptr || !ptr->inherits( mimeType ) )
          continue;

      int initalPreference = m_initialPreference;
      ++it;
      if (it != m_lstServiceTypes.end())
      {
         int i = it->toInt(&isNumber);
         if (isNumber)
            initalPreference = i;
      }
      return initalPreference;
  }

  KMimeType::Ptr mimePtr = KMimeType::mimeType( mimeType );
  if ( mimePtr && mimePtr == KMimeType::defaultMimeTypePtr() )
      mimePtr = 0;

  // Try its parent service types.
  it = m_lstServiceTypes.begin();
  for( ; it != m_lstServiceTypes.end(); ++it )
  {
      it->toInt(&isNumber);
      if (isNumber)
         continue;

      // The mimetype inheritance ("is also") works the other way.
      // e.g. if we're looking for a handler for mimePtr==smb-workgroup
      // then a handler for inode/directory is ok.
      if ( !mimePtr || !mimePtr->is( *it ) )
          continue;

      int initalPreference = m_initialPreference;
      ++it;
      if (it != m_lstServiceTypes.end())
      {
         int i = it->toInt(&isNumber);
         if (isNumber)
            initalPreference = i;
      }
      return initalPreference;
  }
  return 0;
}

class KServiceReadProperty : public KConfigBase
{
public:
   KServiceReadProperty(const QString &_key, const QByteArray &_value)
	: key(_key), value(_value) { }

   bool internalHasGroup(const QByteArray &) const { /*qDebug("hasGroup(const QByteArray &)");*/ return false; }

   QStringList groupList() const { return QStringList(); }

   QMap<QString,QString> entryMap(const QString &) const
      { return QMap<QString,QString>(); }

   void reparseConfiguration() { }

   KEntryMap internalEntryMap( const QString &) const { return KEntryMap(); }

   KEntryMap internalEntryMap() const { return KEntryMap(); }

   void putData(const KEntryKey &, const KEntry&, bool) { }

   KEntry lookupData(const KEntryKey &) const
   { KEntry entry; entry.mValue = value; return entry; }
protected:
   QString key;
   QByteArray value;
};

QVariant KService::property( const QString& _name) const
{
   return property( _name, QVariant::Invalid);
}

// Return a string QVariant if string isn't null, and invalid variant otherwise
// (the variant must be invalid if the field isn't in the .desktop file)
// This allows trader queries like "exist Library" to work.
static QVariant makeStringVariant( const QString& string )
{
    // Using isEmpty here would be wrong.
    // Empty is "specified but empty", null is "not specified" (in the .desktop file)
    return string.isNull() ? QVariant() : QVariant( string );
}

QVariant KService::property( const QString& _name, QVariant::Type t ) const
{
  if ( _name == "Type" )
    return QVariant( m_strType ); // can't be null
  else if ( _name == "Name" )
    return QVariant( m_strName ); // can't be null
  else if ( _name == "Exec" )
    return makeStringVariant( m_strExec );
  else if ( _name == "Icon" )
    return makeStringVariant( m_strIcon );
  else if ( _name == "Terminal" )
    return QVariant( static_cast<int>(m_bTerminal) );
  else if ( _name == "TerminalOptions" )
    return makeStringVariant( m_strTerminalOptions );
  else if ( _name == "Path" )
    return makeStringVariant( m_strPath );
  else if ( _name == "Comment" )
    return makeStringVariant( m_strComment );
  else if ( _name == "GenericName" )
    return makeStringVariant( m_strGenName );
  else if ( _name == "ServiceTypes" )
    return QVariant( m_lstServiceTypes );
  else if ( _name == "AllowAsDefault" )
    return QVariant( static_cast<int>(m_bAllowAsDefault) );
  else if ( _name == "InitialPreference" )
    return QVariant( m_initialPreference );
  else if ( _name == "Library" )
    return makeStringVariant( m_strLibrary );
  else if ( _name == "DesktopEntryPath" ) // can't be null
    return QVariant( entryPath() );
  else if ( _name == "DesktopEntryName")
    return QVariant( m_strDesktopEntryName ); // can't be null
  else if ( _name == "Categories")
    return QVariant( d->categories );
  else if ( _name == "Keywords")
    return QVariant( m_lstKeywords );

  // Ok we need to convert the property from a QString to its real type.
  // Maybe the caller helped us.
  if (t == QVariant::Invalid)
  {
    // No luck, let's ask KServiceTypeFactory what the type of this property
    // is supposed to be.
    t = KServiceTypeFactory::self()->findPropertyTypeByName(_name);
    if (t == QVariant::Invalid)
    {
      kdDebug(7012) << "Request for unknown property '" << _name << "'\n";
      return QVariant(); // Unknown property: Invalid variant.
    }
  }

  // Then we use a homebuild class based on KConfigBase to convert the QString.
  // For some often used property types we do the conversion ourselves.
  QMap<QString,QVariant>::ConstIterator it = m_mapProps.find( _name );
  if ( (it == m_mapProps.end()) || (!it->isValid()))
  {
     //kdDebug(7012) << "Property not found " << _name << endl;
     return QVariant(); // No property set.
  }

  switch(t)
  {
    case QVariant::String:
        return *it;
    case QVariant::Bool:
    case QVariant::Int:
        {
           QString aValue = it->toString().toLower();
           int val = 0;
           if (aValue == "true" || aValue == "on" || aValue == "yes")
              val = 1;
           else
           {
              bool bOK;
              val = aValue.toInt( &bOK );
              if( !bOK )
                 val = 0;
           }
           if (t == QVariant::Bool)
           {
               return QVariant(bool(val));
           }
           return QVariant(val);
        }
    default:
        // All others
        KServiceReadProperty ksrp(_name, it->toString().toUtf8());
        return ksrp.readPropertyEntry(_name, t);
  }
}

QStringList KService::propertyNames() const
{
  QStringList res;

  QMap<QString,QVariant>::ConstIterator it = m_mapProps.begin();
  for( ; it != m_mapProps.end(); ++it )
    res.append( it.key() );

  res.append( "Type" );
  res.append( "Name" );
  res.append( "Comment" );
  res.append( "GenericName" );
  res.append( "Icon" );
  res.append( "Exec" );
  res.append( "Terminal" );
  res.append( "TerminalOptions" );
  res.append( "Path" );
  res.append( "ServiceTypes" );
  res.append( "AllowAsDefault" );
  res.append( "InitialPreference" );
  res.append( "Library" );
  res.append( "DesktopEntryPath" );
  res.append( "DesktopEntryName" );
  res.append( "Keywords" );
  res.append( "Categories" );

  return res;
}

KService::List KService::allServices()
{
  return KServiceFactory::self()->allServices();
}

KService::Ptr KService::serviceByName( const QString& _name )
{
  return KServiceFactory::self()->findServiceByName( _name );
}

KService::Ptr KService::serviceByDesktopPath( const QString& _name )
{
  return KServiceFactory::self()->findServiceByDesktopPath( _name );
}

KService::Ptr KService::serviceByDesktopName( const QString& _name )
{
  KService::Ptr s = KServiceFactory::self()->findServiceByDesktopName( _name.toLower() );
  if (!s && !_name.startsWith("kde-"))
     s = KServiceFactory::self()->findServiceByDesktopName( "kde-"+_name.toLower() );
  return s;
}

KService::Ptr KService::serviceByMenuId( const QString& _name )
{
  return KServiceFactory::self()->findServiceByMenuId( _name );
}

KService::Ptr KService::serviceByStorageId( const QString& _storageId )
{
  KService::Ptr service = KService::serviceByMenuId( _storageId );
  if (service)
     return service;

  service = KService::serviceByDesktopPath(_storageId);
  if (service)
     return service;

  if (!QDir::isRelativePath(_storageId) && QFile::exists(_storageId))
     return new KService(_storageId);

  QString tmp = _storageId;
  tmp = tmp.mid(tmp.lastIndexOf('/')+1); // Strip dir

  if (tmp.endsWith(".desktop"))
     tmp.truncate(tmp.length()-8);

  if (tmp.endsWith(".kdelnk"))
     tmp.truncate(tmp.length()-7);

  service = KService::serviceByDesktopName(tmp);

  return service;
}

KService::List KService::allInitServices()
{
  return KServiceFactory::self()->allInitServices();
}

bool KService::substituteUid() const {
  QVariant v = property("X-KDE-SubstituteUID", QVariant::Bool);
  return v.isValid() && v.toBool();
}

QString KService::username() const {
  // See also KDesktopFile::tryExec()
  QString user;
  QVariant v = property("X-KDE-Username", QVariant::String);
  user = v.isValid() ? v.toString() : QString();
  if (user.isEmpty())
     user = ::getenv("ADMIN_ACCOUNT");
  if (user.isEmpty())
     user = "root";
  return user;
}

bool KService::noDisplay() const {
  QMap<QString,QVariant>::ConstIterator it = m_mapProps.find( "NoDisplay" );
 if ( (it != m_mapProps.end()) && (it->isValid()))
  {
     QString aValue = it->toString().toLower();
     if (aValue == "true" || aValue == "on" || aValue == "yes")
        return true;
  }

  it = m_mapProps.find( "OnlyShowIn" );
  if ( (it != m_mapProps.end()) && (it->isValid()))
  {
     QString aValue = it->toString();
     QStringList aList = aValue.split(';');
     if (!aList.contains("KDE"))
        return true;
  }

  it = m_mapProps.find( "NotShowIn" );
  if ( (it != m_mapProps.end()) && (it->isValid()))
  {
     QString aValue = it->toString();
     QStringList aList = aValue.split(';');
     if (aList.contains("KDE"))
        return true;
  }

  if (!KAuthorized::authorizeControlModule(d->menuId))
     return true;

  return false;
}

QString KService::untranslatedGenericName() const {
  QVariant v = property("UntranslatedGenericName", QVariant::String);
  return v.isValid() ? v.toString() : QString();
}

QString KService::parentApp() const {
  QMap<QString,QVariant>::ConstIterator it = m_mapProps.find( "X-KDE-ParentApp" );
  if ( (it == m_mapProps.end()) || (!it->isValid()))
  {
     return QString();
  }

  return it->toString();
}

bool KService::allowMultipleFiles() const {
  // Can we pass multiple files on the command line or do we have to start the application for every single file ?
  return (m_strExec.contains( "%F" ) || m_strExec.contains( "%U" ) ||
          m_strExec.contains( "%N" ) || m_strExec.contains( "%D" ));
}

QStringList KService::categories() const
{
  return d->categories;
}

QString KService::menuId() const
{
  return d->menuId;
}

void KService::setMenuId(const QString &_menuId)
{
  d->menuId = _menuId;
}

QString KService::storageId() const
{
  if (!d->menuId.isEmpty())
     return d->menuId;
  return entryPath();
}

QString KService::locateLocal() const
{
  if (d->menuId.isEmpty() || desktopEntryPath().startsWith(".hidden") ||
      (QDir::isRelativePath(desktopEntryPath()) && d->categories.isEmpty()))
     return KDesktopFile::locateLocal(desktopEntryPath());

  return ::locateLocal("xdgdata-apps", d->menuId);
}

QString KService::newServicePath(bool showInMenu, const QString &suggestedName,
                                QString *menuId, const QStringList *reservedMenuIds)
{
   QString base = suggestedName;
   if (!showInMenu)
     base.prepend("kde-");

   QString result;
   for(int i = 1; true; i++)
   {
      if (i == 1)
         result = base + ".desktop";
      else
         result = base + QString("-%1.desktop").arg(i);

      if (reservedMenuIds && reservedMenuIds->contains(result))
         continue;

      // Lookup service by menu-id
      KService::Ptr s = serviceByMenuId(result);
      if (s)
         continue;

      if (showInMenu)
      {
         if (!locate("xdgdata-apps", result).isEmpty())
            continue;
      }
      else
      {
         QString file = result.mid(4); // Strip "kde-"
         if (!locate("apps", ".hidden/"+file).isEmpty())
            continue;
      }

      break;
   }
   if (menuId)
      *menuId = result;

   if (showInMenu)
   {
       return ::locateLocal("xdgdata-apps", result);
   }

   QString file = result.mid(4); // Strip "kde-"
   return ::locateLocal("apps", ".hidden/"+file);
}


void KService::virtual_hook( int id, void* data )
{ KSycocaEntry::virtual_hook( id, data ); }


void KService::rebuildKSycoca(QWidget *parent)
{
  KServiceProgressDialog dlg(parent, "ksycoca_progress",
                      i18n("Updating System Configuration"),
                      i18n("Updating system configuration."));

  QByteArray data;
  DCOPClient *client = KApplication::dcopClient();

  int result = client->callAsync("kded", "kbuildsycoca", "recreate()",
               data, &dlg, SLOT(slotFinished()));

  if (result)
  {
     dlg.exec();
  }
}

KServiceProgressDialog::KServiceProgressDialog(QWidget *_parent, const char *_name,
                          const QString &_caption, const QString &text)
 : QProgressDialog(_parent)
{
  connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotProgress()));
  setObjectName(_name);
  setWindowTitle(_caption);
  setModal(true);
  setLabelText(text);
  setRange(0, 20);
  m_timeStep = 700;
  m_timer.start(m_timeStep);
  setAutoClose(false);
}

void
KServiceProgressDialog::slotProgress()
{
  int p = value();
  if (p == 18)
  {
     reset();
     setValue(1);
     m_timeStep = m_timeStep * 2;
     m_timer.start(m_timeStep);
  }
  else
  {
     setValue(p+1);
  }
}

void
KServiceProgressDialog::slotFinished()
{
  setValue(20);
  m_timer.stop();
  QTimer::singleShot(1000, this, SLOT(close()));
}

#include "kservice_p.moc"
