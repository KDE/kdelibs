#ifndef __protocol_h__
#define __protocol_h__

#include <stdio.h>

class Connection
{
public:
  Connection( int _in_fd, int _out_fd );
  virtual ~Connection();
  
  int inFD() { return m_in; }
  int outFD() { return m_out; }

  int send( int _cmd, const void *_p, int _len );
  void* read( int* _cmd, int* _len );
  
  bool eof() { return feof( m_fin ); }
  
  char* buffer() { return m_pBuffer; }
  
protected:
  Connection();
  
  void init( int _in_fd, int _out_fd );

  int m_in;
  int m_out;
  FILE *m_fin;
  FILE *m_fout;

  char* m_pBuffer;
};

class Slave : public Connection
{
public:
  Slave( const char *_cmd );
  ~Slave();
 
  int pid() { return m_pid; }
  
protected:
  int buildPipe( int *_recv, int *_send );
  
  int m_pid;
};

#endif
