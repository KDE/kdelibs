/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 - 2001 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 1999        David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

// $Id$

#include <config.h>

#include "kservice.h"
#include <sys/types.h>
#include <sys/stat.h>

#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>

#include <qstring.h>
#include <qfile.h>
#include <qtl.h>

#include <ksimpleconfig.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfigbase.h>
#include <dcopclient.h>

#include "kservicefactory.h"
#include "kservicetypefactory.h"
#include "kservicetype.h"
#include "kuserprofile.h"
#include "ksycoca.h"

KService::KService( const QString & _name, const QString &_exec, const QString &_icon)
 : KSycocaEntry( QString::null)
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
 : KSycocaEntry( _fullpath)
{
  KDesktopFile config( _fullpath );

  init(&config);
}

KService::KService( KDesktopFile *config )
 : KSycocaEntry( config->filename())
{
  init(config);
}

void
KService::init( KDesktopFile *config )
{
  m_bValid = true;

  bool absPath = (entryPath()[0] == '/');

  config->setDesktopGroup();
  config->setDollarExpansion( true ); // mainly for Exec and Path
  if(absPath && access(QFile::encodeName(entryPath()), R_OK))
  {
    m_bValid = false;
    return;
  }
  QMap<QString, QString> entryMap = config->entryMap(config->group());
  
  entryMap.remove("Encoding"); // reserved as part of Desktop Entry Standard
  entryMap.remove("Version");  // reserved as part of Desktop Entry Standard
  
  m_bDeleted = config->readBoolEntry( "Hidden", false );
  entryMap.remove("Hidden");
  if (m_bDeleted)
  {
    m_bValid = false;
    return;
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

  QString name = entryPath();
  int pos = name.findRev('/');
  if (pos != -1)
     name = name.mid(pos+1);
  pos = name.find('.');
  if (pos != -1)
     name = name.left(pos);

  m_strExec = config->readEntry( "Exec" );
  entryMap.remove("Exec");
  m_strName = config->readEntry( "Name" );
  //kdDebug() << "parsing " << entryPath() << " Name=" << m_strName << endl;
  entryMap.remove("Name");
  if ( m_strName.isEmpty() )
  {
    m_bValid = false;
    return;
  }

  m_strIcon = config->readEntry( "Icon", "unknown" );
  entryMap.remove("Icon");
  m_bTerminal = (config->readBoolEntry( "Terminal" )); // should be a property IMHO
  entryMap.remove("Terminal");
  m_strTerminalOptions = config->readEntry( "TerminalOptions" ); // should be a property IMHO
  entryMap.remove("TerminalOptions");
  m_strPath = config->readEntry( "Path" );
  entryMap.remove("Path");
  m_strComment = config->readEntry( "Comment" );
  entryMap.remove("Comment");
  m_strGenName = config->readEntry( "GenericName" );
  entryMap.remove("GenericName");
  m_lstKeywords = config->readListEntry("Keywords");
  entryMap.remove("Keywords");
  m_strLibrary = config->readEntry( "X-KDE-Library" );
  entryMap.remove("X-KDE-Library");
  m_strInit = config->readEntry("X-KDE-Init" );
  entryMap.remove("X-KDE-Init");

  m_lstServiceTypes = config->readListEntry( "ServiceTypes" );
  entryMap.remove("ServiceTypes");
  // For compatibility with KDE 1.x
  m_lstServiceTypes += config->readListEntry( "MimeType", ';' );
  entryMap.remove("MimeType");

  if ( m_strType == "Application" && !m_lstServiceTypes.contains("Application") )
    // Applications implement the service type "Application" ;-)
    m_lstServiceTypes += "Application";

  QString dcopServiceType = config->readEntry("X-DCOP-ServiceType").lower();
  entryMap.remove("X-DCOP-ServiceType");
  if (dcopServiceType == "unique")
     m_DCOPServiceType = DCOP_Unique;
  else if (dcopServiceType == "multi")
     m_DCOPServiceType = DCOP_Multi;
  else if (dcopServiceType == "wait")
     m_DCOPServiceType = DCOP_Wait;
  else
     m_DCOPServiceType = DCOP_None;

  m_strDesktopEntryName = name.lower();

  m_bAllowAsDefault = config->readBoolEntry( "AllowDefault", true );
  entryMap.remove("AllowDefault");

  m_initialPreference = config->readNumEntry( "InitialPreference", 1 );
  entryMap.remove("InitialPreference");

  // Store all additional entries in the property map.
  // A QMap<QString,QString> would be easier for this but we can't
  // brake BC, so we have to store it in m_mapProps.
//  qWarning("Path = %s", entryPath().latin1());
  QMap<QString,QString>::ConstIterator it = entryMap.begin();
  for( ; it != entryMap.end();++it)
  {
//     qWarning("   Key = %s Data = %s", it.key().latin1(), it.data().latin1());
     m_mapProps.insert( it.key(), QVariant( it.data()));
  }
}

KService::KService( QDataStream& _str, int offset ) : KSycocaEntry( _str, offset )
{
  load( _str );
}

KService::~KService()
{
  //debug("KService::~KService()");
}

QPixmap KService::pixmap( KIcon::Group _group, int _force_size, int _state, QString * _path ) const
{
  return KGlobal::iconLoader()->loadIcon( m_strIcon, _group, _force_size, _state, _path );
}

void KService::load( QDataStream& s )
{
  // dummies are here because of fields that were removed, to keep bin compat.
  // Feel free to re-use, but fields for Applications only (not generic services)
  // should rather be added to application.desktop
  Q_INT8 def, term, dummy1, dummy2;
  Q_INT8 dst, initpref;
  QString dummyStr1, dummyStr2;
  int dummyI1, dummyI2;
  Q_UINT32 dummyUI32;

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
    >> m_lstKeywords >> m_strInit >> dummyUI32 >> m_strGenName;

  m_bAllowAsDefault = def;
  m_bTerminal = term;
  m_DCOPServiceType = (DCOPServiceType_t) dst;
  m_initialPreference = initpref;

  m_bValid = true;
}

void KService::save( QDataStream& s )
{
  KSycocaEntry::save( s );
  Q_INT8 def = m_bAllowAsDefault, initpref = m_initialPreference;
  Q_INT8 term = m_bTerminal;
  Q_INT8 dst = (Q_INT8) m_DCOPServiceType;
  Q_INT8 dummy1 = 0, dummy2 = 0; // see ::load
  QString dummyStr1, dummyStr2;
  int dummyI1 = 0, dummyI2 = 0;
  Q_UINT32 dummyUI32 = 0;

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
    << m_lstKeywords << m_strInit << dummyUI32 << m_strGenName;
}

bool KService::hasServiceType( const QString& _servicetype ) const
{
  if (!m_bValid) return false; // safety test

  //kdDebug(7012) << "Testing " << m_strDesktopEntryName << " for " << _servicetype << endl;

  // For each service type we are associated with, if it doesn't
  // match then we try its parent service types.
  QStringList::ConstIterator it = m_lstServiceTypes.begin();
  for( ; it != m_lstServiceTypes.end(); ++it )
  {
      //kdDebug(7012) << "    has " << (*it) << endl;
      QString servicetype( *it );
      bool found = true; // we are optimistic to start with :)
      while ( servicetype != _servicetype )
      {
          KServiceType::Ptr ptr = KServiceType::serviceType( servicetype );
          if (!ptr || !ptr->isDerived() ) { found = false; break; }
          servicetype = ptr->parentServiceType();
          //kdDebug(7012) << "up to " << servicetype << endl;
      }
      if (found)
          return true;
  }
  return false;
}

class KServiceReadProperty : public KConfigBase
{
public:
   KServiceReadProperty(const QString &_key, const QCString &_value)
	: key(_key), value(_value) { }

   bool internalHasGroup(const QCString &) const { /*qDebug("hasGroup(const QCString &)");*/ return false; }

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
   QCString value;
};

QVariant KService::property( const QString& _name ) const
{
  if ( _name == "Type" )
    return QVariant( m_strType );
  else if ( _name == "Name" )
    return QVariant( m_strName );
  else if ( _name == "Exec" )
    return QVariant( m_strExec );
  else if ( _name == "Icon" )
    return QVariant( m_strIcon );
  else if ( _name == "Terminal" )
    return QVariant( static_cast<int>(m_bTerminal) );
  else if ( _name == "TerminalOptions" )
    return QVariant( m_strTerminalOptions );
  else if ( _name == "Path" )
    return QVariant( m_strPath );
  else if ( _name == "Comment" )
    return QVariant( m_strComment );
  else if ( _name == "GenericName" )
    return QVariant( m_strGenName );
  else if ( _name == "ServiceTypes" )
    return QVariant( m_lstServiceTypes );
  else if ( _name == "AllowAsDefault" )
    return QVariant( static_cast<int>(m_bAllowAsDefault) );
  else if ( _name == "InitialPreference" )
    return QVariant( m_initialPreference );
  else if ( _name == "Library" )
    return QVariant( m_strLibrary );
  else if ( _name == "DesktopEntryPath" )
    return QVariant( entryPath() );
  else if ( _name == "DesktopEntryName")
    return QVariant( m_strDesktopEntryName );

  // Ok we need to convert the property from a QString to its real type.
  // First we need to ask KServiceTypeFactory what the type of this property
  // is supposed to be.
  // Then we use a homebuild class based on KBaseConfig to convert the QString.
  // For some often used property types we do the conversion ourselves.

  QVariant::Type t = KServiceTypeFactory::self()->findPropertyTypeByName(_name);
  if (t == QVariant::Invalid)
  {
    kdDebug(7012) << "Request for unknown property '" << _name << "'\n";
    return QVariant(); // Unknown property: Invalid variant.
  }

  QMap<QString,QVariant>::ConstIterator it = m_mapProps.find( _name );
  if ( (it == m_mapProps.end()) || (!it.data().isValid()))
  {
     //kdDebug(7012) << "Property not found " << _name << endl;
     return QVariant(); // No property set.
  }

  switch(t)
  {
    case QVariant::String:
        return it.data();
    case QVariant::Bool:
    case QVariant::Int:
        {
           QString aValue = it.data().toString();
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
              if (val)
                 return QVariant(true);
              else
                 return QVariant(false);
           }
           return QVariant(val);
        }
    default:
        // All others
        KServiceReadProperty ksrp(_name, it.data().toString().utf8());
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

  return res;
}

KService::List KService::allServices()
{
  return KServiceFactory::self()->allServices();
}

KService::Ptr KService::serviceByName( const QString& _name )
{
  KService * s = KServiceFactory::self()->findServiceByName( _name );
  return KService::Ptr( s );
}

KService::Ptr KService::serviceByDesktopPath( const QString& _name )
{
  KService * s = KServiceFactory::self()->findServiceByDesktopPath( _name );
  return KService::Ptr( s );
}

KService::Ptr KService::serviceByDesktopName( const QString& _name )
{
  KService * s = KServiceFactory::self()->findServiceByDesktopName( _name );
  return KService::Ptr( s );
}

KService::List KService::allInitServices()
{
  return KServiceFactory::self()->allInitServices();
}

bool KService::substituteUid() const {
  QVariant v = property("X-KDE-SubstituteUID");
  return v.isValid() && v.toBool();
}

QString KService::username() const {
  // See also KDesktopFile::tryExec()
  QString user;
  QVariant v = property("X-KDE-Username");
  user = v.isValid() ? v.toString() : QString::null;
  if (user.isEmpty())
     user = ::getenv("ADMIN_ACCOUNT");
  if (user.isEmpty())
     user = "root";
  return user;
}

bool KService::noDisplay() const {
  QMap<QString,QVariant>::ConstIterator it = m_mapProps.find( "NoDisplay" );
  if ( (it == m_mapProps.end()) || (!it.data().isValid()))
  {
     return false;
  }

  QString aValue = it.data().toString();
  if (aValue == "true" || aValue == "on" || aValue == "yes")
     return true;
  else
     return false;
}

bool KService::allowMultipleFiles() const {
  // Can we pass multiple files on the command line or do we have to start the application for every single file ?
  if ( m_strExec.find( "%F" ) != -1 || m_strExec.find( "%U" ) != -1 ||
       m_strExec.find( "%N" ) != -1 || m_strExec.find( "%D" ) != -1 )
    return true;
  else
    return false;
}

void KService::virtual_hook( int id, void* data )
{ KSycocaEntry::virtual_hook( id, data ); }

