#ifndef __iojob_h__
#define __iojob_h__ "$Id$"

#include "kio_interface.h"

class IOJob : public IOProtocol
{
public:
  IOJob( Connection *_conn );
  virtual ~IOJob() { }
  
  virtual bool get( const char *_url );
  virtual bool getSize( const char *_url );

  /**
   * @param _mode may be -1. In this case no special permission mode is set.
   */
  virtual bool put( const char *_url, int _mode,
		    bool _overwrite, bool _resume, int _size );
  virtual bool mkdir( const char *_url, int _mode );

  virtual bool copy( const char* _source, const char *_dest );
  virtual bool copy( QStringList& _source, const char *_dest );
  virtual bool move( const char* _source, const char *_dest );
  virtual bool move( QStringList& _source, const char *_dest );
  virtual bool del( const char *_url );
  virtual bool del( QStringList& _source );

  virtual bool testDir( const char *_url );
  virtual bool listDir( const char *_url );

  virtual bool unmount( const char *_point );
  virtual bool mount( bool _ro, const char *_fstype, const char* _dev, const char *_point );
  
  virtual bool data( const void *_p, int _len );
  
  virtual void slotReady();
  virtual void slotFinished();
  virtual void slotError( int _errid, const char *_txt );
  
  int cmd() { return m_cmd; }
  
  bool isReady() { return m_bIsReady; }
  bool hasFinished() { return ( m_cmd == CMD_NONE ); }
  bool hasError() { return m_bError; }
  int errorId() { return m_iError; }
  QString errorText() { return m_strError; }
  void clearError() { m_bError = false; }
  
protected:
  int m_cmd;
  bool m_bIsReady;
  bool m_bError;
  int m_iError;
  QString m_strError;
};

#endif
