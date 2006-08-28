/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000- Waldo Bastain <bastain@kde.org>
   Copyright (C) 2000- Dawit Alemayehu <adawit@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef __kprotocolmanager_h__
#define __kprotocolmanager_h__

#include <qstringlist.h>

#include <kapplication.h>
#include <kio/global.h>
#include "kprotocolinfo.h"

class KConfig;

/**
 * Provides information about I/O (Internet, etc.) settings chosen/set
 * by the end user.
 *
 * KProtocolManager has a heap of static functions that allows only read
 * access to KDE's IO related settings. These include proxy, cache, file
 * transfer resumption, timeout and user-agent related settings.
 *
 * The information provided by this class is generic enough to be applicable
 * to any application that makes use of KDE's IO sub-system.  Note that this
 * mean the proxy, timeout etc. settings are saved in a separate user-specific
 * config file and not in the config file of the application.
 *
 * Original author:
 * @author Torben Weis <weis@kde.org>
 *
 * Revised by:
 * @author Waldo Bastain <bastain@kde.org>
 * @author Dawit Alemayehu <adawit@kde.org>
 * @see KPAC
 */
class KIO_EXPORT KProtocolManager
{
public:


/*=========================== USER-AGENT SETTINGS ===========================*/


  /**
   * Returns the default user-agent string.
   *
   * @return the default user-agent string
   */
  static QString defaultUserAgent();

  /**
   * Returns the default user-agent value.
   *
   * @param keys can be any of the following:
   * @li 'o'	Show OS
   * @li 'v'	Show OS Version
   * @li 'p'	Show platform
   * @li 'm'	Show machine architecture
   * @li 'l'	Show language
   * @return the default user-agent value with the given @p keys
   */
  static QString defaultUserAgent(const QString &keys);

  /**
   * Returns the userAgent string configured for the
   * specified host.
   *
   * If hostname is not found or is empty (i.e. "" or
   * QString()) this function will return the default
   * user agent.
   *
   * @param hostname name of the host
   * @return specified userAgent string
   */
  static QString userAgentForHost( const QString &hostname );


/*=========================== TIMEOUT CONFIG ================================*/


  /**
   * Returns the preferred timeout value for reading from
   * remote connections in seconds.
   *
   * @return timeout value for remote connection in secs.
   */
  static int readTimeout();

  /**
   * Returns the preferred timeout value for remote connections
   * in seconds.
   *
   * @return timeout value for remote connection in secs.
   */
  static int connectTimeout();

  /**
   * Returns the preferred timeout value for proxy connections
   * in seconds.
   *
   * @return timeout value for proxy connection in secs.
   */
  static int proxyConnectTimeout();

  /**
   * Returns the preferred response timeout value for
   * remote connecting in seconds.
   *
   * @return timeout value for remote connection in seconds.
   */
  static int responseTimeout();


/*=============================== PROXY CONFIG ==============================*/


  /**
   * Returns whether or not the user specified the
   * use of proxy server to make connections.
   * @return true to use a proxy
   */
  static bool useProxy();

  /**
   * Returns whether or not the the proxy server
   * lookup should be reversed or not.
   * @return true to use a reversed proxy
   */
  static bool useReverseProxy();

  /**
   * Types of proxy configuration
   * @li NoProxy     - No proxy is used
   * @li ManualProxy - Proxies are manually configured
   * @li PACProxy    - A Proxy configuration URL has been given
   * @li WPADProxy   - A proxy should be automatically discovered
   * @li EnvVarProxy - Use the proxy values set through environment variables.
   */
  enum ProxyType
  {
      NoProxy,
      ManualProxy,
      PACProxy,
      WPADProxy,
      EnvVarProxy
  };

  /**
   * Returns the type of proxy configuration that is used.
   * @return the proxy type
   */
  static ProxyType proxyType();

  /**
   * Proxy authorization modes.
   *
   * @li Prompt     - Ask for authorization as needed
   * @li Automatic  - Use auto login as defined in kionetrc files.
   */
  enum ProxyAuthMode
  {
      Prompt,
      Automatic
  };

  /**
   * Returns the way proxy authorization should be handled.
   *
   * @return the proxy authorization mode
   * @see ProxyAuthMode
   */
  static ProxyAuthMode proxyAuthMode();

  /**
   * Returns the strings for hosts that should contacted
   * DIRECTLY, bypassing any proxy settings.
   * @return a list of (comma-separated) hostnames or partial host
   *         names
   */
  static QString noProxyFor();

  /**
   * Returns the proxy server address for a given
   * protocol.
   *
   * @param protocol the protocol whose proxy info is needed
   * @returns the proxy server address if one is available,
   *          or QString() if not available
   */
  static QString proxyFor( const QString& protocol );

  /**
   * Returns the Proxy server address for a given URL
   * If automatic proxy configuration is configured, KPAC
   * is used to determine the proxy server, otherwise the return
   * value of proxyFor for the URL's protocol is used.
   * If an empty string is returned, the request is to be aborted,
   * a return value of "DIRECT" requests a direct connection.
   *
   * @param url the URL whose proxy info is needed
   * @returns the proxy server address if one is available
   *          or QString() otherwise
   */
  static QString proxyForUrl( const KUrl& url );

  /**
   * Marks this proxy as bad (down). It will not be used for the
   * next 30 minutes. (The script may supply an alternate proxy)
   * @param proxy the proxy to mark as bad (as URL)
   */
  static void badProxy( const QString & proxy );

  /**
   * Returns the URL of the script for automatic proxy configuration.
   * @return the proxy configuration script
   */
  static QString proxyConfigScript();


/*========================== CACHE CONFIG ===================================*/


  /**
   * Returns true/false to indicate whether a cache
   * should be used
   *
   * @return true to use the cache, false otherwisea
   */
  static bool useCache();

  /**
   * Returns the maximum age in seconds cached files should be
   * kept before they are deleted as necessary.
   *
   * @return the maximum cache age in seconds
   */
  static int maxCacheAge();

  /**
   * Returns the maximum size that can be used for caching.
   *
   * By default this function returns the DEFAULT_MAX_CACHE_SIZE
   * value as defined in http_slave_defaults.h.  Not that the
   * value returned is in bytes, hence a value of 5120 would mean
   * 5 Kb.
   *
   * @return the maximum cache size in bytes
   */
  static int maxCacheSize(); // Maximum cache size in Kb.

  /**
   * The directory which contains the cache files.
   * @return the directory that contains the cache files
   */
  static QString cacheDir();

  /**
   * Returns the Cache control directive to be used.
   * @return the cache control value
   */
  static KIO::CacheControl cacheControl();


/*============================ DOWNLOAD CONFIG ==============================*/

  /**
   * Returns true if partial downloads should be
   * automatically resumed.
   * @return true to resume partial downloads
   */
  static bool autoResume();

  /**
   * Returns true if partial downloads should be marked
   * with a ".part" extension.
   * @return true if partial downloads should get an ".part" extension
   */
  static bool markPartial();

  /**
   * Returns the minimum file size for keeping aborted
   * downloads.
   *
   * Any data downloaded that does not meet this minimum
   * requirement will simply be discarded. The default size
   * is 5 KB.
   *
   * @return the minimum keep size for aborted downloads in bytes
   */
  static int minimumKeepSize();


  /*============================ NETWORK CONNECTIONS ==========================*/
  /**
   * Returns true if proxy connections should be persistent.
   * @return true if proxy connections should be persistent
   */
  static bool persistentProxyConnection();

  /**
   * Returns true if connections should be persistent
   * @return true if the connections should be persistent
   */
  static bool persistentConnections();


  /*===================== PROTOCOL CAPABILITIES ===============================*/

  /**
   * Returns whether the protocol can list files/objects.
   * If a protocol supports listing it can be browsed in e.g. file-dialogs
   * and konqueror.
   *
   * Whether a protocol supports listing is determined by the "listing="
   * field in the protocol description file.
   * If the protocol support listing it should list the fields it provides in
   * this field. If the protocol does not support listing this field should
   * remain empty (default.)
   *
   * @param url the url to check
   * @return true if the protocol support listing
   * @see listing()
   */
  static bool supportsListing( const KUrl &url );

  /**
   * Returns whether the protocol can retrieve data from URLs.
   *
   * This corresponds to the "reading=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if it is possible to read from the URL
   */
  static bool supportsReading( const KUrl &url );

  /**
   * Returns whether the protocol can store data to URLs.
   *
   * This corresponds to the "writing=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol supports writing
   */
  static bool supportsWriting( const KUrl &url );

  /**
   * Returns whether the protocol can create directories/folders.
   *
   * This corresponds to the "makedir=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol can create directories
   */
  static bool supportsMakeDir( const KUrl &url );

  /**
   * Returns whether the protocol can delete files/objects.
   *
   * This corresponds to the "deleting=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol supports deleting
   */
  static bool supportsDeleting( const KUrl &url );

  /**
   * Returns whether the protocol can create links between files/objects.
   *
   * This corresponds to the "linking=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol supports linking
   */
  static bool supportsLinking( const KUrl &url );

  /**
   * Returns whether the protocol can move files/objects between different
   * locations.
   *
   * This corresponds to the "moving=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol supports moving
   */
  static bool supportsMoving( const KUrl &url );

  /**
   * Returns whether the protocol can copy files/objects directly from the
   * filesystem itself. If not, the application will read files from the
   * filesystem using the file-protocol and pass the data on to the destination
   * protocol.
   *
   * This corresponds to the "copyFromFile=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol can copy files from the local file system
   */
  static bool canCopyFromFile( const KUrl &url );

  /**
   * Returns whether the protocol can copy files/objects directly to the
   * filesystem itself. If not, the application will receive the data from
   * the source protocol and store it in the filesystem using the
   * file-protocol.
   *
   * This corresponds to the "copyToFile=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol can copy files to the local file system
   */
  static bool canCopyToFile( const KUrl &url );

  /**
   * Returns whether the protocol can rename (i.e. move fast) files/objects
   * directly from the filesystem itself. If not, the application will read
   * files from the filesystem using the file-protocol and pass the data on
   * to the destination protocol.
   *
   * This corresponds to the "renameFromFile=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol can rename/move files from the local file system
   */
  static bool canRenameFromFile( const KUrl &url );

  /**
   * Returns whether the protocol can rename (i.e. move fast) files/objects
   * directly to the filesystem itself. If not, the application will receive
   * the data from the source protocol and store it in the filesystem using the
   * file-protocol.
   *
   * This corresponds to the "renameToFile=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol can rename files to the local file system
   */
  static bool canRenameToFile( const KUrl &url );

  /**
   * Returns whether the protocol can recursively delete directories by itself.
   * If not (the usual case) then KIO will list the directory and delete files
   * and empty directories one by one.
   *
   * This corresponds to the "deleteRecursive=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol can delete non-empty directories by itself.
   */
  static bool canDeleteRecursive( const KUrl &url );

  /**
   * This setting defines the strategy to use for generating a filename, when
   * copying a file or directory to another directory. By default the destination
   * filename is made out of the filename in the source URL. However if the
   * ioslave displays names that are different from the filename of the URL
   * (e.g. kio_fonts shows Arial for arial.ttf, or kio_trash shows foo.txt and
   * uses some internal URL), using Name means that the display name (UDS_NAME)
   * will be used to as the filename in the destination directory.
   *
   * This corresponds to the "fileNameUsedForCopying=" field in the protocol description file.
   * Valid values for this field are "Name" or "FromURL" (default).
   *
   * @param url the url to check
   * @return how to generate the filename in the destination directory when copying/moving
   */
  static KProtocolInfo::FileNameUsedForCopying fileNameUsedForCopying( const KUrl &url );

  /**
   * Returns default mimetype for this URL based on the protocol.
   *
   * This corresponds to the "defaultMimetype=" field in the protocol description file.
   *
   * @param url the url to check
   * @return the default mime type of the protocol, or null if unknown
   */
  static QString defaultMimetype( const KUrl& url );

  /**
   * Returns whether the protocol should be treated as a filesystem
   * or as a stream when reading from it.
   *
   * This corresponds to the "input=" field in the protocol description file.
   * Valid values for this field are "filesystem", "stream" or "none" (default).
   *
   * @param url the url to check
   * @return the input type of the given @p url
   */
  static KProtocolInfo::Type inputType( const KUrl &url );

  /**
   * Returns whether the protocol should be treated as a filesystem
   * or as a stream when writing to it.
   *
   * This corresponds to the "output=" field in the protocol description file.
   * Valid values for this field are "filesystem", "stream" or "none" (default).
   *
   * @param url the url to check
   * @return the output type of the given @p url
   */
  static KProtocolInfo::Type outputType( const KUrl &url );

  /**
   * Returns the list of fields this protocol returns when listing
   * The current possibilities are
   * Name, Type, Size, Date, AccessDate, Access, Owner, Group, Link, URL, MimeType
   * as well as Extra1, Extra2 etc. for extra fields (see extraFields).
   *
   * This corresponds to the "listing=" field in the protocol description file.
   * The supported fields should be separated with ',' in the protocol description file.
   *
   * @param url the url to check
   * @return a list of field names
   */
  static QStringList listing( const KUrl &url );


  /**
   * Returns whether the protocol can act as a source protocol.
   *
   * A source protocol retrieves data from or stores data to the
   * location specified by a URL.
   * A source protocol is the opposite of a filter protocol.
   *
   * The "source=" field in the protocol description file determines
   * whether a protocol is a source protocol or a filter protocol.
   * @param url the url to check
   * @return true if the protocol is a source of data (e.g. http), false if the
   *         protocol is a filter (e.g. gzip)
   */
  static bool isSourceProtocol( const KUrl &url );

  /*=============================== OTHERS ====================================*/


  /**
   * Force a reload of the general config file of
   * io-slaves ( kioslaverc).
   */
  static void reparseConfiguration();

  /**
   * Return the protocol to use in order to handle the given @p url
   * It's usually the same, except that FTP, when handled by a proxy,
   * needs an HTTP ioslave.
   *
   * When a proxy is to be used, proxy contains the URL for the proxy.
   * @param url the url to check
   * @param proxy the URL of the proxy to use
   * @return the slave protocol (e.g. 'http'), can be null if unknown
   */
  static QString slaveProtocol(const KUrl &url, QString &proxy);

  /**
   * @internal
   * (Shared with SlaveConfig)
   */
  static KConfig *config();
private:
  static KConfig *http_config();
};
#endif
