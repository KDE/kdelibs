#ifndef __kprotocolmanager_h__
#define __kprotocolmanager_h__

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

/**
 * This class is useful if you want to know which protocols
 * KDE supports. In addition you can query lots of informations
 * about a certain protocol. KProtocolManager scans the *.desktop
 * files of all installed kioslaves to get this information.
 *
 * In addition KProtocolManager has a heap of static functions that
 * allow you to read an write IO related KDE settings. These contain
 * proxies, resuming, timeouts.
 *
 * However, please notice that these settings apply to all applications.
 * That means that the proxy, timeouts etc. are saved in the users config
 * file and NOT in the config file of the application.
 *
 * @author: Torben Weis
 */
class KProtocolManager
{
public:
  enum Type { T_STREAM, T_FILESYSTEM, T_NONE, T_ERROR };

  QString executable( const QString& _protocol ) const;
  Type inputType( const QString& _protocol ) const;
  Type outputType( const QString& _protocol ) const;
  QStringList listing( const QString& _protocol ) const;
  bool isSourceProtocol( const QString& _protocol ) const;
  bool isFilterProtocol( const QString& _protocol ) const;
  bool isKnownProtocol( const QString& _protocol ) const;
  bool supportsListing( const QString& _protocol ) const;
  bool supportsReading( const QString& _protocol ) const;
  bool supportsWriting( const QString& _protocol ) const;
  bool supportsMakeDir( const QString& _protocol ) const;
  bool supportsDeleting( const QString& _protocol ) const;
  bool supportsLinking( const QString& _protocol ) const;
  bool supportsMoving( const QString& _protocol ) const;

  QStringList protocols() const;

  static int readTimeout();
  static bool markPartial();
  static int minimumKeepSize();
  static bool autoResume();
  static bool persistentConnections();
  static QString remoteFileProtocol();

  static bool useProxy();
  static QString ftpProxy();
  static QString httpProxy();
  static QString noProxyFor();

  /**
   * Sets timeout for read operations. This applies to ftp and http connections.
   * If after this timeout read doesn't finish reading packet, read operation is
   * stopped with alarm command and starts reading again.
   * This value is used if remote server supports resuming.
   * For opposite case see @ref #setReadTimeoutNoResume
   *
   */
  static void setReadTimeout( int _time );

  /**
   * Set this flag if you want slaves to add extension .PART to all files during transfer.
   * This extension will be removed when file is transfered.
   *
   * This is a better way to discern finished transfers in case of transfer errors.
   * Default value is false - don't add extension.
   *
   */
  static void setMarkPartial( bool _mode );

  /**
   * Set the minimum size for keepenig of interrupted transfer
   *
   * Downloaded file will only be kept, if its size is bigger then this limit,
   * Otherwise it will be deleted
   *
   * Default value is 5000 bytes
   *
   */
  static void setMinimumKeepSize( int _size );

  /**
   * Set this flag if you want slaves to automatically resume files without
   * asking in rename dialog
   *
   * Default value is false - don't resume automaticaly.
   *
   */
  static void setAutoResume( bool _mode );

  /**
   * Set this flag if you want slaves to have persistent connections ( ftp )
   *
   * Default value is true - keep persistent connections
   *
   */
  static void setPersistentConnections( bool _mode );

  /**
   * Set a protocol which should be used for remote "file"-URLs
   *
   * Default value is empty: Pass hostname as part of path.
   *
   * Example:
   * With "setRemoteFileProtocol("smb"), the URL
   *    "file://atlas/dfaure"
   * will be converted to
   *    "smb://atlas/dfaure"
   *
   * File URLs without a hostname are not affected.
   *
   */
  static void setRemoteFileProtocol( const QString &remoteFileProtocol );

  /**
   * Set this flag if you want use proxies
   *
   * Default value is false - don't use proxies.
   *
   */
  static void setUseProxy( bool _mode );

  /**
   * Set the proxy for ftp transfer
   *
   */
  static void setFtpProxy( const QString& _proxy );

  /**
   * Set the proxy for http transfer
   *
   */
  static void setHttpProxy( const QString& _proxy );


  /**
   * Set the URL's for which we should not use proxy
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
    QString executable;
    Type inputType;
    Type outputType;
    QStringList listing;
    bool isSourceProtocol;
    bool supportsListing;
    bool supportsReading;
    bool supportsWriting;
    bool supportsMakeDir;
    bool supportsDeleting;
    bool supportsLinking;
    bool supportsMoving;
  };

  typedef QMap<QString,Protocol> Map;
  typedef QMap<QString,Protocol>::Iterator Iterator;
  typedef QMap<QString,Protocol>::ConstIterator ConstIterator;
  Map m_protocols;

  static KProtocolManager *s_pManager;
};

#endif
