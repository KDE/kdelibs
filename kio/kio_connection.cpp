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
#ifdef HAVE_VFORK_H
#include <vfork.h>
#endif

#include <iostream.h>

#include "kio_connection.h"

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
    cerr << "Could not send header\n";
    return 0;
  }

  n = fwrite( _p, 1, _len, m_fout );

  if ( n != _len ) {
    cerr << "Could not write data\n";
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

  if ( n == -1) switch (errno) {
    case EAGAIN: cerr << "read failed, errno=EAGAIN" << endl;
    case EIO: cerr << "read failed, errno=EIO" << endl;
    case EISDIR: cerr << "read failed, errno=EISDIR" << endl;
    case EBADF: cerr << "read failed, errno=EBADF" << endl;
    case EINVAL: cerr << "read failed, errno=EINVAL" << endl;
    case EFAULT: cerr << "read failed, errno=EFAULT" << endl;
  }
  
  if ( n != 8 )
  {
    cerr << "No header " << n << endl;
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
    again2:
    n = ::read( m_in, m_pBuffer, len );
    if ( n == -1 && errno == EINTR )
      goto again2;
  
    if ( n == -1 )
    {
      cerr << "ERRNO is " << errno << endl;
      exit(3);
    }
  
    if ( n != len )
    {
      cerr << "Not enough data " << n << " instead of " << len << endl;
      return 0L;
    }

    m_pBuffer[ n ] = 0;
  }
  else
    m_pBuffer[ 0 ] = 0;
  
  *_cmd = cmd;
  *_len = len;
  
  return m_pBuffer;
}

KIOSlave::KIOSlave( const char *_cmd ) 
	: KIOConnection()
{
  // Indicate an error;
  m_pid = -1;
  
  int recv_in, recv_out;
  int send_in, send_out;
  
  if( !buildPipe( &recv_in, &send_in ) ) return;
  if( !buildPipe( &recv_out, &send_out ) ) return;
	
  QApplication::flushX();
  m_pid = vfork();
  if( m_pid == 0 ) {
    dup2( recv_in, 0 );	fcntl(0, F_SETFD, 0);
    dup2( send_out, 1 ); fcntl(1, F_SETFD, 0);
    close( recv_in );
    close( recv_out );
    close( send_in );
    close( send_out );

    char *argv[4] = { NULL, NULL, NULL, NULL };
    char *cmd = strdup( _cmd );
    argv[0] = cmd;
    execv( argv[0], argv );
    cerr << "Slave: exec failed for " << _cmd << endl;
    cerr << "PATH=" << (const char*)getenv("PATH") << endl;
    _exit( 0 );
  }
  close( recv_in );
  close( send_out );

  init( recv_out, send_in, KIOConnection::defaultBufferSize() );
}

KIOSlave::~KIOSlave()
{
  cerr <<  "KILLING SLAVE xb " << m_pid << endl;
  kill( m_pid, SIGTERM );
}

int KIOSlave::buildPipe( int *_recv, int *_send )
{
  int pipe_fds[2];
  if( pipe( pipe_fds ) != -1 )
  {
    *_recv = pipe_fds[0];
    *_send = pipe_fds[1];
    return 1;
  }
  return 0;
}
