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
 * KProtocolManager has a heap of static functions that
 * allow you to read and write IO related KDE settings.
 * These include proxies, resuming, timeouts.
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

  /** Values to be added to the default user agent string */
  struct UAMODIFIERS
  {
    UAMODIFIERS() {
      showOS = false;
      showPlatform = false;
      showOSVersion = false;
      showMachine = false;
      showLanguage = false;
    }
    bool showOS;        // if true, the OS value will be included (ex: Linux/FreeBSD)
    bool showPlatform;  // if true, the platform value will be included (ex: X11)
    bool showOSVersion; // if true, the OS version will be included (ex: 2.4.1)
    bool showMachine;   // if true, the processor type will be included (ex: i486)
    bool showLanguage;  // if true, the language in use will be included (ex: en_US)
  };

  static int readTimeout();
  static bool markPartial();
  static int minimumKeepSize();
  static bool autoResume();
  static bool persistentConnections();

  /**
   * Returns the preffered timeout value for remote connections
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
   * Returns the preffered timeout value for proxy connections
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
   * Returns the preffered response timeout value for
   * remote connecting in secs.
   *
   * The maximum value that can be set by the user is 6
   * minutes while the minimum is 3 seconds.  If the value
   * is set above or below the threshold limit, this
   * function will return the default value given by
   * @ref defaultResponseTimeout()
   *
   * @return timeout value for remote connection in secs.
   */
  static int responseTimeout();

  /**
   * Returns (in seconds) the set default timeout
   * value for connecting to remote machines.
   *
   * This method returns the value defined by
   * DEFAULT_CONNECT_TIMEOUT in the source file.
   */
  static int defaultConnectTimeout();

  /**
   * Returns (in seconds) the set default timeout value
   * for connecting to a proxy machine.
   *
   * This method returns the value defined by
   * DEFAULT_PROXY_CONNECT_TIMEOUT in the source file.
   */
  static int defaultProxyConnectTimeout();

  /**
   * Returns (in seconds) the set default timeout value
   * for connecting to a proxy machine.
   *
   * This method returns the value defined by
   * DEFAULT_RESPONSE_TIMEOUT in the source file.
   */
  static int defaultResponseTimeout();

  /**
   * Returns the minimum timeout value that can be
   * assigned for connecting and responses to and
   * from remote and proxy machines.
   *
   * This method returns the value defined by
   * MIN_TIMEOUT_VALUE in the source file.
   */
  static int minimumTimeoutThreshold();

  /**
   * Returns whether or not the user specified the
   * use of proxy server to make connections.
   */
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
   * Returns whether a proxy config script is being used
   *
   * @returns true if a proxy config script is used
   */
  static bool hasProxyConfigScript();

  /**
   * Return the protocol to use in order to handle the given @p protocol
   * It's usually the same, except that FTP, when handled by a proxy,
   * needs an HTTP ioslave.
   */
  static QString slaveProtocol( const QString & protocol );

  /**
   * Returns true/false to indicate whether a cache
   * should be used
   */
  static bool useCache();

  /**
   * Returns the maximum age (in seconds) cached files
   * should be kept before they are deleted.
   */
  static int maxCacheAge();  // Maximum cache age in seconds.

  /**
   * Returns the maximum age (in seconds) cached files
   * should be kept before they are deleted.
   */
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

  /**
   * Set the URL of the script for automatic proxy configuration
   */
  static void setProxyConfigScript( const QString & /* url */ );

  /**
   * Force a reload of the general config file of
   * io-slaves ( kioslaverc).
   */
  static void reparseConfiguration();

  /**
   * Returns the userAgent string configured for the
   * specified host.
   *
   * If hostname is not found or is empty (i.e. "" or
   * QString::null) this function will return the default
   * agent string as defined above in DEFAULT_USERAGENT_STRING.
   *
   * @param hostname name of the host
   * @return specified userAgent string
   */
  static QString userAgentForHost( const QString& /*hostname*/ );

  /**
   * Sets the list of userAgent.
   *
   * Sets the user agent name to be sent when connecting
   * to different sites. The default user agent string is
   * defined above by DEFAULT_USERAGENT_STRING.
   *
   * @param list the list of user agent strings to use.
   */
  static void setUserAgentList( const QStringList& /*agentlist*/ );

  /**
   * Returns the list of user agents configured for
   * specific domains.
   *
   * This function returns the list of site specific user-
   * agent entries set by the user. The format is:
   *
   * site-name::user-agent string::alias (description of user-agent string)
   *
   * @return the list of per-site user-agent entries.
   */
  static QStringList userAgentList();

  /**
   * the directory which contains the cache files
   */
  static QString cacheDir();

  /**
   * Sets flags that determine which extra infromation, if any,
   * will be appended to the default user-agent value.
   *
   * See @ref UAMODIFIERS above for details.
   *
   * @param mods the modifiers to be enabled.
   */
  static void setDefaultUserAgentModifiers( const UAMODIFIERS& );

  /**
   * Reads the default user-agent modifiers from config file
   * and sets the values for the supplied argument accordingly.
   *
   * @param mod the modifier to be updated with config file values
   */
  static void defaultUserAgentModifiers( UAMODIFIERS& );

  /**
   * Returns the default user-agent value.
   *
   * This function returns the default user-agent value
   * set
   */
  static QString defaultUserAgent();

  /**
   * Returns the default user-agent with the specified extra
   * identifiers enabled.
   *
   * This is a convienence method for modifying the default user-agent
   * string without affecting the saved values.  Unlike @ref setUserAgentModifier
   * this method niether reads nor changes the modifiers set in the
   * config file.  It simply returns the default user-agent after
   * applying the values of supplied by the modifier argument.
   *
   * @param mod values to be applied to default user-agent string.
   */
  static QString customDefaultUserAgent( const UAMODIFIERS& );

  /**
   * Returns true if the user-agent identification should be
   * sent to remote sites.
   *
   * This function is present to respect the user's privacy
   * concerns and should be respected by all io-slave that
   * include the user-agent string.
   *
   * @return true if user-agent string should be included when
   * communicating with remote machine
   */
  static bool sendUserAgent();

  /**
   * Sets the flag that determines whether the user agent string will
   * be sent to remote sites.
   *
   * @param values to be applied to default user-agent string.
   */
  static void setEnableSendUserAgent( bool );

  /**
   * returns the Cache control directive to be used by default,
   * say if no reload flag is set and no metadata specifies anything else.
   */
  static KIO::CacheControl defaultCacheControl();

  /**
   * sets the policy Cache control directive.
   */
  static void setDefaultCacheControl(KIO::CacheControl);

private:
  static KConfig *config();
  static KPAC *pac();

  static KConfig *_config;
  static KPAC *_pac;
};

#endif
