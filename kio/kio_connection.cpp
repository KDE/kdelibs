// $Id$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/time.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "kio_connection.h"

#include <kdebug.h>

KIOConnection::KIOConnection( int _in_fd, int _out_fd, size_t _buf_len )
{
  init( _in_fd, _out_fd, _buf_len );
}

KIOConnection::KIOConnection()
{
  m_fin = m_fout = 0L;
  m_pBuffer = 0L;
}

size_t KIOConnection::defaultBufferSize()
{
  return 0xFFFF;
}

void KIOConnection::init( int _in_fd, int _out_fd , size_t _buf_len)
{
  m_in = _in_fd;
  m_out = _out_fd;
  
  m_fin = fdopen( _in_fd, "rb" );
  m_fout = fdopen( _out_fd, "wb" );

  m_pBuffer = (char *)malloc(_buf_len);
  m_iBufferSize = _buf_len;
}

KIOConnection::~KIOConnection()
{
  if (m_fin)
    fclose(m_fin);
  if (m_fout)
    fclose(m_fout);
  
  if (m_pBuffer)
    free(m_pBuffer);
}

int KIOConnection::send( int _cmd, const void *_p, int _len )
{
  static char buffer[ 64 ];
  sprintf( buffer, "%4x_%2x_", _len, _cmd );

  int n = fwrite( buffer, 1, 8, m_fout );

  if ( n != 8 ) {
    kdebug( KDEBUG_ERROR, 7017, "Could not send header");
    return 0;
  }

  n = fwrite( _p, 1, _len, m_fout );

  if ( n != _len ) {
    kdebug( KDEBUG_ERROR, 7017, "Could not write data");
    return 0;
  }

  fflush( m_fout );

  return 1;
}

void* KIOConnection::read( int* _cmd, int* _len )
{
  static char buffer[ 8 ];

again1:
  int n = ::read( m_in, buffer, 8 );
  if ( n == -1 && errno == EINTR )
    goto again1;

  if ( n == -1) {
    kdebug( KDEBUG_ERROR, 7017, "Header read failed, errno=%d", errno);
  }
  
  if ( n != 8 )
  {
    //kdebug( KDEBUG_ERROR, 7017, "Header has invalid size (%d)", n);
    return 0L;
  }
  
  buffer[ 4 ] = 0;
  buffer[ 7 ] = 0;
  
  char *p = buffer;
  while( *p == ' ' ) p++;
  long int len = strtol( p, 0L, 16 );

  p = buffer + 5;
  while( *p == ' ' ) p++;
  long int cmd = strtol( p, 0L, 16 );

  if ( len > 0L )
  {
    int bytesToGo = len;
    int bytesRead = 0;
    do 
    {
       n = ::read(m_in, m_pBuffer+bytesRead, bytesToGo);
       if (n == -1) 
       { 
          if (errno == EINTR)
             continue;

          kdebug( KDEBUG_ERROR, 7017, "Data read failed, errno=%d", errno);
          return 0;
       }
       if (n != bytesToGo)
       {
          kdebug( KDEBUG_INFO, 7017, "Not enough data read (%d instead of %d) cmd=%ld", n, bytesToGo, cmd);
       }

       bytesRead += n;
       bytesToGo -= n;
    }
    while(bytesToGo);
    m_pBuffer[ len ] = 0;
  }
  else
    m_pBuffer[ 0 ] = 0;
  
  *_cmd = cmd;
  *_len = len;
  
  return m_pBuffer;
}

KIOSlave::KIOSlave( const char *_cmd ) 
	: KIOConnection(), KProcess()
{
  *this << _cmd;

  if ( !start(NotifyOnExit, (Communication) (Stdin | Stdout | NoRead)) )
  {
      kdebug( KDEBUG_ERROR, 7016, "Couldn't execute %s.", _cmd);
      return;
  }

  init( out[0], in[1], KIOConnection::defaultBufferSize() );
}

KIOSlave::~KIOSlave()
{
  if (isRunning())
  {
     kdebug( KDEBUG_ERROR, 7016, "Killing running slave pid = %ld", getPid());
  }
}

