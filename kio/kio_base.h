#ifndef __iojob_h__
#define __iojob_h__

#include "kio_interface.h"

#include <string>

class IOJob : public IOProtocol
{
public:
  IOJob( Connection *_conn );
  virtual ~IOJob() { }
  
  virtual bool put( const char *_url, int _mode, bool _overwrite );
  virtual bool get( const char *_url );
  virtual bool mkdir( const char *_url, int _mode );
  virtual bool listDir( const char *_url );
  virtual bool copy( const char *_source, const char *_dest );
  virtual bool testDir( const char *_url );
  virtual bool unmount( const char *_point );
  virtual bool mount( bool _ro, const char *_fstype, const char* _dev, const char *_point );
  
  virtual bool data( void *_p, int _len );
  
  virtual void slotReady();
  virtual void slotFinished();
  virtual void slotError( int _errid, const char *_txt );
  
  int cmd() { return m_cmd; }
  
  bool isReady() { return m_bIsReady; }
  bool hasFinished() { return ( m_cmd == CMD_NONE ); }
  bool hasError() { return m_bError; }
  int errorId() { return m_iError; }
  const char* errorText() { return m_strError.c_str(); }
  void clearError() { m_bError = false; }
  
protected:
  int m_cmd;
  bool m_bIsReady;
  bool m_bError;
  int m_iError;
  string m_strError;
};

#endif

