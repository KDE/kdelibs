/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __kservices_h__
#define __kservices_h__

#include <qlist.h>
#include <qstringlist.h>
#include <qstring.h>
#include <qshared.h>
#include <qmap.h>
#include <qdatastream.h>
#include <qvariant.h>

#include <ksimpleconfig.h>
#include <ksharedptr.h>

#include "ktypecode.h"

/**
 * Represents a service, i.e. an application bound to one or several mimetypes
 * as written in its desktop entry file.
 *
 * To use the public static methods of this class, you must do 
 * the following registry initialisation (in main() for instance)
 * <pre>
 * #include <kregistry.h>
 * #include <kregfactories.h> 
 *
 * KRegistry::self()->addFactory( new KServiceTypeFactory );
 * KRegistry::self()->addFactory( new KServiceFactory );
 * KRegistry::self()->load();
 * </pre>
 *
 * The ServiceTypeFactory is more or less mandatory, but if you don't want to
 * keep all the services in your application's memory (and you probably don't),
 * see KTraderServiceProvider in corba/kded/ktrader.h
 */
class KService : public KShared
{
  K_TYPECODE( TC_KService )

public:  
  typedef KSharedPtr<KService> Ptr;
  typedef const QSharedPtr<QVariant> PropertyPtr;

  /*
   * Constructor.  You may pass in arguments to create a service with
   * specific properties, otherwise a null service will be constructed.
   *
   * @param _put_in_list will add the service to the list of known
   *        services. But sometimes you may just want to create
   *        a service object for internal purposes.
   */
  KService( const QString& _name, const QString& _exec, const QString &_corbaexec, 
            const QString& _icon, const QStringList& _lstServiceTypes, 
	    const QString& _comment = QString::null, bool _allow_as_default = true, 
	    const QString& _path = QString::null, const QString& _terminal = QString::null, 
	    const QString& _file = QString::null, const QString& _act_mode = QString::null, 
	    const QStringList& _repoids = QStringList(), bool _put_in_list = true );
  KService( bool _put_in_list = true );
  KService( KSimpleConfig& _cfg, bool _put_in_list = true );
  KService( QDataStream& _str, bool _put_in_list = true );

  virtual ~KService();
  
  /**
   * @return the name of the service. 
   */
  QString name() const { return m_strName; }
  /** 
   * @return the command that the service executes. 
   */
  QString exec() const { return m_strExec; }
  /** 
   * @return the command that the CORBA based service executes.
   *         (usually something like "myapp --server" )
   */
  QString CORBAExec() const { return m_strCORBAExec; }
  /**
   * @return the icon associated with the service. 
   */
  QString icon() const { return m_strIcon; }
  /** 
   * @return any options associated with the terminal the service
   * runs in, if it requires a terminal.  The service must be a
   * tty-oriented program).
   */
  QString terminalOptions() const { return m_strTerminalOptions; }
  /** 
   * @return the path to the location where the service desktop entry 
   * is stored. 
   */
  QString path() const { return m_strPath; }
  /**
   * @return the descriptive comment for the service, if there is one. 
   */
  QString comment() const { return m_strComment; }
  /**
   * @return the CORBA activation mode for the service, if there is one.
   *         (to be used for the IMR)
   */
  QString activationMode() const { return m_strActivationMode; }
  /**
   * @return the CORBA Repository IDs for the service, if there are any.
   */
  QStringList repoIds() const { return m_lstRepoIds; }

  /**
   * @return the filename of the desktop entry file responsible for
   *         these services.
   */
  // QString file() const { return m_strFile; };
  QStringList serviceTypes() const { return m_lstServiceTypes; }
  /** 
   * @param _service is the name of the service type you are
   *        interested in determining whether this services supports.
   *        
   * @return TRUE if the service you specified is supported,
   *        otherwise FALSE.  */
  bool hasServiceType( const QString& _service ) const;
  /**
   * @return TRUE if the service may be used as a default setting, for
   *         example in a file manager. Usually that is the case, but
   *         some services may only be started when the user selected
   *         them. This kind of services returns FALSE here.
   */
  bool allowAsDefault() const { return m_bAllowAsDefault; }

  virtual PropertyPtr property( const QString& _name ) const;
  virtual QStringList propertyNames() const;

  bool isValid() const { return m_bValid; }

  virtual void load( QDataStream& );
  virtual void save( QDataStream& ) const;
  
  /**
   * @return a pointer to the requested service or 0 if the service is
   *         unknown.
   */
  static KService* service( const QString& _name );
  
  /**
   * @return the whole list of services. Useful to display them.
   */
  static KService* parseService( const QString& _file, KSimpleConfig &config,
				 bool _put_in_list = true );

  /**
   * @return the whole list of services. Useful for being able to
   * to display them in a list box, for example.
   * Creates a KServiceFactory if it currently knows
   * nothing about services.
   */
  static const QList<KService>& services();

private:
  static void initStatic();

  QString m_strName;
  QString m_strExec;
  QString m_strCORBAExec;
  QString m_strIcon;
  QString m_strTerminalOptions;
  QString m_strPath;
  QString m_strComment;
  QString m_strActivationMode;
  QStringList m_lstRepoIds;
  // QString m_strFile;
  QStringList m_lstServiceTypes;
  bool m_bAllowAsDefault;
  QMap<QString,QVariant> m_mapProps;
  bool m_bValid;
  
  static QList<KService>* s_lstServices;
};

QDataStream& operator>>( QDataStream& _str, KService& s );
QDataStream& operator<<( QDataStream& _str, const KService& s );

#endif
