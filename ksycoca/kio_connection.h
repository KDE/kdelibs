#ifndef __protocol_h__
#define __protocol_h__ "$Id$"

#include <sys/types.h>

#include <stdio.h>

/**
  * This class provides a simple means for IPC between two applications
  * via a pipe.
  */
class KIOConnection
{
public:
  KIOConnection();
  KIOConnection( int _in_fd, int _out_fd, size_t _buf_len = defaultBufferSize());
  virtual ~KIOConnection();
  
  int inFD() { return m_in; }
  int outFD() { return m_out; }

  int send( int _cmd, const void *_p, int _len );
  void* read( int* _cmd, int* _len );

  bool eof() { return feof( m_fin ); }

  /**
    * Return a writable pointer to the buffer.
    */
  char* buffer() { return m_pBuffer; }

  /**
    * Return the current size of the buffer, in bytes.
    */
  size_t bufferSize() {return m_iBufferSize;}

protected:
  void init( int _in_fd, int _out_fd, size_t _buf_len );
  /**
    * Return the default size of the buffer.  This is useful when
    * constructing a "Connection", but rarely useful otherwise.
    */
  static size_t defaultBufferSize();

protected:
  int m_in;
  int m_out;
  FILE *m_fin;
  FILE *m_fout;

  char* m_pBuffer;
  size_t m_iBufferSize;
};

class KIOSlave : public KIOConnection
{
public:
  KIOSlave( const char *_cmd );
  ~KIOSlave();
 
  int pid() { return m_pid; }
  
protected:
  int buildPipe( int *_recv, int *_send );

  int m_pid;
};

#endif
