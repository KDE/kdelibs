// $Id$

#include "kio_base.h"

#include <assert.h>

#include <iostream.h>

KIOJobBase::KIOJobBase( KIOConnection *_conn ) : KIOProtocol( _conn )
{
  m_cmd = CMD_NONE;
  m_bIsReady = false;
  m_bError = false;
}

bool KIOJobBase::unmount( const char *_point )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_UNMOUNT;
  
  m_bIsReady = false;
  m_bError = false;

  return KIOConnectionSignals::unmount( _point );
}

bool KIOJobBase::mount( bool _ro, const char *_fstype, const char* _dev, const char *_point )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_MOUNT;
  
  m_bIsReady = false;
  m_bError = false;

  return KIOConnectionSignals::mount( _ro, _fstype, _dev, _point );
}

bool KIOJobBase::testDir( const char *_url )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_TESTDIR;
  
  m_bIsReady = false;
  m_bError = false;

  return KIOConnectionSignals::testDir( _url );
}

bool KIOJobBase::copy( QStringList& _source, const char *_dest )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_MCOPY;
  
  m_bIsReady = false;
  m_bError = false;
  
  return KIOConnectionSignals::copy( _source, _dest );
}

bool KIOJobBase::copy( const char* _source, const char *_dest )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_COPY;
  
  m_bIsReady = false;
  m_bError = false;
  
  return KIOConnectionSignals::copy( _source, _dest );
}

bool KIOJobBase::move( QStringList& _source, const char *_dest )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_MMOVE;
  
  m_bIsReady = false;
  m_bError = false;
  
  return KIOConnectionSignals::move( _source, _dest );
}

bool KIOJobBase::move( const char* _source, const char *_dest )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_MOVE;
  
  m_bIsReady = false;
  m_bError = false;
  
  return KIOConnectionSignals::move( _source, _dest );
}

bool KIOJobBase::del( QStringList& _source )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_MDEL;
  
  m_bIsReady = false;
  m_bError = false;

  return KIOConnectionSignals::del( _source );
}

bool KIOJobBase::del( const char *_url )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_DEL;
  
  m_bIsReady = false;
  m_bError = false;

  return KIOConnectionSignals::del( _url );
}

bool KIOJobBase::listDir( const char *_url )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_LIST;
  
  m_bIsReady = false;
  m_bError = false;

  return KIOConnectionSignals::listDir( _url );
}

bool KIOJobBase::get( const char *_url )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_GET;
  
  m_bIsReady = false;
  m_bError = false;

  return KIOConnectionSignals::get( _url );
}

bool KIOJobBase::getSize( const char* _url )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_GET_SIZE;
  
  m_bIsReady = false;
  m_bError = false;

  return KIOConnectionSignals::getSize( _url );
}

bool KIOJobBase::put( const char *_url, int _mode, bool _overwrite, bool _resume, int _size )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_PUT;
  
  m_bIsReady = false;
  m_bError = false;
  
  return KIOConnectionSignals::put( _url, _mode, _overwrite, _resume, _size );
}

bool KIOJobBase::mkdir( const char *_url, int _mode )
{
  assert( m_cmd == CMD_NONE );
  m_cmd = CMD_MKDIR;
  
  m_bIsReady = false;
  m_bError = false;
  
  return KIOConnectionSignals::mkdir( _url, _mode );
}

bool KIOJobBase::data( const void *_p, int _len )
{
  assert( m_cmd == CMD_PUT && m_bIsReady );
  
  return KIOConnectionSignals::data( _p, _len );
}

void KIOJobBase::slotReady()
{
  m_bIsReady = true;
}

void KIOJobBase::slotFinished()
{
  m_cmd = CMD_NONE;
  m_bIsReady = false;
}
  
void KIOJobBase::slotError( int _errid, const char *_txt )
{
  m_bError = true;
  m_iError = _errid;
  m_strError = _txt;
}
