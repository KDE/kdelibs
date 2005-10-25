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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __kservices_h__
#define __kservices_h__

#include <qstringlist.h>
#include <qlist.h>
#include <qmap.h>
#include <qvariant.h>
#include <kicontheme.h>

#include "ksycocaentry.h"

class QDataStream;
class KDesktopFile;
class KService;
class KBuildSycoca;
class QWidget;

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
class KIO_EXPORT KService : public KSycocaEntry
{
  K_SYCOCATYPE( KST_KService, KSycocaEntry )

  friend class KBuildSycoca;

public:
  typedef KSharedPtr<KService> Ptr;
  typedef QList<Ptr> List;
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
  explicit KService( const QString & _fullpath );

  /**
   * Construct a service and take all information from a desktop file.
   * @param config the desktop file to read
   */
  KService( KDesktopFile *config ); // KDE-4.0: make explicit

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
   * Returns the menu ID of the service desktop entry.
   * The menu ID is used to add or remove the entry to a menu.
   * @return the menu ID
   * @since 3.2
   */
  QString menuId() const;

  /**
   * Returns a normalized ID suitable for storing in configuration files.
   * It will be based on the menu-id when available and otherwise falls
   * back to desktopEntryPath()
   * @return the storage ID
   * @since 3.2
   */
  QString storageId() const;

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
   * Returns the DCOPServiceType supported by this service.
   * @return the DCOPServiceType supported by this service
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
   * Returns the untranslated (US English) generic name
   * for the service, if there is one
   * (e.g. "Mail Client").
   * @return the generic name,
   *         or QString::null if not set
   * @since 3.2
   */
  QString untranslatedGenericName() const;

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
   * What preference to associate with this service initially
   * for handling the specified mimetype. (before the user has
   * had any chance to define a profile for it).
   * The bigger the value, the most preferred the service is.
   * @return the service preference level of the service for
   * this mimetype
   */
  int initialPreferenceForMimeType( const QString& mimeType ) const;

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
   * have convenience access functions like exec(),
   * serviceTypes etc.
   *
   * It depends upon the serviceTypes() of this service which
   * properties a service can have.
   *
   * @param _name the name of the property
   * @return the property, or invalid if not found
   * @see KServiceType
   */
  virtual QVariant property( const QString& _name ) const;

  /**
   * Returns the requested property.
   *
   * @param _name the name of the property
   * @param t the assumed type of the property
   * @return the property, or invalid if not found
   * @see KServiceType
   * @since 3.2
   */
  QVariant property( const QString& _name, QVariant::Type t ) const;

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
   * Returns a path that can be used for saving changes to this
   * service
   * @return path that can be used for saving changes to this service
   * @since 3.2
   */
  QString locateLocal();

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
   * @internal
   * Set the menu id
   */
  void setMenuId(const QString &menuId);
  /**
   * @internal
   * Sets whether to use a terminal or not
   */
  void setTerminal(bool b) { m_bTerminal = b; }
  /**
   * @internal
   * Sets the terminal options to use
   */
  void setTerminalOptions(const QString &options) { m_strTerminalOptions = options; }

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
   * Find a service based on its path as returned by desktopEntryPath().
   * It's usually better to use serviceByStorageId() instead.
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
   * Find a service by its menu-id
   *
   * @param _menuId the menu id of the service
   * @return a pointer to the requested service or 0 if the service is
   *         unknown.
   * @em Very @em important: Don't store the result in a KService* !
   * @since 3.2
   */
  static Ptr serviceByMenuId( const QString& _menuId );

  /**
   * Find a service by its storage-id or desktop-file path. This
   * function will try very hard to find a matching service.
   *
   * @param _storageId the storage id or desktop-file path of the service
   * @return a pointer to the requested service or 0 if the service is
   *         unknown.
   * @em Very @em important: Don't store the result in a KService* !
   * @since 3.2
   */
  static Ptr serviceByStorageId( const QString& _storageId );

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

  /**
   * Returns a path that can be used to create a new KService based
   * on @p suggestedName.
   * @param showInMenu true, if the service should be shown in the KDE menu
   *        false, if the service should be hidden from the menu
   * @param suggestedName name to base the file on, if a service with such
   *        name already exists, a prefix will be added to make it unique.
   * @param menuId If provided, menuId will be set to the menu id to use for
   *        the KService
   * @param reservedMenuIds If provided, the path and menu id will be chosen
   *        in such a way that the new menu id does not conflict with any
   *        of the reservedMenuIds
   * @return The path to use for the new KService.
   * @since 3.2
   */
  static QString newServicePath(bool showInMenu, const QString &suggestedName,
                                QString *menuId = 0,
                                const QStringList *reservedMenuIds = 0);


  /**
   * Rebuild KSycoca and show a progress dialog while doing so.
   * @param parent Parent widget for the progress dialog
   * @since 3.2
   */
  static void rebuildKSycoca(QWidget *parent);

protected:

  void init(KDesktopFile *config);

  QStringList &accessServiceTypes() { return m_lstServiceTypes; }


private:
  KService( const KService& ); // forbidden
  KService& operator=(const KService&);

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
  QString m_strDesktopEntryName;
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
