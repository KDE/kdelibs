/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __kprotocolmanager_h__
#define __kprotocolmanager_h__

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

class KProtocolManagerPrivate;

/**
 * Information about I/O (Internet, etc.) protocols supported by KDE.
 *
 * This class is useful if you want to know which protocols
 * KDE supports. In addition you can find out lots of information
 * about a certain protocol. KProtocolManager scans the *.desktop
 * files of all installed kioslaves to get this information.
 *
 * In addition, KProtocolManager has a heap of static functions that
 * allow you to read and write IO related KDE settings. These include
 * proxies, resuming, timeouts.
 *
 * However, please note that these settings apply to all applications.
 * This means that the proxy, timeouts etc. are saved in the users config
 * file and @em not in the config file of the application.
 *
 * @author Torben Weis <weis@kde.org>
 */
class KProtocolManager
{
public:
  enum Type { T_STREAM, T_FILESYSTEM, T_NONE, T_ERROR };

  /**
   * @return the library / executable to open for the protocol @p protocol
   * Example : "kio_ftp", meaning either the executable "kio_ftp" or
   * the library "kio_ftp.la" (recommended), whichever is available.
   */
  QString exec( const QString& protocol ) const;

  Type inputType( const QString& protocol ) const;
  Type outputType( const QString& protocol ) const;
  /**
   * @return the list of fields this protocol returns when listing
   * The current possibilities are
   * Name, Type, Size, Date, AccessDate, CreationDate, Access, Owner, Group, Link, URL, MimeType
   */
  QStringList listing( const QString& protocol ) const;
  bool isSourceProtocol( const QString& protocol ) const;
  bool isHelperProtocol( const QString& protocol ) const;
  bool isFilterProtocol( const QString& protocol ) const;
  bool isKnownProtocol( const QString& protocol ) const;
  bool supportsListing( const QString& protocol ) const;
  bool supportsReading( const QString& protocol ) const;
  bool supportsWriting( const QString& protocol ) const;
  bool supportsMakeDir( const QString& protocol ) const;
  bool supportsDeleting( const QString& protocol ) const;
  bool supportsLinking( const QString& protocol ) const;
  bool supportsMoving( const QString& protocol ) const;

  /**
   * @return list of all known protocols
   */
  QString defaultMimetype( const QString& protocol ) const;

  /**
   * Determining the mimetype of a URL can be done by simply looking at
   * the extension (that's the fast mode). However, when
   * the extension is unknown, a KIO::mimetype job will be necessary
   * (i.e. downloading the beginning of the file or asking the server for
   * the mimetype).
   *
   * @return true if we can trust the mimetype @p mimetype for
   * the protocol @p protocol. A HTTP URL ending with
   * .pl or .asp may not return that actual type, but anything else.
   * This information is used by KRun to know whether it should trust
   * or not the result of KMimeType::findByURL.
   * Defaults to true.
   */
  bool mimetypeFastMode( const QString& protocol, const QString& mimetype ) const;

  /**
   * Same as @ref mimetypeFastMode but using patterns to match the filename.
   * Used to avoid conflicts with existing mimetypes.
   * Example: *.stm is both audio/x-mod and shtml, over HTTP.
   * Using mimetypeFastMode is preferred in other cases, though.
   */
  bool patternFastMode( const QString& protocol, const QString& filename ) const;

  /**
   * @return list of all known protocols
   */
  QStringList protocols() const;

  static int readTimeout();
  static bool markPartial();
  static int minimumKeepSize();
  static bool autoResume();
  static bool persistentConnections();
  static QString remoteFileProtocol();

  static bool useProxy();

  /*
  * This method has been deprecated, please
  * use @ref proxyFor.
  *
  * @deprecated
  */
  static QString ftpProxy();

  /*
  * This method has been deprecated, please
  * use @ref proxyFor.
  *
  * @deprecated
  */
  static QString httpProxy();

  static QString noProxyFor();

  /*
  * Returns the proxy server address for a given
  * protocol
  *
  * @param protocol the protocol whose proxy info is needed
  * @return the proxy server address if one is available
  */
  static QString proxyFor( const QString& /* protocol */);

  static bool useCache();
  static int maxCacheAge();  // Maximum cache age in seconds.
  static int maxCacheSize(); // Maximum cache size in Kb.

  /**
   * Sets timeout for read operations.
   * This applies to FTP and HTTP connections.
   * If after a time @p timeout, the read operation doesn't finish
   * reading a packet, the read operation is
   * stopped with alarm command and the operation is restarted.
   * This value is used if the remote server supports resuming.
   * For the opposite case see @ref setReadTimeoutNoResume()
   */
  static void setReadTimeout( int _time );

  /**
   * Set this flag if you want slaves to add the extension .PART
   *  to all files during transfer.
   * This extension will be removed when file is fully transferred.
   *
   * This is a better way to discern finished transfers in case
   *  of transfer errors.
   * @param _mode Default value is @p false: Don't add the extension .PART.
   *
   */
  static void setMarkPartial( bool _mode );

  /**
   * Set the minimum size for keeping an interrupted transfer.
   *
   * A downloaded file whose transfer was interrupted will only be kept if
   * its size is bigger than @ _size, otherwise it will be deleted.
   *
   * Default value is 5000 bytes
   *
   */
  static void setMinimumKeepSize( int _size );

  /**
   * Set this flag if you want slaves to automatically resume
   * downloading files without asking the user in the "rename" dialog.
   *
   * @param _mode Default value is @p false: Don't resume automatically.
   *
   */
  static void setAutoResume( bool _mode );

  /**
   * Set this flag if you want slaves to have persistent connections (FTP).
   *
   * @param _mode Default value is true: Keep persistent connections.
   *
   */
  static void setPersistentConnections( bool _mode );

  /**
   * Set a protocol which should be used for remote @p file URLs.
   *
   * Default value is empty: Pass hostname as part of path.
   *
   * Example:
   * With setRemoteFileProtocol("smb"), the URL
   *    "file://atlas/dfaure"
   * will be converted to
   *    "smb://atlas/dfaure"
   *
   * File URLs without a hostname are not affected.
   *
   */
  static void setRemoteFileProtocol( const QString &remoteFileProtocol );

  static void setUseCache( bool _mode );

  static void setMaxCacheSize( int cache_size );

  static void setMaxCacheAge( int cache_age );

  /**
   * Set this flag if you want use proxies.
   *
   * @param Default value is false: Don't use proxies.
   *
   */
  static void setUseProxy( bool _mode );

  /**
   * Set the proxy for FTP transfer.
   *
   * This method has been deprecated, please
   * use @ref setProxyFor.
   *
   * @deprecated
   */
  static void setFtpProxy( const QString& _proxy );

  /**
   * Set the proxy for HTTP transfer
   *
   * This method has been deprecated, please
   * use @ref setProxyFor.
   *
   * @deprecated
   */
  static void setHttpProxy( const QString& _proxy );

  /*
  * Sets the proxy for the protocol given by @p protocol.
  *
  * When setting the proxy for a given protocol, do not
  * include any separator characters.  For example, to
  * set the proxy info for the "ftp" protocol , simply
  * use "ftp" and not "ftp://". However, the case does
  * not matter as it is always converted to lower
  * characters.
  *
  * @param protocol type of protocol to set proxy for
  * @param _proxy the proxy server address
  */
  static void setProxyFor( const QString& /* protocol */, const QString& /* _proxy */ );


  /**
   * Set the URLs for which we should not use the proxy.
   *
   */
  static void setNoProxyFor( const QString& _noproxy );


  static KProtocolManager& self() {
    if ( ! s_pManager )
      s_pManager = new KProtocolManager;
    return *s_pManager;
  }

protected:
  KProtocolManager();

private:
  void scanConfig( const QString& _dir );

  struct Protocol
  {
    QString exec;
    Type inputType;
    Type outputType;
    QStringList listing;
    bool isSourceProtocol;
    bool isHelperProtocol;
    bool supportsListing;
    bool supportsReading;
    bool supportsWriting;
    bool supportsMakeDir;
    bool supportsDeleting;
    bool supportsLinking;
    bool supportsMoving;
    QString defaultMimetype;
    QStringList mimetypesExcludedFromFastMode;
    QStringList patternsExcludedFromFastMode;
  };

  typedef QMap<QString,Protocol> Map;
  typedef QMap<QString,Protocol>::Iterator Iterator;
  typedef QMap<QString,Protocol>::ConstIterator ConstIterator;
  Map m_protocols;

  static KProtocolManager *s_pManager;

  KProtocolManagerPrivate *d;
};

#endif
