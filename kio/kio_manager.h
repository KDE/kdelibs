#ifndef __manager_h__
#define __manager_h__

#include <string>
#include <list>

class K2Config;

class ProtocolManager
{
public:
  enum Type { T_STREAM, T_FILESYSTEM, T_NONE, T_ERROR };

  ProtocolManager();
  
  string find( const char *_protocol );
  Type inputType( const char *_protocol );
  Type outputType( const char *_protocol );
  bool listing( const char *_protocol, list<string>& _listing );
  bool supportsListing( const char *_protocol );
  bool supportsReading( const char *_protocol );
  bool supportsWriting( const char *_protocol );
  bool supportsMakeDir( const char *_protocol );
  bool supportsDeleting( const char *_protocol );
  bool supportsLinking( const char *_protocol );
  bool supportsMoving( const char *_protocol );

  int getReadTimeout();
  bool getMarkPartial();
  int getMinimumKeepSize();
  bool getAutoResume();
  bool getPersistent();

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
  void setPersistent( bool _mode );

  static ProtocolManager* self() { 
    if ( ! s_pManager )
      s_pManager = new ProtocolManager;
    return s_pManager;
  }
  
protected:
  void init();
  K2Config* findIntern( const char *_protocol );
  
  K2Config *m_pConfig;
  
  static ProtocolManager *s_pManager;
};

#endif
