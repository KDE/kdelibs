#ifndef __kprotocolmanager_h__
#define __kprotocolmanager_h__

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

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
  
  int readTimeout() const;
  bool markPartial() const;
  int minimumKeepSize() const;
  bool autoResume() const;
  bool persistentConnections() const;
  QString remoteFileProtocol() const;
 
  bool useProxy() const;
  QString ftpProxy() const;
  QString httpProxy() const;
  QString noProxyFor() const;

  /**
   * Sets timeout for read operations. This applies to ftp and http connections.
   * If after this timeout read doesn't finish reading packet, read operation is
   * stopped with alarm command and starts reading again.
   * This value is used if remote server supports resuming.
   * For opposite case see @ref #setReadTimeoutNoResume
   *
   */
  void setReadTimeout( int _time );

  /**
   * Set this flag if you want slaves to add extension .PART to all files during transfer.
   * This extension will be removed when file is transfered.
   *
   * This is a better way to discern finished transfers in case of transfer errors.
   * Default value is false - don't add extension.
   *
   */
  void setMarkPartial( bool _mode );

  /**
   * Set the minimum size for keepenig of interrupted transfer
   *
   * Downloaded file will only be kept, if its size is bigger then this limit,
   * Otherwise it will be deleted
   *
   * Default value is 5000 bytes
   *
   */
  void setMinimumKeepSize( int _size );

  /**
   * Set this flag if you want slaves to automatically resume files without
   * asking in rename dialog
   *
   * Default value is false - don't resume automaticaly.
   *
   */
  void setAutoResume( bool _mode );

  /**
   * Set this flag if you want slaves to have persistent connections ( ftp )
   *
   * Default value is true - keep persistent connections
   *
   */
  void setPersistentConnections( bool _mode );

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
  void setRemoteFileProtocol( const QString &remoteFileProtocol );

  /**
   * Set this flag if you want use proxies
   *
   * Default value is false - don't use proxies.
   *
   */
  void setUseProxy( bool _mode );

  /**
   * Set the proxy for ftp transfer
   *
   */
  void setFtpProxy( const QString& _proxy );

  /**
   * Set the proxy for http transfer
   *
   */
  void setHttpProxy( const QString& _proxy );


  /**
   * Set the URL's for which we should not use proxy
   *
   */
  void setNoProxyFor( const QString& _noproxy );


  static KProtocolManager& self() { 
    if ( ! s_pManager )
      s_pManager = new KProtocolManager;
    return *s_pManager;
  }
  
protected:
  KProtocolManager();

private:
  void scanConfig( const QString& _dir, bool _islocal );

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
