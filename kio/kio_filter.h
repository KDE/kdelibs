#ifndef __filter_h__
#define __filter_h__

class Filter
{
public:
  Filter( const char *_cmd );
  virtual ~Filter();
 
  int pid() { return m_pid; }
 
  bool send( void *_p, int _len );
  bool finish();
  
protected:
  virtual void emitData( void *_p, int _len );
  
private:
  int buildPipe( int *_recv, int *_send );
  
  int m_pid;

  int recv_in, recv_out;
  int send_in, send_out;
};

#endif
