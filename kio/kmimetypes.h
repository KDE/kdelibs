#ifndef __kmimetypes_h__
#define __kmimetypes_h__

#include <sys/stat.h>

#include <list>
#include <string>
#include <map>

#include <qstring.h>
#include <qstrlist.h>
#include <qdict.h>

#include <k2url.h>
#include <ksimpleconfig.h>

class KMimeType
{
public:
  KMimeType( const char *_type, const char *_icon, const char *_comment, QStrList& _patterns );
  virtual ~KMimeType();
  
  /**
   * @param _url may be 0L
   */
  virtual QString icon( const char *_url, bool _is_local ) { return m_strIcon; }
  virtual QString icon( K2URL& _url, bool _is_local ) { return m_strIcon; }
  /**
   * @param _url may be 0L
   */
  virtual QString comment( const char *_url, bool _is_local ) { return m_strComment; }
  virtual QString comment( K2URL& _url, bool _is_local ) { return m_strComment; }
  virtual const char* mimeType() { return m_strMimeType; }
  
  virtual QStrList patterns() { return m_lstPatterns; }
  
  /**
   * Looks wether the given filename matches this mimetypes extension patterns.
   *
   * @param _filename is the real decoded filename or the decoded path without trailing '/'.
   *
   * @see #m_lstPatterns
   */
  virtual bool matchFilename( const char *_filename );

  /**
   * @return a pointer to the mime type '_name' or a pointer to the default mime type "application/octet-stream".
   *         0L is NEVER returned.
   */
  static KMimeType* find( const char *_name );
  /**
   * This function looks at mode_t first. If that does not help it looks at the extension. 
   * This is ok for FTP, FILE, TAR and friends, but is not for
   * HTTP ( cgi scripts! ). You should use @ref KfmRun instead, but this function returns immediately
   * while @ref KfmRun is async. If no extension matches, then @ref KMimeMagic is used if the URL a local
   * file or "application/octet-stream" is returned otherwise.
   *
   * @param _url is the right most URL with a filesystem protocol. It is up to you to
   *             find out about that if you have a nested URL.
   *             For example "http://localhost/mist.gz#gzip:/decompress"
   *             would have to pass the "http://..." URL part, while
   *             "file:/tmp/x.tar#tar:/src/test.gz#gzip:/decompress" would have
   *             to pass the "tar:/..." part of the URL, since gzip is a filter
   *             protocol and not a filesystem protocol.
   * @param _fast_mode If set to true no disk access is allowed to find out the mimetype. The result may be suboptimal,
   *                   but it is FAST.
   * @return a pointer to the matching mimetype. 0L is NEVER returned.
   */
  static KMimeType* findByURL( K2URL& _url, mode_t _mode = 0, bool _is_local_file = false, bool _fast_mode = false );

  /**
   * Called by the main function
   */
  static void initStatic();
  /**
   * Called by the main function after the registry parsed all mimetype
   * config files. This function makes shure that vital mime types are installed.
   */
  static void check();
  
protected:
  static void errorMissingMimeType( const char *_type );
  static void scanMimeTypes( const char* _path );

  QString m_strMimeType;
  QString m_strIcon;
  QString m_strComment;
  QStrList m_lstPatterns;
  
  static QDict<KMimeType>* s_mapTypes;
  static KMimeType* s_pDefaultType;
};

class KFolderType : public KMimeType
{
public:
  KFolderType( const char *_type, const char *_icon, const char *_comment, QStrList& _patterns );

  virtual QString icon( const char *_url, bool _is_local );
  virtual QString icon( K2URL& _url, bool _is_local );
  virtual QString comment( const char *_url, bool _is_local );
  virtual QString comment( K2URL& _url, bool _is_local );
};

class KDELnkMimeType : public KMimeType
{
public:
  enum ServiceType { ST_MOUNT, ST_UNMOUNT, /* ST_PROPERTIES, */ ST_USER_DEFINED };
		     
  struct Service
  {
    QString m_strName;
    QString m_strIcon;
    QString m_strExec;
    ServiceType m_type;
  };
  
  KDELnkMimeType( const char *_type, const char *_icon, const char *_comment, QStrList& _patterns );

  virtual QString icon( const char *_url, bool _is_local );
  virtual QString icon( K2URL& _url, bool _is_local );
  virtual QString comment( const char *_url, bool _is_local );
  virtual QString comment( K2URL& _url, bool _is_local );

  static void builtinServices( K2URL& _url, list<Service>& _lst );
  static void userDefinedServices( K2URL& _url, list<Service>& _lst );

  /**
   * @param _url is the URL of the kdelnk file. The URL must be local, otherwise
   *             nothing will happen.
   */
  static void executeService( const char *_url, KDELnkMimeType::Service& _service );

  /**
   * Invokes the default action for the kdelnk file. If the kdelnk file
   * is not local, then only false is returned. Otherwise we would create
   * a security problem. Only types Link and Mimetype could be followed.
   *
   * @return true on success and false on failure.
   *
   * @see KRun::runURL
   */
  static bool run( const char *_url, bool _is_local );

protected:
  static bool runFSDevice( const char *_url, KSimpleConfig &cfg );
  static bool runApplication( const char *_url, KSimpleConfig &cfg );
  static bool runLink( const char *_url, KSimpleConfig &cfg );
  static bool runMimeType( const char *_url, KSimpleConfig &cfg );
};

class KExecMimeType : public KMimeType
{
public:
  KExecMimeType( const char *_type, const char *_icon, const char *_comment, QStrList& _patterns );
};

#endif
