// $Id$

#ifndef __main_h__
#define __main_h__

#include <kio_interface.h>
#include <kio_base.h>

#include <list>

#include "ftp.h"

class FtpProtocol : public IOProtocol
{
public:
  FtpProtocol( Connection *_conn );
  virtual ~FtpProtocol() { }

  virtual void slotGet( const char *_url );
  virtual void slotGetSize( const char *_url );

  virtual void slotPut( const char *_url, int _mode,
			bool _overwrite, bool _resume, int _size );

  virtual void slotMkdir( const char *_url, int _mode );

  virtual void slotCopy( const char *_source, const char *_dest );
  virtual void slotCopy( QStringList& _source, const char *_dest );

  virtual void slotMove( const char *_source, const char *_dest );
  virtual void slotMove( QStringList& _source, const char *_dest );

  virtual void slotDel( QStringList& _source );

  virtual void slotListDir( const char *_url );
  virtual void slotTestDir( const char *_url );

  virtual void slotData( void *_p, int _len );
  virtual void slotDataEnd();

  Connection *connection() { return ConnectionSignals::m_pConnection; }
  
  void jobError( int _errid, const char *_txt );

protected:
  struct Copy
  {
    QString m_strAbsSource;
    QString m_strRelDest;
    mode_t m_access;
    mode_t m_type;
    off_t m_size;
  };
  
  struct CopyDir
  {
    QString m_strAbsSource;
    QString m_strRelDest;
    mode_t m_access;
    mode_t m_type;
  };

  void doCopy( QStringList& _source, const char *_dest, bool _rename, bool _move = false );

  long listRecursive( const char *_path, QValueList<Copy>& _files,
		      QValueList<CopyDir>& _dirs, bool _rename );
  long listRecursive2( const char *_abs_path, const char *_rel_path,
		       QValueList<Copy>& _files, QValueList<CopyDir>& _dirs );

  bool m_bAutoSkip, m_bIgnoreJobErrors, m_bCanResume;
  int m_cmd;
  Ftp ftp;
};

class FtpIOJob : public IOJob
{
public:
  FtpIOJob( Connection *_conn, FtpProtocol *_Ftp );
  
  virtual void slotError( int _errid, const char *_txt );

protected:
  FtpProtocol *m_pFtp;
};

#endif
