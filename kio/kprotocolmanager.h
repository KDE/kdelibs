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

#include <qstringlist.h>

#include <kapp.h>
#include <kio/global.h>

// This value has been deprecated!! Use KProtocolManager::defaultUserAgent() instead.
#define DEFAULT_USERAGENT_STRING \
QString("Mozilla/5.0 (Konqueror/%1; compatible MSIE 5.5; X11)").arg(KDE_VERSION_STRING)

class KConfig;
class KPAC;

/**
 * Information about I/O (Internet, etc.) settings.
 *
 * KProtocolManager has a heap of static functions that allows you to read
 * and write IO related KDE settings. These include proxy settings,
 * file transfer resuming, and general timeout settings.
 *
 * Note however that these settings apply to all applications.  This means that
 * the proxy, timeouts etc. are saved in the users config file and @em not in the
 * config file of the application.
 *
 * @p Original author:
 * @author Torben Weis <weis@kde.org>
 *
 * @p Revised by:
 * @author Waldo Bastain <bastain@kde.org>
 * @author Dawit Alemayehu <adawit@kde.org>
 */
class KProtocolManager
{
public:

  /**
   * Returns the default user-agent value.
   *
   * This function returns the default user-agent value
   */
  static QString defaultUserAgent();

  /**
   * Returns the default user-agent value.
   *
   * This function returns the default user-agent value
   * taking into account 'keys'
   *
   * Keys can be any of the folliwing:
   * 'o'	Show OS
   * 'v'	Show OS Version
   * 'p'	Show platform
   * 'm'	Show machine architecture
   * 'l'	Show language
   *
   */
  static QString defaultUserAgent(const QString &keys);

  /**
   * Returns the userAgent string configured for the
   * specified host.
   *
   * If hostname is not found or is empty (i.e. "" or
   * QString::null) this function will return the default
   * user agent.
   *
   * @param hostname name of the host
   * @return specified userAgent string
   */
  static QString userAgentForHost( const QString& );

  /**
   * @deprecated
   * TODO KDE 3.0: remove me
   */
  static void setUserAgentList( const QStringList& /*agentlist*/ );

  /**
   * @deprecated
   * TODO KDE 3.0: remove me
   */
  static QStringList userAgentList();


  /**************************************** TIMEOUT CONFIG ***************************************/

  /**
   * Returns the preferred timeout value for reading from
   * remote connections in secs.
   *
   * The minimum value that is accept
   * is set above or below the threshold limit, this
   * function will return the default value given by
   * @ref defaultConnectTimeout().
   *
   * @return timeout value for remote connection in secs.
   */
  static int readTimeout();

  /**
   * Returns the preferred timeout value for remote connections
   * in secs.
   *
   * The maximum value that can be set by the user is 6
   * minutes while the minimum is 3 seconds.  If the value
   * is set above or below the threshold limit, this
   * function will return the default value given by
   * @ref defaultConnectTimeout().
   *
   * @return timeout value for remote connection in secs.
   */
  static int connectTimeout();

  /**
   * Returns the preferred timeout value for proxy connections
   * in secs.
   *
   * The maximum value that can be set by the user is 2
   * minutes while the minimum is 3 seconds.  If the value
   * is set above or below the threshold limit, this
   * function will return the default value given by
   * @ref defaultProxyConnectTimeout()
   *
   * @return timeout value for proxy connection in secs.
   */
  static int proxyConnectTimeout();

  /**
   * Returns the preferred response timeout value for
   * remote connecting in secs.
   *
   * The maximum value that can be set by the user is 6
   * minutes while the minimum is 3 seconds.  If the value
   * is set above or below the threshold limit, this
   * function will return the default value given by
   * @ref defaultResponseTimeout()
   *
   * @return timeout value for remote connection in seconds.
   */
  static int responseTimeout();

  /**
   * Returns the set default timeout value for connecting
   * to remote machines.
   *
   * @return timeout value in seconds
   */
  static int defaultConnectTimeout();

  /**
   * @deprecated  Use DEFAULT_READ_TIMEOUT instead.
   * TODO KDE 3.0: Remove
   */
  static int defaultReadTimeout();

  /**
   * @deprecated  Use DEFAULT_PROXY_CONNECT_TIMEOUT instead.
   * TODO KDE 3.0: Remove
   */
  static int defaultProxyConnectTimeout();

  /**
   * @deprecated  Use DEFAULT_RESPONSE_TIMEOUT instead.
   * TODO KDE 3.0: Remove
   */
  static int defaultResponseTimeout();

  /**
   * @deprecated  Use MIN_TIMEOUT_VALUE instead.
   * TODO KDE 3.0: Remove
   */
  static int minimumTimeoutThreshold();

  /**
   * Sets the amount of time to wait for a response from
   * the socket
   *
   * @param timeout, the timeout value in seconds.
   */
  static void setReadTimeout( int timeout );

  /**
   * Sets the amount of time to wait for data arrival
   * from the remote/peer machine.
   *
   * @param timeout, the timeout value in seconds.
   */
  static void setConnectTimeout( int timeout );

  /**
   * Sets the amount of time to wait for data arrival
   * from the remote/peer machine.
   *
   * @param timeout, the timeout value in seconds.
   */
  static void setProxyConnectTimeout( int timeout );

  /**
   * Sets the amount of time to wait for data arrival
   * from the remote/peer machine.
   *
   * @param timeout, the timeout value in seconds.
   */
  static void setResponseTimeout( int timeout );


  /**************************************** PROXY CONFIG *****************************************/

  /**
   * Returns whether or not the user specified the
   * use of proxy server to make connections.
   */
  static bool useProxy();

  /**
   * Types of proxy configuration
   * @li NoProxy     - No proxy is used
   * @li ManualProxy - Proxies are manually configured
   * @li PACProxy    - A Proxy configuration URL has been given
   * @li WPADProxy   - A proxy should be automatically discovered
   */
  enum ProxyType
  {
      NoProxy,
      ManualProxy,
      PACProxy,
      WPADProxy
  };

  /**
   * Returns the type of proxy configuration that is used.
   */
  static ProxyType proxyType();

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

  /**
   * Returns the strings for hosts that should contacted
   * DIRECT bypassing any proxy settings.
   */
  static QString noProxyFor();

  /*
   * Returns the proxy server address for a given
   * protocol
   *
   * @param protocol the protocol whose proxy info is needed
   * @returns the proxy server address if one is available
   */
  static QString proxyFor( const QString& /* protocol */);

  /**
   * Returns the Proxy server address for a given URL
   * If automatic proxy configuration is configured, @ref KPAC
   * is used to determine the proxy server, otherwise the return
   * value of @ref proxyFor for the URL's protocol is used.
   * If an empty string is returned, the request is to be aborted,
   * a return value of "DIRECT" requests a direct connection.
   *
   * @param url the URL whose proxy info is needed
   * @returns the proxy server address if one is available
   */
  static QString proxyForURL( const KURL& /* url */ );

  /**
   * Marks this proxy as bad (down). It will not be used for the
   * next 30 minutes. (The script may supply an alternate proxy)
   */
  static void badProxy( const QString & /* proxy */ );

  /**
   * This method has been deprecated. Use
   * @ref setProxyType instead.
   *
   * @deprecated
   */
  static void setUseProxy( bool _mode );
  /**
   * Set the type of proxy configuration to use.
   *
   * @param type the @ref ProxyType to use.
   */
  static void setProxyType( ProxyType type );

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

  /**
   * @return the URL of the script for automatic proxy configuration
   */
  static QString proxyConfigScript();

  /**
   * Set the URL of the script for automatic proxy configuration
   */
  static void setProxyConfigScript( const QString & /* url */ );


  /**************************************** CACHE CONFIG *****************************************/

  /**
   * Returns true/false to indicate whether a cache
   * should be used
   */
  static bool useCache();

  static void setUseCache( bool _mode );

  /**
   * Returns the maximum age (in seconds) cached files
   * should be kept before they are deleted.
   */
  static int maxCacheAge();  // Maximum cache age in seconds.
  static void setMaxCacheAge( int cache_age );

  /**
   * Returns the maximum age (in seconds) cached files
   * should be kept before they are deleted.
   */
  static int maxCacheSize(); // Maximum cache size in Kb.
  static void setMaxCacheSize( int cache_size );

  /**
   * The directory which contains the cache files
   */
  static QString cacheDir();

  /**
   * Returns the Cache control directive to be used.
   */
  static KIO::CacheControl cacheControl();

  /**
   * Sets the Cache control directive.
   */
  static void setCacheControl(KIO::CacheControl);


  /**************************************** DOWNLOAD CONFIG ***************************************/

  static bool autoResume();

  static bool markPartial();

  static int minimumKeepSize();

  static bool persistentConnections();

  /**
   * Set this flag if you want slaves to add the extension .PART to all
   * files during transfer.  This extension will be removed when file is
   * fully transferred.
   *
   * This is a better way to discern finished transfers in case
   * of transfer errors.
   *
   * @param _mode Default value is @p false: Don't add the extension .PART.
   */
  static void setMarkPartial( bool _mode );

  /**
   * Set the minimum size for keeping an interrupted transfer.
   *
   * A downloaded file whose transfer was interrupted will only be kept if
   * its size is bigger than @ _size, otherwise it will be deleted.
   *
   * Default value is 5000 bytes
   */
  static void setMinimumKeepSize( int _size );

  /**
   * Set this flag if you want slaves to automatically resume
   * downloading files without asking the user in the "rename" dialog.
   *
   * @param _mode Default value is @p false: Don't resume automatically.
   */
  static void setAutoResume( bool _mode );

  /**
   * Set this flag if you want slaves to have persistent connections (FTP).
   *
   * @param _mode Default value is true: Keep persistent connections.
   */
  static void setPersistentConnections( bool _mode );


  /**************************************** OTHERS *****************************************/

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
   */
  static QString slaveProtocol(const KURL &url, QString &proxy);

  // FIXME KDE 3.0: remove me.
  static QString slaveProtocol( const QString & protocol );

private:
  static KConfig *config();
  static KConfig *http_config();
  static KPAC *pac();
};

#endif
