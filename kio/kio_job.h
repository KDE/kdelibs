#ifndef __kio_job_h__
#define __kio_job_h__

#include <map>
                     
#include "kio_base.h"

#include <kurl.h>

#include <qobject.h>
#include <qstring.h>
#include <qstrlist.h>
#include <qdatetime.h>

#include <time.h>

class KIOSimpleProgressDlg;
class KIOListProgressDlg;
class KIOLittleProgressDlg;

class QSocketNotifier;
class QDialog;

/** 
* This is main class for doing IO operations.
*
* Use this class if you need to do any file transfer using various transfer protocols.
* Simply create new instance of this class, connect your custom slots to KIOJob signals
* and then call methods like copy, get, mount etc.
*
* KIOJob by default shows progress dialog for these operations ( this feature can be turned off ).
*
* KIOJob emits signals for almost all events that happen during transfer. Utilize these for
* more sophisticated control over transfer. The most important are @ref #sigFinished and
* @ref #sigError
*
* @short A main class for doing IO operations.
*/ 
class KIOJob : public QObject, public IOJob
{

  Q_OBJECT

public:

  KIOJob(const char *name = 0);
  virtual ~KIOJob();
  
  int id() { return m_id; }
  
  enum GUImode { NONE, SIMPLE, LIST, LITTLE };

  /**
   * AutoDelete mode is turned on by default. It is not recommended
   * to turn it off at all. Later versions may not even be able
   * to turn it off at all.
   */
  void setAutoDelete( bool _mode ) { m_bAutoDelete = _mode; }

  /**
   * Use this to set whether KIOJob should cache slaves into the pool.
   * Caching means, that when slave is done it is cached for next use.
   * This also means, that it is kept in the memory.
   *
   * @param  _mode  if true - cache slaves to the pool. This is a default value.
   *                if false - don't cache them but destroy immediately after it's done.
   */
  void cacheToPool( bool _mode ) { m_bCacheToPool = _mode; }


  /**
   * Specify what type of GUI will this KIOJob use.
   * Call this before you call any other operation method ( copy, move, del etc. )
   * Valid values are NONE, SIMPLE, LIST and LITTLE
   *
   * @param  _mode  NONE   - don't show any dialogs.
   *                SIMPLE - show a simple progress dialog. It shows progress for one operation.
   *                LIST   - show a list progress dialog. It shows progress for all operations.
   *                LITTLE - uses a little progress widget to display progress. This widget has to
   *                         be created in the caller application and is meant for inclusion in the
   *                         statusbar
   *
   * @see #showSimpleGUI #hideSimpleGUI #showListGUI #hideListGUI
   */
  void setGUImode( GUImode _mode );

  /**
   * Use this to set whether progress dialogs will start iconified or no.
   * Call this before you call any other command ( like copy etc. )
   * Default value is false = do not start iconified.
   *
   * @see #iconifySimpleGUI #iconifyListGUI
   */
  void startIconified( bool _mode ) { m_bStartIconified = _mode; }

  /**
   * Show / hide simple progress dialog.
   *
   * @param  _mode  if true - show dialog. If it doesn't exist, it will be created.
   *                if false - hide and destroy dialog.
   */
  void showSimpleGUI( bool _mode );

  /**
   * Iconify / deiconify simple progress dialog.
   *
   * @param  _mode  if true - iconify dialog.
   *                if false - deiconify dialog.
   */
  void iconifySimpleGUI( bool _mode );

  /**
   * Show / hide list progress dialog.
   *
   * @param  _mode  if true - show dialog. If it doesn't exist, it will be created.
   *                if false - hide and destroy dialog.
   */
  static void showListGUI( bool _mode );

  /**
   * Iconify / deiconify list progress dialog.
   *
   * @param  _mode  if true - iconify dialog.
   *                if false - deiconify dialog.
   */
  static void iconifyListGUI( bool _mode);

  /**
   * Dock / undock list progress dialog.
   *
   * @param  _mode  if true - dock dialog.
   *               if false - undock dialog.
   */
  static void dockListGUI( bool _mode);

  /**
   * Connect specified little progress dialog with this KIOJob.
   *
   * @param  _dlg   pointer to KIOLittleProgressDlg. This progress dialog is normally created
   *                in the caller application that also creates this KIOJob.
   */
  void connectProgress( KIOLittleProgressDlg *dlg );

  virtual bool copy( QStringList& _source, const char *_dest, bool _move = false );
  virtual bool copy( QStrList& _source, const char *_dest, bool _move = false );
  virtual bool copy( const char* _source, const char *_dest, bool _move = false );

  virtual bool move( QStringList& _source, const char *_dest );
  virtual bool move( QStrList& _source, const char *_dest );
  virtual bool move( const char* _source, const char *_dest );

  virtual bool del( QStringList& _source );
  virtual bool del( QStrList& _source );
  virtual bool del( const char* _source );

  virtual bool get( const char *_url );
  virtual bool getSize( const char *_url );

  virtual bool put( const char *_url, int _mode, bool _overwrite,
		            bool _resume, int _len );

  virtual bool listDir( const char *_url );
  /**
   * Tests whether _url is a directory.
   * Will emit @ref #sigIsDirectory or @ref #sigIsFile depending on which one it is.
   */
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
  virtual void slotListEntry( const UDSEntry& _entry );
  virtual void slotMimeType( const char *_type );
  virtual void slotRedirection( const char *_url );

  /**
   * Stops the current action ( that means kills any running servers associated with
   * this job and deletes itself ignoring whether auto-delete mode is on or off.
   *
   * @param  quiet  if true - KIOJob will not emit sigCanceled when killed. This is a default value.
   */
  virtual void kill( bool quiet = false );
  
  static KIOJob* find( int id );

  static QString findDeviceMountPoint( const char *device, const char *file = "/etc/mtab" );

  /**
   * Convert size from bytes to the string representation.
   *
   * @param  _size  size in bytes
   * @return  converted size as a string - e.g. 123.4 kB , 12 MB
   */
  static QString convertSize( int size );

  friend KIOSimpleProgressDlg;
  friend KIOListProgressDlg;
  friend KIOLittleProgressDlg;

signals:

  /**
   * KIOJob has stopped because of error.
   *
   * @param  id     id number of this KIOJob.
   * @param  errid  id number of the error.
   * @param  txt    additional text message for the error.
   *
   * @see kio_interface.h
   */
  void sigError( int id, int errid, const char *txt );

  /**
   * KIOJob has finished.
   *
   * @param  id  id number of this KIOJob.
   */
  void sigFinished( int id );

  /**
   * KIOJob has been canceled.
   *
   * @param  id  id number of this KIOJob.
   */
  void sigCanceled( int id );

  void sigListEntry( int id, const UDSEntry& entry );
  void sigMimeType( int id, const char *mimetype );

  /**
   * The saved file has been renamed.
   *
   * @param  id    id number of this KIOJob.
   * @param  name  new name of the destination file ( Usually from rename dialog ).
   */
  void sigRenamed( int id, const char *name );

  /**
   * Copying has been started.
   *
   * @param  id    id number of this KIOJob.
   * @param  from  a source name.
   * @param  to    a destination name.
   */
  void sigCopying(int id, const char *from, const char *to );

  /**
   * KIOJob can / cannot be resumed.
   *
   * @param  id      id number of this KIOJob.
   * @param  resume  if true - this KIOJob can be resumed.
   *                 if false - this KIOJob cannot be resumed.
   */
  void sigCanResume( int id, bool resume );

  /**
   * @param  id    id number of this KIOJob.
   * @param  data  may be 0L if the file has zero size.
   */
  void sigPreData( int id, const char *data, int len );
  void sigData( int id, const char *data, int len );
  void sigRedirection( int id, const char *url );
  void sigIsDirectory( int id );
  void sigIsFile( int id );
  
  /**
   * Current speed of the transfer in bytes per second.
   *
   * @param  id                id number of this KIOJob.
   * @param  bytes_per_second  speed in bytes per second.
   */
  void sigSpeed( int id, unsigned long bytes_per_second );

  /**
   * Total size of transfer ( counted recursively in case of directories ).
   *
   * @param  id     id number of this KIOJob.
   * @param  bytes  total size in bytes.
   */
  void sigTotalSize( int id, unsigned long bytes );

  /**
   * Total number of files in this transfer ( counted recursively ).
   *
   * @param  id     id number of this KIOJob.
   * @param  files  total number of files.
   */
  void sigTotalFiles( int id, unsigned long files );

  /**
   * Total number of directories in this transfer ( counted recursively ).
   *
   * @param  id    id number of this KIOJob.
   * @param  dirs  total number of directories.
   */
  void sigTotalDirs( int id, unsigned long dirs );

  /**
   * Already processed size in bytes.
   *
   * @param  id     id number of this KIOJob.
   * @param  bytes  processed size in bytes.
   */
  void sigProcessedSize( int id, unsigned long bytes );

  /**
   * Number of already transfered files.
   *
   * @param  id     id number of this KIOJob.
   * @param  files  number of processed files.
   */
  void sigProcessedFiles( int id, unsigned long files );

  /**
   * Number of already transfered directories.
   *
   * @param  id    id number of this KIOJob.
   * @param  dirs  number of processed directories.
   */
  void sigProcessedDirs( int id, unsigned long dirs );

protected slots:

  /**
   * Connected to the socket notifier
   *
   * @ref #m_pNotifier
   */
  virtual void slotDispatch( int );

  /**
   * Stops the current action ( that means kills any running servers associated with
   * this KIOJob and deletes itself if auto-delete mode is on.
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
  Slave* createSlave( const char *_protocol, int& _error, QString & _error_text );

  /**
   * Creates a new slave if the @ref KIOSlavePool has no matching one.
   * @ref m_pSlave and @ref m_strSlaveProtocol are set accordingly on success.
   * 
   * @param _error is the error code on failure and undefined else.
   * @param _error_text is the error text on failure and undefined else.
   *
   * @return @ref m_pSlave on success or 0L on failure.
   */
  Slave* createSlave(  const char *_protocol, const char *_host, const char *_user,
		       const char *_pass, int& _error, QString& _error_text );

  QDialog* createDialog( const char *_text );
  
  void createGUI();

  static void initStatic();

  bool m_bAutoDelete;

  bool m_bStartIconified;

  bool m_bCacheToPool;
  
  int m_iGUImode;

  KIOSimpleProgressDlg* m_pSimpleProgressDlg;
  KIOLittleProgressDlg* m_pLittleProgressDlg;

  QDialog *m_pDialog;
  
  Slave* m_pSlave;
  QSocketNotifier* m_pNotifier;
  QString m_strSlaveProtocol;
  QString m_strSlaveHost;
  QString m_strSlaveUser;
  QString m_strSlavePass;

  unsigned long m_iTotalSize;
  unsigned long m_iTotalFiles;
  unsigned long m_iTotalDirs;
  unsigned long m_iProcessedSize;
  unsigned long m_iProcessedFiles;
  unsigned long m_iProcessedDirs;
  unsigned long m_iSpeed;
  QTime m_RemainingTime;
  bool m_bStalled;

  bool m_bCanResume;
  QString m_strFrom;
  QString m_strTo;
  QString m_strDir;

  /**
   * Used in @ref #preget
   */
  bool m_bPreGet;
  char* m_pPreGetBuffer;
  int m_iPreGetBufferSize;
  int m_iPreGetBufferMaxSize;
  bool m_bPreGetFinished;
  QString m_strPreGetMimeType;
  
  int m_id;
  static int s_id;

  static map<int,KIOJob*>* s_mapJobs;
  static KIOListProgressDlg* m_pListProgressDlg;
};


/**
 * Implements a "last recently used" algorithm.
 */
class KIOSlavePool
{

public:

  KIOSlavePool() { }
  
  Slave* slave( const char *_protocol );
  Slave* slave( const char *_protocol, const char *_host, const char *_user,
		const char *_pass);

  void addSlave( Slave *_slave, const char *_protocol, const char *_host,
		 const char *_user, const char *_pass );
  
  static KIOSlavePool* self();

protected:

  void eraseOldest();
  
  struct Entry
  {
    time_t m_time;
    Slave* m_pSlave;
    QString m_host;
    QString m_user;
    QString m_pass;
  };
  
  multimap<QString,Entry> m_mapSlaves;

  static KIOSlavePool* s_pSelf;
};

#endif

