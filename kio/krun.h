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

class KRun : public QObject
{
  Q_OBJECT
public:
  /**
   * @param _mode is the st_mode field of <tt>struct stat</tt>. If you dont know this set it to 0.
   * @param _is_local_file if this parameter is set to false, then '_url' is examnined to find out
   *                       wether it is a local URL or not. This flag is just used to improve speed, since
   *                       the function @ref K2URL::isLocalFile is a bit slow.
   */
  KRun( const char *_url, mode_t _mode = 0, bool _is_local_file = false, bool _auto_delete = true );
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
   */
  static bool run( KService* _service, QStrList& _urls );
  /**
   * Opens a list of URLs with.
   *
   * @param _exec is the name of the executable, for example "/usr/bin/netscape".
   * @param _name is the logical name of the application, for example "Netscape 4.06"
   * @param _icon is the icon which should be used by the application
   * @param _miniicon is the icon which should be used by the application
   */
  static bool run( const char *_exec, QStrList& _urls, const char *_name = "",
		   const char *_icon = "", const char *_mini_icon = "", const char *_kdelnk_file = "" );

  static bool runOldApplication( const char *_exec, QStrList& _urls, bool _allow_multiple );
  
signals:
  void finished();
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
   * Called if the mimetype has been detected. The function checks wether the document
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
  static bool run( const char *_cmd );
  
  /**
   * Quotes a string for the shell
   */
  static void shellQuote( string &_str );
};

#endif
