// $Id$

#include "kio_connection.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

#include <iostream.h>

Connection::Connection( int _in_fd, int _out_fd )
{
  init( _in_fd, _out_fd );
}

Connection::Connection()
{
  m_fin = 0L;
  m_fout = 0L;
  m_pBuffer = 0L;
}

void Connection::init( int _in_fd, int _out_fd )
{
  m_in = _in_fd;
  m_out = _out_fd;
  
  m_fin = fdopen( _in_fd, "rb" );
  m_fout = fdopen( _out_fd, "wb" );

  m_pBuffer = new char[ 0xFFFF ];
}

Connection::~Connection()
{
  /* close( m_in );
  close( m_out ); */
  
  if ( m_fin )
    fclose( m_fin );
  if ( m_fout )
    fclose( m_fout );
  
  if ( m_pBuffer )
    delete [] m_pBuffer;
}

int Connection::send( int _cmd, const void *_p, int _len )
{
  static char buffer[ 100 ];
  sprintf( buffer, "%4x_%2x_", _len, _cmd );

  int n = fwrite( buffer, 1, 8, m_fout );
  
  if ( n != 8 )
  {
    cerr << "Could not send header\n";
    return 0;
  }
  
  n = fwrite( _p, 1, _len, m_fout );

  if ( n != _len )
  {
    cerr << "Could not write data\n";
    return 0;
  }
  
  fflush( m_fout );

  return 1;
}

void* Connection::read( int* _cmd, int* _len )
{
  static char buffer[ 8 ];
  
again1:
  int n = ::read( m_in, buffer, 8 );
  if ( n == -1 && errno == EINTR )
    goto again1;
  
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

Slave::Slave( const char *_cmd ) : Connection()
{
  // Indicate an error;
  m_pid = -1;
  
  int recv_in, recv_out;
  int send_in, send_out;
  
  if( !buildPipe( &recv_in, &send_in ) ) return;
  if( !buildPipe( &recv_out, &send_out ) ) return;

  m_pid = fork();
  if( m_pid == 0 )
  {
    dup2( recv_in, 0 );	fcntl(0,F_SETFD,0);
    dup2( send_out, 1 ); fcntl(1,F_SETFD,0);
    close( recv_in );
    close( recv_out );
    close( send_in );
    close( send_out );

    char *argv[4] = { NULL, NULL, NULL, NULL };
    char *cmd = strdup( _cmd );
    argv[0] = cmd;
    execv( argv[0], argv );
    cerr << "Slave: exec failed...!" << endl;
    cerr << "Have you installed kdebase?" << endl;
    exit( 0 );
  }
  close( recv_in );
  close( send_out );

  init( recv_out, send_in );
}

Slave::~Slave()
{
  cerr <<  "KILLING SLAVE xb " << m_pid << endl;
  kill( m_pid, SIGTERM );
}

int Slave::buildPipe( int *_recv, int *_send )
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
