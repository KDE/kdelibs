// $Id$

#include "kio_filter.h"

#include <kdebug.h>

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <iostream.h>

Filter::Filter( const char *_cmd )
{
  // Indicate an error;
  m_pid = -1;
  send_in = -1;
  recv_out = -1;
  
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

  fcntl( recv_out, F_SETFL, O_NONBLOCK );
  fcntl( send_in, F_SETFL, O_NONBLOCK );
}

Filter::~Filter()
{
  if ( recv_out != -1 )
    close( recv_out );
  if ( send_in != -1 )
    close( send_in );
}

int Filter::buildPipe( int *_recv, int *_send )
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

bool Filter::send( void *_p, int _len )
{
  int written = 0;

  fd_set rfds;
  fd_set wfds;
  
  char buffer[ 2048 ];
  
  while( written < _len )
  {
    FD_ZERO( &rfds );
    FD_ZERO( &wfds );
    int max_fd = 0;
    
    FD_SET( recv_out, &rfds );
    max_fd = recv_out + 1;
    FD_SET( send_in, &wfds );
    if ( send_in + 1 > max_fd )
      max_fd = send_in + 1;

    int err;
    timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
  again:
    if ( ( err = select( max_fd, &rfds, &wfds, 0L, &tv ) ) == -1 && errno == EINTR )
      goto again;
    if ( err == -1 )
      assert( 0 );

    if ( FD_ISSET( send_in, &wfds ) )
    {
      int n;
    again1:
      if ( ( n = write( send_in, (void*)((char*)_p + written), _len - written ) ) == -1 && errno == EINTR )
	goto again1;
      else if ( n == -1 && errno != EAGAIN )
      {
	kdebug( KDEBUG_ERROR, 7005, "ERROR: Write" );
	return false;
      }
      
      if ( n != -1 )
	written += n;
    }
    if ( FD_ISSET( recv_out, &rfds ) )
    {
      int n;
      do
      {  
      again2:
	if ( ( n = read( recv_out, buffer, 2048 ) ) == -1 && errno == EINTR )
	  goto again2;
	else if ( n == -1 && errno != EAGAIN )
	{
	  kdebug( KDEBUG_ERROR, 7005, "ERROR: Read %d", errno );
	  return false;
	}
	
	if ( n != -1 )
	  emitData( buffer, n );
	
      } while( n > 0 );
    }
  }

  return true;
}

bool Filter::finish()
{
  close( send_in );
  send_in = -1;
  
  fcntl( recv_out, F_SETFL, 0 );

  char buffer[ 2048 ];

  int n;
  do
  {
  again3:
    if ( ( n = read( recv_out, buffer, 2048 ) ) == -1 && errno == EINTR )
      goto again3;

    if ( n != -1 )
      emitData( buffer, n );
    
  } while( n > 0 );

  return true;
}

void Filter::emitData( void *_p, int _len )
{
  assert( 0 );
  fwrite( _p, 1, _len, stderr );
}
