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

#include "ksycocaentry.h"
#include "ksycocatype.h"

class KService;
//class KServiceList;
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
  K_SYCOCATYPE( KST_KService, KSycocaEntry );

public:
  typedef KSharedPtr<KService> Ptr;
  typedef QValueList<Ptr> List;
  typedef const QSharedPtr<QVariant> PropertyPtr;

  /**
   * Constructor.  You may pass in arguments to create a service with
   * specific properties
   */
  KService( const QString& _name, const QString& _exec, const QString &_corbaexec,
            const QString& _icon, const QStringList& _lstServiceTypes,
	    const QString& _comment = QString::null, bool _allow_as_default = true,
	    const QString& _path = QString::null, const QString& _terminal = QString::null,
	    const QString& _file = QString::null, const QString& _act_mode = QString::null,
	    const QStringList& _repoids = QStringList(),
	    const QString& _lib = QString::null, int _minor = 0, int _major = 0, 
            const QStringList& _deps = QStringList());
  /**
   * Construct a service and take all informations from a @ref KSimpleConfig object.
   */
  KService( KSimpleConfig& _cfg );
  /**
   * @internal construct a service from a stream. 
   * The stream must already be positionned at the correct offset
   */
  KService( QDataStream& _str, int offset );

  virtual ~KService();

  /**
   * @return the name of the service.
   */
  virtual QString name() const { return m_strName; }
  /**
   * @return the command that the service executes.
   */
  QString exec() const { return m_strExec; }
  /**
   * @return the command that the CORBA based service executes.
   *         (usually something like "myapp --server" )
   */
  //  QString CORBAExec() const { return m_strCORBAExec; }
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
   * @return the requested properties. Some often used properties
   *         have convenience access functions like @ref #exec,
   *         @ref #serviceTypes etc.
   *
   *         It depends upon the @ref #serviceTypes of this service which
   *         properties a service can have.
   *
   * @see KServiceType
   */
  virtual PropertyPtr property( const QString& _name ) const;
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
  static Ptr service( const QString& _name );

  /**
   * @return the whole list of services. Useful for being able to
   * to display them in a list box, for example.
   * Very memory consuming, don't use unless really necessary.
   */
  static List allServices();

private:
  QString m_strName;
  QString m_strExec;
  QString m_strCORBAExec;
  QString m_strIcon;
  QString m_strTerminalOptions;
  QString m_strPath;
  QString m_strComment;
  QString m_strActivationMode;
  QStringList m_lstRepoIds;
  QString m_strLibrary;
  int m_libraryMajor;
  int m_libraryMinor;
  QStringList m_lstLibraryDeps;
  QStringList m_lstServiceTypes;
  bool m_bAllowAsDefault;
  QMap<QString,QVariant> m_mapProps;
  bool m_bValid;
};

//QDataStream& operator>>( QDataStream& _str, KService& s );
//QDataStream& operator<<( QDataStream& _str, KService& s );

#endif
