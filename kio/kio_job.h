#ifndef __kio_job_h__
#define __kio_job_h__

#include "kio_base.h"

#include <qobject.h>
#include <qstring.h>
#include <qstrlist.h>

#include <map>
#include <list>
#include <string>

#include <time.h>

class KIOCopyProgressDlg;
class QSocketNotifier;
class QDialog;

class KIOJob : public QObject, public IOJob
{
  Q_OBJECT
public:
  KIOJob();
  virtual ~KIOJob();
  
  int id() { return m_id; }
  
  /**
   * AutoDelete mode is turned on by default. It is not recommended
   * to turn it off at all. Later versions may not even be able
   * to turn it off at all.
   */
  void setAutoDelete( bool _mode ) { m_bAutoDelete = _mode; }
  void enableGUI( bool _mode ) { m_bGUI = _mode; }
  
  virtual bool copy( list<string>& _source, const char *_dest );
  virtual bool copy( QStrList& _source, const char *_dest );
  virtual bool copy( const char* _source, const char *_dest );
  virtual bool get( const char *_url );
  virtual bool listDir( const char *_url );
  virtual bool testDir( const char *_url );
  virtual bool mount( bool _ro, const char *_fstype, const char* _dev, const char *_point );
  virtual bool unmount( const char *_point );
  
  /**
   * Starts fetching '_url' and buffers _max_len characters or some more
   * if available. These are sent using @ref #sigPreData. If we know
   * about the mimetype ( example HTTP protocol ) then @ref #sigMimeType
   * is called and no data is buffered.
   * The connection is sleeping until someone calls @ref #cont.
   */
  virtual bool preget( const char *_url, int _max_len );
  /**
   * Call only after using preget and after you received either
   * the signal @ref #sigMimeType or @ref #sigPreData.
   * Afte calling this functions the KIOJob behaves like calling
   * @ref #get. All buffered data is emitted using @ref #sigData now.
   */
  virtual void cont();
  
  virtual void slotData( void *_p, int _len );
  virtual void slotError( int _errid, const char *_txt );
  virtual void slotFinished();
  virtual void slotIsDirectory();
  virtual void slotIsFile();
  virtual void slotRenamed( const char *_new );
  virtual void slotResume( bool _resume );
  
  virtual void slotTotalSize( unsigned long _bytes );
  virtual void slotTotalFiles( unsigned long _files );
  virtual void slotTotalDirs( unsigned long _dirs );
  virtual void slotProcessedSize( unsigned long _bytes );
  virtual void slotProcessedFiles( unsigned long _files );
  virtual void slotProcessedDirs( unsigned long _dirs );
  virtual void slotScanningDir( const char *_dir );
  virtual void slotSpeed( unsigned long _bytes_per_second );
  virtual void slotCopyingFile( const char *_from, const char *_to );
  virtual void slotMakingDir( const char *_dir );
  virtual void slotGettingFile( const char *_url );
  virtual void slotListEntry( UDSEntry& _entry );
  virtual void slotMimeType( const char *_type );
  virtual void slotRedirection( const char *_url );

  /**
   * Stops the current action ( that means kills any running servers associated with
   * this job and deletes itself ignoring wether auto-delete mode is on or off.
   */
  virtual void kill();
  
  /**
   * Call this before you use any other function of this class and before you create
   * an instance of this class.
   */
  static void initStatic();
  static KIOJob* find( int _id );

  static QString findDeviceMountPoint( const char *_device, const char *_file = "/etc/mtab" );

signals:
  void sigError( int id, int _errid, const char *_txt );
  void sigFinished( int id );
  void sigListEntry( int id, UDSEntry& _entry );
  void sigMimeType( int id, const char *_mimetype );
  void sigRenamed( int id, const char *_new );
  void sigCopying(int id, const char *_file );
  void sigResumed( int id, bool _resume );

  /**
   * @param _data may be 0L if the file has zero size.
   */
  void sigPreData( int id, const char *_data, int _len );
  void sigData( int id, const char *_data, int _len );
  void sigRedirection( int id, const char *_url );
  void sigIsDirectory( int id );
  void sigIsFile( int id );
  
  void sigSpeed( int id, unsigned long _bytes_per_second );
  void sigTotalSize( int id, unsigned long _bytes );
  void sigTotalFiles( int id, unsigned long _files );
  void sigTotalDirs( int id, unsigned long _dirs );
  void sigProcessedSize( int id, unsigned long _bytes );
  void sigProcessedFiles( int id, unsigned long _files );
  void sigProcessedDirs( int id, unsigned long _dirs );

protected slots:
  /**
   * Connected to the socket notifier
   *
   * @ref #see m_pNotifier
   */
  virtual void slotDispatch( int );

  /**
   * Stops the current action ( that means kills any running servers associated with
   * this job and deletes itself if auto-delete mode is on.
   */
  virtual void slotCancel();
  
protected:
  /**
   * Cleanup function used in the destructor.
   */
  void clean();
  void connectSlave( Slave *_s );
  /**
   * Creates a new slave if the @ref KIOSlavePool has no matching one.
   * @ref m_pSlave and @ref m_strSlaveProtocol are set accordingly on success.
   * 
   * @param _error is the error code on failure and undefined else.
   * @param _error_text is the error text on failure and undefined else.
   *
   * @return @ref m_pSlave on success or 0L on failure.
   */
  Slave* createSlave( const char *_protocol, int& _error, string& _error_text );

  QDialog* createDialog( const char *_text );
  
  bool m_bAutoDelete;
  bool m_bGUI;
  
  KIOCopyProgressDlg* m_pCopyProgressDlg;
  QDialog *m_pDialog;
  
  Slave* m_pSlave;
  QSocketNotifier* m_pNotifier;
  string m_strSlaveProtocol;

  /**
   * Used in @ref #preget
   */
  bool m_bPreGet;
  char* m_pPreGetBuffer;
  int m_iPreGetBufferSize;
  int m_iPreGetBufferMaxSize;
  bool m_bPreGetFinished;
  string m_strPreGetMimeType;
  
  int m_id;
  static int s_id;

  static map<int,KIOJob*>* s_mapJobs;
};

/**
 * Implements a "last recently used" algorithm.
 */
class KIOSlavePool
{
public:
  KIOSlavePool() { }
  
  Slave* slave( const char *_protocol );
  void addSlave( Slave *_slave, const char *_protocol );
  
  static KIOSlavePool* self();
  
protected:
  void eraseOldest();
  
  struct Entry
  {
    time_t m_time;
    Slave* m_pSlave;
  };
  
  multimap<string,Entry> m_mapSlaves;

  static KIOSlavePool* s_pSelf;
};

#endif

