#ifndef __filter_h__
#define __filter_h__

class KIOFilter
{
public:
  /**
   * If the arguments parameter is specified, it must be a null
   * terminated list. ie. the last element of the array should 
   * be null. 
   */
  KIOFilter( const char *_cmd, const char **arguments=0 );
  virtual ~KIOFilter();
 
  int pid() { return m_pid; }
 
  bool send( void *_p, int _len );
  bool finish();
  
protected:
  virtual void emitData( void *_p, int _len );
  
private:
  int buildPipe( int *_recv, int *_send );
  
  int m_pid;

  int recv_in;
  int recv_out;
  int send_in;
  int send_out;
};

#endif
