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

#include <qstringlist.h>
#include <qmap.h>
#include <qvariant.h>
#include <kicontheme.h>

#include "ksycocaentry.h"

class QDataStream;
class KDesktopFile;
class KService;
class KBuildSycoca;
/**
 * Represent a service, i.e. an application bound to one or several mimetypes
 * (or servicetypes) as written in its desktop entry file.
 *
 * A service may be a library, too.
 * The starting point you need is often the static methods.
 * Service types are stored as desktop files in the "service" resource..
 *
 * @see KServiceType
 * @see KServiceGroup
 * @author Torben Weis <weis@kde.org>
 */
class KService : public KSycocaEntry
{
  K_SYCOCATYPE( KST_KService, KSycocaEntry )

  friend class KBuildSycoca;

public:
  typedef KSharedPtr<KService> Ptr;
  typedef QValueList<Ptr> List;
public:
  /**
   * Construct a temporary service with a given name, exec-line and icon.
   * @param _name the name of the service
   * @param _exec the executable 
   * @param _icon the name of the icon
   */
  KService( const QString & _name, const QString &_exec, const QString &_icon);

  /**
   * Construct a service and take all information from a config file.
   *
   * @param _fullpath Full path to the config file.
   */
  KService( const QString & _fullpath );

  /**
   * Construct a service and take all information from a desktop file.
   * @param config the desktop file to read
   */
  KService( KDesktopFile *config );

  /**
   * @internal
   * Construct a service from a stream.
   * The stream must already be positionned at the correct offset.
   */
  KService( QDataStream& _str, int offset );

  virtual ~KService();

  /**
   * Returns the type of the service.
   * @return the type of the service ("Application" or "Service")
   */
  virtual QString type() const { return m_strType; }
  /**
   * Returns the name of the service.
   * @return the name of the service,
   *         or QString::null if not set
   */
  virtual QString name() const { return m_strName; }
  /**
   * Returns the executable.
   * @return the command that the service executes,
   *         or QString::null if not set
   */
  QString exec() const { return m_strExec; }
  /**
   * Returns the name of the service's library.
   * @return the name of the library that contains the services
   *         implementation,
   *         or QString::null if not set
   */
  QString library() const { return m_strLibrary; }
  /**
   * Returns the name of the init function to call (KControl modules).
   * @return the name of the init function to call in this service
   *         during startup of KDE. (KControl modules only),
   *         or QString::null if not set
   */
  QString init() const { return m_strInit; }

  /**
   * Returns the name of the icon.
   * @return the icon associated with the service,
   *         or QString::null if not set
   */
  QString icon() const { return m_strIcon; }
  /**
   * Returns the pixmap that represents the icon.
   * @return a pixmap for this service (finds and loads icon()),
   *         null if not set
   * @see icon()
   */
  QPixmap pixmap( KIcon::Group _group, int _force_size = 0, int _state = 0,
                  QString * _path = 0L ) const;
  /**
   * Checks whethe the service should be run in a terminal.
   * @return true if the service is to be run in a terminal.
   */
  bool terminal() const { return m_bTerminal; }
  /**
   * Returns any options associated with the terminal the service
   * runs in, if it requires a terminal.
   *
   * The service must be a tty-oriented program.
   * @return the terminal options,
   *         or QString::null if not set
   */
  QString terminalOptions() const { return m_strTerminalOptions; }
  /**
   * Checks whether the service runs with a different user id.
   * @return true if the service has to be run under a different uid.
   * @see username()
   */
  bool substituteUid() const;
  /**
   * Returns the user name, if the service runs with a 
   * different user id.
   * @return the username under which the service has to be run,
   *         or QString::null if not set
   * @see substututeUid()a
   */
  QString username() const;

  /**
   * Returns the path to the location where the service desktop entry
   * is stored.
   *
   * This is a relative path if the desktop entry was found in any
   * of the locations pointed to by $KDEDIRS (e.g. "Internet/kppp.desktop")
   * It is a full path if the desktop entry originates from another
   * location.
   * @return the path of the service's desktop file,
   *         or QString::null if not set
   */
  QString desktopEntryPath() const { return entryPath(); }

  /**
   * Returns the filename of the service desktop entry without any
   * extension. E.g. "kppp"
   * @return the name of the desktop entry without path or extension,
   *         or QString::null if not set
   */
  QString desktopEntryName() const { return m_strDesktopEntryName; }

  /**
   * Describes the DCOP type of the service.
   * @li None - This service has no DCOP support
   * @li Unique - This service provides a unique DCOP service.
   *              The service name is equal to the desktopEntryName.
   * @li Multi - This service provides a DCOP service which can be run
   *             with multiple instances in parallel. The service name of
   *             an instance is equal to the desktopEntryName + "-" +
   *             the PID of the process.
   * @li Wait - This service has no DCOP support, the launcher will wait
   *            till it is finished.
   */
  enum DCOPServiceType_t { DCOP_None = 0, DCOP_Unique, DCOP_Multi, DCOP_Wait };

  /**
   * Returns the @ref DCOPServiceType supported by this service.
   * @return the @ref DCOPServiceType supported by this service
   */
  DCOPServiceType_t DCOPServiceType() const { return m_DCOPServiceType; }

  /**
   * Returns the working directory to run the program in.
   * @return the working directory to run the program in,
   *         or QString::null if not set
   */
  QString path() const { return m_strPath; }

  /**
   * Returns the descriptive comment for the service, if there is one.
   * @return the descriptive comment for the service, or QString::null
   *         if not set
   */
  QString comment() const { return m_strComment; }

  /**
   * Returns the generic name for the service, if there is one
   * (e.g. "Mail Client").
   * @return the generic name, 
   *         or QString::null if not set
   */
  QString genericName() const { return m_strGenName; }

  /**
   * Returns a list of descriptive keywords the service, if there are any.
   * @return the list of keywords
   */
  QStringList keywords() const { return m_lstKeywords; }

  /**
   * Returns a list of VFolder categories.
   * @return the list of VFolder categories
   * @since 3.1
   */
  QStringList categories() const;

  /**
   * Returns the service types that this service supports.
   * @return the list of service types that are supported
   */
  QStringList serviceTypes() const { return m_lstServiceTypes; }

  /**
   * Checks whether the service supports this service type
   * @param _service The name of the service type you are
   *        interested in determining whether this services supports.
   *
   * @return true if the service you specified is supported,
   *        otherwise false.
   */
  bool hasServiceType( const QString& _service ) const;
  /**
   * Checks whether a service is used as a default setting, for
   *         example as plugin in a file manager. Usually that is the 
   *         case, but some services may only be started when the user 
   *         selected them. This kind of services returns false here.
   * @return true if the service may be used as a default setting
   */
  bool allowAsDefault() const { return m_bAllowAsDefault; }

  /**
   * Checks whether this service can handle several files as
   * startup arguments.
   * @return true if multiple files may be passed to this service at
   * startup. False if only one file at a time may be passed.
   */
  bool allowMultipleFiles() const;

  /**
   * What preference to associate with this service initially (before
   * the user has had any chance to define a profile for it).
   * The bigger the value, the most preferred the service is.
   * @return the service preference level of the service
   */
  int initialPreference() const { return m_initialPreference; }

  /**
   * @internal. Allows KServiceType::offers to tweak the initial preference.
   */
  void setInitialPreference( int i ) { m_initialPreference = i; }

  /**
   * Whether the entry should be suppressed in menus.
   * @return true to suppress this service
   */
  bool noDisplay() const;

  /**
   * Name of the application this service belongs to.
   * (Useful for e.g. plugins)
   * @return the parent application, or QString::null if not set
   * @since 3.1
   */
  QString parentApp() const;

  /**
   * Returns the requested property. Some often used properties
   * have convenience access functions like @ref exec(),
   * @ref serviceTypes etc.
   *
   * It depends upon the @ref serviceTypes() of this service which
   * properties a service can have.
   * 
   * @param _name the name of the property
   * @return the property, or invalid if not found
   * @see KServiceType
   */
  virtual QVariant property( const QString& _name ) const;
  /**
   * Returns the list of all properties that this service can have.
   * That means, that some of these properties may be empty.
   * @return the list of supported properties
   */
  virtual QStringList propertyNames() const;

  /**
   * Checks whether the service is valid.
   * @return true if the service is valid (e.g. name is not empty)
   */
  bool isValid() const { return m_bValid; }

  /**
   * @internal
   * Load the service from a stream.
   */
  virtual void load( QDataStream& );
  /**
   * @internal
   * Save the service to a stream.
   */
  virtual void save( QDataStream& );

  /**
   * Find a service by name, i.e. the translated Name field. You should
   * really not use this method, since the name is translated.
   *
   * @param _name the name to search
   * @return a pointer to the requested service or 0 if the service is
   *         unknown.
   * @em Very @em important: Don't store the result in a KService* !
   */
  static Ptr serviceByName( const QString& _name );

  /**
   * Find a service by its relative path to the applnk or services
   * directory, for instance "Internet/konqbrowser.desktop". Better not use
   * it for menu entries though, since the user can move them.
   * It's ok to use it for services though (e.g. "http_cache_cleaner.desktop")
   *
   * @param _path the path of the configuration file
   * @return a pointer to the requested service or 0 if the service is
   *         unknown.
   * @em Very @em important: Don't store the result in a KService* !
   */
  static Ptr serviceByDesktopPath( const QString& _path );

  /**
   * Find a service by the name of its desktop file, not depending on
   * its actual location (as long as it's under the applnk or service
   * directories). For instance "konqbrowser" or "kcookiejar". Note that
   * the ".desktop" extension is implicit.
   *
   * This is the recommended method (safe even if the user moves stuff)
   * but note that it assumes that no two entries have the same filename.
   *
   * @param _name the name of the configuration file
   * @return a pointer to the requested service or 0 if the service is
   *         unknown.
   * @em Very @em important: Don't store the result in a KService* !
   */
  static Ptr serviceByDesktopName( const QString& _name );

  /**
   * Returns the whole list of services.
   *
   *  Useful for being able to
   * to display them in a list box, for example.
   * More memory consuming than the ones above, don't use unless
   * really necessary.
   * @return the list of all services
   */
  static List allServices();

  /**
   * Returns all services that require initialisation.
   *
   * Only needed by "kcminit"
   * @return the list of all services that need to be initialized
   */
  static List allInitServices();

protected:

  void init(KDesktopFile *config);

  QStringList &accessServiceTypes() { return m_lstServiceTypes; }

private:
  QString m_strType;
  QString m_strName;
  QString m_strExec;
  QString m_strIcon;
  QString m_strTerminalOptions;
  QString m_strPath;
  QString m_strComment;
  QString m_strLibrary;
  QStringList m_lstServiceTypes;
  bool m_bAllowAsDefault;
  int m_initialPreference;
  bool m_bTerminal;
  //bool m_bSuid;
  //QString m_strUsername;
  QString m_strDesktopEntryName;
  //QString m_docPath;
  //bool m_bHideFromPanel;
  DCOPServiceType_t m_DCOPServiceType;
  QMap<QString,QVariant> m_mapProps;
  bool m_bValid;
  QStringList m_lstKeywords;
  QString m_strInit;
  QString m_strGenName;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KServicePrivate;
  KServicePrivate* d;
};
#endif
