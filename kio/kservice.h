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
#include <qstring.h>
#include <qmap.h>
#include <qdatastream.h>
#include <qvariant.h>

#include "ksycocaentry.h"

class KDesktopFile;
class KService;
/**
 * Represent a service, i.e. an application bound to one or several mimetypes
 * (or servicetypes) as written in its desktop entry file.
 *
 * A service may be a library, too.
 * The starting point you need is often the static methods.
 *
 * @author Torben Weis <weis@kde.org>
 */
class KService : public KSycocaEntry
{
  K_SYCOCATYPE( KST_KService, KSycocaEntry )

public:
  typedef KSharedPtr<KService> Ptr;
  typedef QValueList<Ptr> List;
public:

  /**
   * Construct a service and take all information from a config file.
   *
   * @param _fullpath Full path to the config file.
   * @param _relpath Relative path wrt to $KDEDIRS/+resource.
   */
  KService( const QString & _fullpath );

  /**
   * Construct a service and take all information from a desktop file.
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
   * Returns the type of the service ("Application" or "Service").
   */
  virtual QString type() const { return m_strType; }
  /**
   * Returns the name of the service.
   */
  virtual QString name() const { return m_strName; }
  /**
   * Returns the command that the service executes.
   */
  QString exec() const { return m_strExec; }
  /**
   * Returns the name of the library that contains the services
   *         implementation.
   */
  QString library() const { return m_strLibrary; }
  /**
   * Returns the major number of the library.
   *
   * @see library()
   * @see libraryMinor()
   */
  int libraryMajor() const { return m_libraryMajor; }
  /**
   * Returns the minor number of the library.
   *
   * @see library()
   * @see libraryMajor()
   */
  int libraryMinor() const { return m_libraryMinor; }
  /**
   * Returns the libraries on which this service depends. That is
   *         only of interest if the service itelf is a library.
   */
  QStringList libraryDependencies() const { return m_lstLibraryDeps; }
  /**
   * Returns the icon associated with the service.
   */
  QString icon() const { return m_strIcon; }
  /**
   * Returns a pixmap for this service (finds and loads icon())
   */
  QPixmap pixmap( int _group, int _force_size = 0, int _state = 0,
                  QString * _path = 0L ) const;
  /**
   * Returns @tt true is the service is run in a terminal.
   */
  bool terminal() const { return m_bTerminal; }
  /**
   * Returns any options associated with the terminal the service
   * runs in, if it requires a terminal.
   *
   * The service must be a tty-oriented program.
   */
  QString terminalOptions() const { return m_strTerminalOptions; }
  /**
   * Returns @tt true if the service has to be run under a different uid.
   */
  bool substituteUid() const;
  /**
   * Returns the username under which the service has to be run.
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
   */
  QString desktopEntryPath() const { return entryPath(); }

  /**
   * Returns the filename of the service desktop entry without any
   * extension. E.g. "kppp"
   */
  QString desktopEntryName() const { return m_strDesktopEntryName; }

  /**
   * @li None - This service has no DCOP support
   * @li Unique - This service provides a unique DCOP service.
   *              The service name is equal to the desktopEntryName.
   * @li Multi - This service provides a DCOP service which can be run
   *             with multiple instances in parallel. The service name of
   *             an instance is equal to the desktopEntryName + "-" +
   *             the PID of the process.
   */
  enum DCOPServiceType_t { DCOP_None = 0, DCOP_Unique, DCOP_Multi };

  /**
   * Returns The @ref DCOPServiceType supported by this service.
   */
  DCOPServiceType_t DCOPServiceType() const { return m_DCOPServiceType; }

  /**
   * Returns the working directory to run the program in.
   */
  QString path() const { return m_strPath; }

  /**
   * Returns the descriptive comment for the service, if there is one.
   */
  QString comment() const { return m_strComment; }

  /**
   * Returns a list of descriptive keywords the service, if there are any.
   */
  QStringList keywords() const { return m_lstKeywords; }

  /**
   * Returns the service types that this service supports.
   */
  QStringList serviceTypes() const { return m_lstServiceTypes; }

  /**
   * @param _service The name of the service type you are
   *        interested in determining whether this services supports.
   *
   * @return @tt true if the service you specified is supported,
   *        otherwise @tt false.
   */
  bool hasServiceType( const QString& _service ) const;
  /**
   * Returns @tt true if the service may be used as a default setting, for
   *         example in a file manager. Usually that is the case, but
   *         some services may only be started when the user selected
   *         them. This kind of services returns @tt false here.
   */
  bool allowAsDefault() const { return m_bAllowAsDefault; }

  /**
   * What preference to associate with this service initially (before
   * the user has had any chance to define a profile for it)
   * The bigger the value, the most preferred the service is.
   */
  int initialPreference() const { return m_initialPreference; }

  /**
   * Returns the requested properties. Some often used properties
   *         have convenience access functions like @ref exec(),
   *         @ref serviceTypes etc.
   *
   *         It depends upon the @ref serviceTypes() of this service which
   *         properties a service can have.
   *
   * @see KServiceType
   */
  virtual QVariant property( const QString& _name ) const;
  /**
   * Returns the list of all properties that this service can have.
   *         That means, that some properties may be empty.
   */
  virtual QStringList propertyNames() const;

  /**
   * Returns @tt true if the service is valid (e.g. name is not empty)
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
   * Returns a pointer to the requested service or 0 if the service is
   *         unknown.
   * @em Very @em important: Don't store the result in a @ref KService * !
   */
  static Ptr serviceByName( const QString& _name );

  /**
   * Returns a pointer to the requested service or 0 if the service is
   *         unknown.
   * @em Very @em important: Don't store the result in a @ref KService * !
   */
  static Ptr serviceByDesktopPath( const QString& _name );

  /**
   * Returns a pointer to the requested service or 0 if the service is
   *         unknown.
   * @em Very @em important: Don't store the result in a @ref KService * !
   */
  static Ptr serviceByDesktopName( const QString& _name );

  /**
   * Returns the whole list of services.
   *
   *  Useful for being able to
   * to display them in a list box, for example.
   * More memory consuming than the ones above, don't use unless
   * really necessary.
   */
  static List allServices();

protected:

  void init(KDesktopFile *config);

private:
  QString m_strType;
  QString m_strName;
  QString m_strExec;
  QString m_strIcon;
  QString m_strTerminalOptions;
  QString m_strPath;
  QString m_strComment;
  QString m_strLibrary;
  int m_libraryMajor;
  int m_libraryMinor;
  QStringList m_lstLibraryDeps;
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
};
#endif
