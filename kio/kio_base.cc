#include "kio_base.h"

#include <assert.h>

#include <iostream>

IOJob::IOJob( Connection *_conn ) : IOProtocol( _conn )
{
  m_cmd = CMD_NONE;
  m_bIsReady = false;
  m_bError = false;
}

bool IOJob::unmount( const char *_point )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_UNMOUNT;
  
  m_bIsReady = false;
  m_bError = false;

  return ConnectionSignals::unmount( _point );
}

bool IOJob::mount( bool _ro, const char *_fstype, const char* _dev, const char *_point )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_MOUNT;
  
  m_bIsReady = false;
  m_bError = false;

  return ConnectionSignals::mount( _ro, _fstype, _dev, _point );
}

bool IOJob::testDir( const char *_url )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_TESTDIR;
  
  m_bIsReady = false;
  m_bError = false;

  return ConnectionSignals::testDir( _url );
}

bool IOJob::copy( const char *_source, const char *_dest )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_COPY;
  
  m_bIsReady = false;
  m_bError = false;
  
  return ConnectionSignals::copy( _source, _dest );
}

bool IOJob::listDir( const char *_url )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_LIST;
  
  m_bIsReady = false;
  m_bError = false;

  return ConnectionSignals::listDir( _url );
}

bool IOJob::get( const char *_url )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_GET;
  
  m_bIsReady = false;
  m_bError = false;

  return ConnectionSignals::get( _url );
}

bool IOJob::put( const char *_url, int _mode, bool _overwrite )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_PUT;
  
  m_bIsReady = false;
  m_bError = false;
  
  return ConnectionSignals::put( _url, _mode, _overwrite );
}

bool IOJob::mkdir( const char *_url, int _mode )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_MKDIR;
  
  m_bIsReady = false;
  m_bError = false;
  
  return ConnectionSignals::mkdir( _url, _mode );
}

bool IOJob::data( void *_p, int _len )
{
  assert( m_cmd == CMD_PUT && m_bIsReady );
  
  return ConnectionSignals::data( _p, _len );
}

void IOJob::slotReady()
{
  m_bIsReady = true;
}

void IOJob::slotFinished()
{
  m_cmd = CMD_NONE;
  m_bIsReady = false;
}
  
void IOJob::slotError( int _errid, const char *_txt )
{
  m_bError = true;
  m_iError = _errid;
  m_strError = _txt;
}
