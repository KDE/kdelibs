#ifndef __k_run_h__
#define __k_run_h__

/**
  * Danger Will Robinson... Do not use umode_t or anything else that is
  * defined in asm/types.h.  These typedefs are not portable and should be
  * avoided at all costs.
  */

class KService;

#include <string>
#include <list>

#include <sys/stat.h>

#include <qobject.h>
#include <qtimer.h>
#include <qstrlist.h>

/**
 * Implements a generic runner, i.e. the 'exec' functionality of KDE
 * It can execute any kdelnk, as well as any file, using default binding (service)
 * or another bound service.
 *
 * IMPORTANT : to use this class, you must do 
 * the following registry initialisation (in main() for instance)
 * <pre>
 * #include <kregistry.h>
 * #include <kregfactories.h> 
 *
 *   KRegistry registry;
 *   registry.addFactory( new KMimeTypeFactory );
 *   registry.addFactory( new KServiceFactory );
 *   registry.load();
 * </pre>

 */
class KRun : public QObject
{
  Q_OBJECT
public:
  /**
   * @param _mode is the st_mode field of <tt>struct stat</tt>. If you dont know this set it to 0.
   * @param _is_local_file if this parameter is set to false, then '_url' is examnined to find out
   *                       whether it is a local URL or not. This flag is just used to improve speed, since
   *                       the function @ref KURL::isLocalFile is a bit slow.
   */
  KRun( const QString& _url, mode_t _mode = 0, bool _is_local_file = false, bool _auto_delete = true );
  ~KRun();
  
  bool hasError() { return m_bFault; }
  bool hasFinished() { return m_bFinished; }
  
  /**
   * By default auto deletion is on.
   */
  bool autoDelete() { return m_bAutoDelete; }
  void setAutoDelete() { m_bAutoDelete = m_bAutoDelete; }

  /**
   * Opens a list of URLs with a certain service.
   * @param _service
   * @param _urls the list of URLs, can be empty (app launched without argument)
   */
  static bool run( const KService& _service, QStringList& _urls );
  /**
   * Opens a list of URLs with.
   *
   * @param _exec is the name of the executable, for example "/usr/bin/netscape".
   * @param _name is the logical name of the application, for example "Netscape 4.06"
   * @param _icon is the icon which should be used by the application
   * @param _miniicon is the icon which should be used by the application
   */
  static bool run( const QString& _exec, QStringList& _urls,
		   const QString& _name = QString::null,
		   const QString& _icon = QString::null,
		   const QString& _mini_icon = QString::null,
		   const QString& _kdelnk_file = QString::null );

  static bool runOldApplication( const QString& _exec, QStringList& _urls,
				 bool _allow_multiple );
  
signals:
  /**
   * Emitted when the runner process has finished. WARNING: Do not save
   * the pointer anywhere for future use, since when the slot that is
   * connected to this signal returns KRun will probably delete itself!
   * You should only need this if you allocate a KRun and then want to
   * forget about it until it is done. Otherwise use finished().
   */
  void finishedRef(KRun *);
  void finished();
  /**
   * Emitted when the runner process has encountered an error. WARNING: Do not
   * save the pointer anywhere for future use, since when the slot that is
   * connected to this signal returns KRun will probably delete itself!
   * You should only need this if you allocate a KRun and then want to
   * forget about it until it is done. Otherwise use error().
   */
  void errorRef(KRun *);
  void error();

protected slots:
  void slotTimeout();
  void slotMimeType( int _id, const char *_type );
  void slotPreData( int _id, const char *_data, int _len );
  void slotError( int _id, int _errid, const char *_errortext );
  void slotIsDirectory( int _id );
  void slotIsFile( int _id );
  void slotFinished( int _id );
  
protected:
  virtual void init();
  
  virtual void scanFile();
  
  /**
   * Called if the mimetype has been detected. The function checks whether the document
   * and appends the gzip protocol to the URL. Otherwise @ref #runURL is called to
   * finish the job.
   */
  virtual void foundMimeType( const char *_type );
  
  virtual void killJob();
  
  QString m_strURL;
  bool m_bFault;
  bool m_bAutoDelete;
  bool m_bFinished;
  int m_jobId;
  QTimer m_timer;

  /**
   * Used to indicate that the next action is to scan the file.
   * This action is invoked from @ref #slotTimeout.
   */
  bool m_bScanFile;
  bool m_bIsDirectory;
  
  /**
   * USed to indicate that the next action is to initialize.
   * This action is invoked from @ref #slotTimeout
   */
  bool m_bInit;
  
  bool m_bIsLocalFile;
  mode_t m_mode;
  
  /**
   * Open the given URL. This function is used after the mime type
   * is found out. It will search for all services which can handle
   * the mime type and call @ref #run afterwards.
   */
  static bool runURL( const char *_url, const char *_mimetype );

  /**
   * Runs a shell command.
   *
   * @ref _cmd must be a quoted shell command. You must not append "&" to it, since the
   *           function will do that for you. An example is "<tt>greet 'Hello Torben'</tt>".
   */
  static bool run( const QString& _cmd );
  
  /**
   * Quotes a string for the shell
   */
  static void shellQuote( QString &_str );
};


/*
 * This class handles the openFileManagerWindow call
 * It is not implemented in libkio, so any program that wants to use it
 * must derive from its class and override the method.
 * At least konqueror and libkfm will do.
 */ 
class KFileManager
{
public:
  KFileManager() { pFileManager = this; } ;
  virtual ~KFileManager() {} ;

  virtual void openFileManagerWindow( const char *_url ) = 0L;
  
  static KFileManager * getFileManager() { 
    if (!pFileManager)
      debug("PROGRAM ERROR : program uses a 'run' method in kio, but doesn't implement KFileManager::openFileManagerWindow(const char*)\n");
    return pFileManager;
  }

private:
  static KFileManager * pFileManager;
};

#endif
