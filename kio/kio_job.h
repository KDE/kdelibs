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

/** 
* This is main class for doing IO operations.
*
* Use this class if you need to do any file transfer using various transfer protocols.
*
* Simply create new instance of this class, connect your custom slots to KIOJob signals and then call methods like copy, get, mount etc.
*
* KIOJob by default shows progress dialog for these opreations ( this feature can be turned off ).
*
* KIOJob emits signals for almost all events that happen during transfer. Utilize these for more sophisticated control over transfer. The most important are @ref #sigFinished and @ref #sigError
*
* @short A main class for doing IO operations.
*/ 
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


  /**
   * Enable or disable progress dialogs. Dialogs are enabled per default.
   * Call this before you call any other command ( like copy etc. )
   * @see #showGUI #hideGUI
   */
  void enableGUI( bool _mode ) { m_bGUI = _mode; }

  /**
   * This flags determines, whether progress dialogs will start iconified or no.
   * Default value is normal mode ( not iconified ).
   * Call this before you call any other command ( like copy etc. )
   * @see #iconifyGUI
   */
  void startIconified( bool _mode ) { m_bStartIconified = _mode; }
  
  /**
   * Allows hiding dialogs from the program. Doesn't care whether GUI is enabled.
   */
  void hideGUI();

  /**
   * Allows showing dialogs from the program. Doesn't care whether GUI is enabled.
   */
  void showGUI();

  /**
   * Allows iconifying dialogs from the program. Doesn't care whether GUI is enabled.
   */
  void iconifyGUI();

  /**
   * This flags determines, whether KIOJob should cache slaves into the pool.
   * Default mode is yes - cache to pool.
   * When slave is done it is cached for next use. This also means, that it is kept in memory.
   *
   * Set this flag to false if you don't want this behaviour.
   */
  void cacheToPool( bool _mode ) { m_bCacheToPool = _mode; }

  virtual bool copy( list<string>& _source, const char *_dest, bool _move = false );
  virtual bool copy( QStrList& _source, const char *_dest, bool _move = false );
  virtual bool copy( const char* _source, const char *_dest, bool _move = false );

  virtual bool move( list<string>& _source, const char *_dest );
  virtual bool move( QStrList& _source, const char *_dest );
  virtual bool move( const char* _source, const char *_dest );

  virtual bool del( list<string>& _source );
  virtual bool del( QStrList& _source );
  virtual bool del( const char* _source );

  virtual bool get( const char *_url );
  virtual bool getSize( const char *_url );

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
  virtual void slotCanResume( bool _resume );
  
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
  virtual void slotDeletingFile( const char *_url );
  virtual void slotListEntry( UDSEntry& _entry );
  virtual void slotMimeType( const char *_type );
  virtual void slotRedirection( const char *_url );

  /**
   * Stops the current action ( that means kills any running servers associated with
   * this job and deletes itself ignoring wether auto-delete mode is on or off.
   * @param quiet whether to emit sigCanceled or not. Default is yes.
   */
  virtual void kill( bool quiet = false );
  
  /**
   * Call this before you use any other function of this class and before you create
   * an instance of this class.
   */
  static void initStatic();
  static KIOJob* find( int _id );

  static QString findDeviceMountPoint( const char *_device, const char *_file = "/etc/mtab" );

signals:

  void sigError( int id, int _errid, const char *_txt );

  /**
   * This job has finished
   * @param id is identification number for this job ( Needed if you use more jobs )
   */
  void sigFinished( int id );

  /**
   * This job has been canceled
   */
  void sigCanceled( int id );

  void sigListEntry( int id, UDSEntry& _entry );
  void sigMimeType( int id, const char *_mimetype );

  /**
   * The saved file has been renamed
   * @param _new is a new name of saved file ( Usually from rename dialog )
   */
  void sigRenamed( int id, const char *_new );

  /**
   * Emited when the copying has been started
   * @param _from is a source name
   * @param _to is a destination name
   */
  void sigCopying(int id, const char *_from, const char *_to );

  /**
   * This job can / cannot be resumed
   */
  void sigCanResume( int id, bool _resume );

  /**
   * @param _data may be 0L if the file has zero size.
   */
  void sigPreData( int id, const char *_data, int _len );
  void sigData( int id, const char *_data, int _len );
  void sigRedirection( int id, const char *_url );
  void sigIsDirectory( int id );
  void sigIsFile( int id );
  
  /**
   * Current speed of transfer in bytes per second
   */
  void sigSpeed( int id, unsigned long _bytes_per_second );

  /**
   * Total size of transfer ( counted recursively in case of directories )
   */
  void sigTotalSize( int id, unsigned long _bytes );

  /**
   * Total number of files in this transfer ( counted recursively )
   */
  void sigTotalFiles( int id, unsigned long _files );

  /**
   * Total number of directories in this transfer ( counted recursively )
   */
  void sigTotalDirs( int id, unsigned long _dirs );

  /**
   * Already processed size in bytes
   */
  void sigProcessedSize( int id, unsigned long _bytes );

  /**
   * Number of already transfered files
   */
  void sigProcessedFiles( int id, unsigned long _files );

  /**
   * Number of already transfered directories
   */
  void sigProcessedDirs( int id, unsigned long _dirs );

protected slots:
  /**
   * Connected to the socket notifier
   *
   * @ref #m_pNotifier
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
  bool m_bStartIconified;

  bool m_bCacheToPool;
  
  KIOCopyProgressDlg* m_pCopyProgressDlg;
  QDialog *m_pDialog;
  
  Slave* m_pSlave;
  QSocketNotifier* m_pNotifier;
  string m_strSlaveProtocol;

  unsigned long m_iTotalSize;
  unsigned long m_iTotalFiles;
  unsigned long m_iTotalDirs;
  unsigned long m_iProcessedSize;
  bool m_bCanResume;
  string m_strFrom;
  string m_strTo;

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

