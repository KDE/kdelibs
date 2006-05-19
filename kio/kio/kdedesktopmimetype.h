/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                     David Faure   <faure@kde.org>
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

#ifndef KDEDESKTOPMIMETYPE_H
#define KDEDESKTOPMIMETYPE_H

#include "kmimetype.h"

/**
 * Mime type for desktop files.
 * Handles mount/umount icon, and user-defined properties.
 * @short Mimetype for a .desktop file
 */
class KIO_EXPORT KDEDesktopMimeType : public KMimeType
{
  K_SYCOCATYPE( KST_KDEDesktopMimeType, KMimeType )

public:
  enum ServiceType { ST_MOUNT, ST_UNMOUNT, /* ST_PROPERTIES, */ ST_USER_DEFINED };

  /**
   * Structure representing a service, in the list of services
   * returned by builtinServices and userDefinedServices
   */
  struct Service
  {
    Service() { m_display = true; }
    bool isEmpty() const { return m_strName.isEmpty(); }
    QString m_strName;
    QString m_strIcon;
    QString m_strExec;
    ServiceType m_type;
    bool m_display;
  };
  // KDEDesktopMimeType( const QString & _fullpath, const QString& _type, const QString& _icon,
  //                     const QString& _comment, const QStringList& _patterns );
  // KDEDesktopMimeType( const QString & _fullpath ) : KMimeType( _fullpath ) { }
  /**
   * Construct a desktop mimetype and take all information from a desktop file.
   * @param config the desktop configuration file that describes the mime type
   */
  KDEDesktopMimeType( KDesktopFile *config) : KMimeType( config ) { }
  /** \internal */
  KDEDesktopMimeType( QDataStream& _str, int offset ) : KMimeType( _str, offset ) { }

  virtual QString icon( const KUrl& _url ) const;
  virtual KDE_DEPRECATED QPixmap pixmap( const KUrl& _url, K3Icon::Group _group, int _force_size = 0,
                          int _state = 0, QString * _path = 0L ) const;
  virtual QString comment( const KUrl& _url ) const;

  /**
   * Returns a list of services for the given .desktop file that are handled
   * by kio itself. Namely mount/unmount for FSDevice files.
   * @return the list of services
   */
  static QList<Service> builtinServices( const KUrl& _url );
  /**
   * Returns a list of services defined by the user as possible actions
   * on the given .desktop file. May include empty actions which represent where
   * visual separators should appear in user-visible representations of those actions,
   * such as separators in a menu.
   * @param path the path to the desktop file describing the services
   * @param bLocalFiles true if those services are to be applied to local files only
   * (if false, services that don't have %u or %U in the Exec line won't be taken into account).
   * @return the list of user deviced actions
   */
  static QList<Service> userDefinedServices( const QString& path, bool bLocalFiles );

  /**
   * Overload of userDefinedServices for speed purposes: it takes a KConfig* so that
   * the caller can check things in the file without having it parsed twice.
   */
  static QList<Service> userDefinedServices( const QString& path, KConfig& config, bool bLocalFiles );

  /**
   * Overload of userDefinedServices but also allows you to pass a list of urls for this file.
   * This allows for the menu to be changed depending on the exact files via
   * the X-KDE-GetActionMenu extension.
   */
  static QList<Service> userDefinedServices( const QString& path, KConfig& config, bool bLocalFiles,  const KUrl::List & file_list);

  /**
   * Execute @p service on the list of @p urls.
   * @param urls the list of urls
   * @param service the service to execute
   */
  static void executeService( const KUrl::List& urls, KDEDesktopMimeType::Service& service );

  /**
   * Invokes the default action for the desktop entry. If the desktop
   * entry is not local, then only false is returned. Otherwise we
   * would create a security problem. Only types Link and Mimetype
   * could be followed.
   *
   * @param _url the url to run
   * @param _is_local true if the URL is local, false otherwise
   * @return true on success and false on failure.
   * @see KRun::runUrl
   */
  static pid_t run( const KUrl& _url, bool _is_local );

protected:
  static pid_t runFSDevice( const KUrl& _url, const KSimpleConfig &cfg );
  static pid_t runApplication( const KUrl& _url, const QString & _serviceFile );
  static pid_t runLink( const KUrl& _url, const KSimpleConfig &cfg );
  static pid_t runMimeType( const KUrl& _url, const KSimpleConfig &cfg );
protected:
  virtual void virtual_hook( int id, void* data );
};

#endif
