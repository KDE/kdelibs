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
 * Represents a service, i.e. an application bound to one or several mimetypes
 * (or servicetypes) as written in its desktop entry file.
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
public: // KDoc seems to barf on those typedefs and generates no docs after them

  /**
   * Construct a service and take all informations from a config file
   * @param _fullpath full path to the config file
   * @param _relpath relative path wrt to $KDEDIRS/+resource
   */
  KService( const QString & _fullpath );

  /**
   * Construct a service and take all informations from a desktop file
   */
  KService( KDesktopFile *config );

  /**
   * @internal construct a service from a stream.
   * The stream must already be positionned at the correct offset
   */
  KService( QDataStream& _str, int offset );

  virtual ~KService();

  /**
   * @return the type of the service ("Application" or "Service").
   */
  virtual QString type() const { return m_strType; }
  /**
   * @return the name of the service.
   */
  virtual QString name() const { return m_strName; }
  /**
   * @return the command that the service executes.
   */
  QString exec() const { return m_strExec; }
  /**
   * @return the name of the library that contains the services
   *         implementation.
   */
  QString library() const { return m_strLibrary; }
  /**
   * @return the major number of the library.
   *
   * @see #library
   * @see #libraryMinor
   */
  int libraryMajor() const { return m_libraryMajor; }
  /**
   * @return the minor number of the library.
   *
   * @see #library
   * @see #libraryMajor
   */
  int libraryMinor() const { return m_libraryMinor; }
  /**
   * @return the libraries on which this service depends. That is
   *         only of interest if the service itelf is a library.
   */
  QStringList libraryDependencies() const { return m_lstLibraryDeps; }
  /**
   * @return the icon associated with the service.
   */
  QString icon() const { return m_strIcon; }
  /**
   * @return a pixmap for this service (finds and loads icon())
   */
  QPixmap pixmap( int _group, int _force_size = 0, int _state = 0,
                  QString * _path = 0L ) const;
  /**
   * @return Run the service in a terminal?
   */
  bool terminal() const { return m_bTerminal; }
  /**
   * @return any options associated with the terminal the service
   * runs in, if it requires a terminal.  The service must be a
   * tty-oriented program).
   */
  QString terminalOptions() const { return m_strTerminalOptions; }
  /**
   * @return If the service has to be run under a different uid.
   */
  bool substituteUid() const;
  /**
   * @return The username as which the service has to be run.
   */
  QString username() const;

  /**
   * @return the path to the location where the service desktop entry
   * is stored.
   *
   * This is a relative path if the desktop entry was found in any
   * of the locations pointed to by $KDEDIRS (e.g. "Internet/kppp.desktop")
   * It is a full path if the desktop entry originates from another
   * location.
   */
  QString desktopEntryPath() const { return m_strDesktopEntryPath; }

  /**
   * @return the filename of the service desktop entry without any
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
   * @return The DCOPServiceType supported by this service.
   */
  DCOPServiceType_t DCOPServiceType() const { return m_DCOPServiceType; }

  /**
   * @return the working directory to run the program in
   */
  QString path() const { return m_strPath; }
  /**
   * @return the descriptive comment for the service, if there is one.
   */
  QString comment() const { return m_strComment; }

  /**
   * @return the service types that this service supports
   */
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

  /**
   * What preference to associate with this service initially (before
   * the user has had any chance to define a profile for it)
   * The bigger the value, the most preferred the service is.
   */
  int initialPreference() const { return m_initialPreference; }

  /**
   * @return the requested properties. Some often used properties
   *         have convenience access functions like @ref #exec,
   *         @ref #serviceTypes etc.
   *
   *         It depends upon the @ref #serviceTypes of this service which
   *         properties a service can have.
   *
   * @see KServiceType
   */
  virtual QVariant property( const QString& _name ) const;
  /**
   * @return the list of all properties that this service can have.
   *         That means, that some properties may be empty.
   */
  virtual QStringList propertyNames() const;

  /**
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
   * @return a pointer to the requested service or 0 if the service is
   *         unknown.
   * VERY IMPORTANT : don't store the result in a KService * !
   */
  static Ptr serviceByName( const QString& _name );

  /**
   * @return a pointer to the requested service or 0 if the service is
   *         unknown.
   * VERY IMPORTANT : don't store the result in a KService * !
   */
  static Ptr serviceByDesktopPath( const QString& _name );

  /**
   * @return a pointer to the requested service or 0 if the service is
   *         unknown.
   * VERY IMPORTANT : don't store the result in a KService * !
   */
  static Ptr serviceByDesktopName( const QString& _name );

  /**
   * @return the whole list of services. Useful for being able to
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
  QString m_strDesktopEntryPath;
  QString m_strDesktopEntryName;
  //QString m_docPath;
  //bool m_bHideFromPanel;
  DCOPServiceType_t m_DCOPServiceType;
  QMap<QString,QVariant> m_mapProps;
  bool m_bValid;
};
#endif
