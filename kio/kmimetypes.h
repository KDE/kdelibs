#ifndef __kmimetypes_h__
#define __kmimetypes_h__

#include <sys/types.h>
#include <sys/stat.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qdict.h>
#include <qmap.h>

#include <kurl.h>
#include <ksimpleconfig.h>


#include "ktypecode.h"
#include "kservicetype.h"

class KServiceTypeFactory;

/**
 * Represents a mime type.
 *
 * IMPORTANT : to use the public static methods of this class, you must do 
 * the following registry initialisation (in main() for instance)
 * <pre>
 * #include <kregistry.h>
 * #include <kregfactories.h> 
 *
 *   KRegistry registry;
 *   registry.addFactory( new KServiceTypeFactory );
 *   registry.load();
 * </pre>
 */
class KMimeType : public KServiceType
{
  K_TYPECODE( TC_KMimeType );

public:
  KMimeType( const QString& _type, const QString& _icon, const QString& _comment,
	     const QStringList& _patterns );
  KMimeType( KSimpleConfig& _cfg );
  KMimeType( QDataStream& _str );
  KMimeType();
  virtual ~KMimeType();
  
  /**
   * @param _url may be empty
   */
  virtual QString icon( const QString& _url, bool _is_local ) const { return m_strIcon; }
  virtual QString icon( const KURL& _url, bool _is_local ) const { return m_strIcon; }
  /**
   * @param _url may be 0L
   */
  virtual QString comment( const QString& _url, bool _is_local ) const { return m_strComment; }
  virtual QString comment( const KURL& _url, bool _is_local ) const { return m_strComment; }
  /**
   * @depreciated
   * 
   * Use @ref KServiceType::name instead.
   */
  virtual QString mimeType() const { return m_strName; }
  
  virtual const QStringList& patterns() const { return m_lstPatterns; }
  
  /**
   * Looks whether the given filename matches this mimetypes extension patterns.
   *
   * @param _filename is the real decoded filename or the decoded path without trailing '/'.
   *
   * @see #m_lstPatterns
   */
  virtual bool matchFilename( const QString&_filename ) const;

  virtual void load( QDataStream& );
  virtual void save( QDataStream& ) const;

  virtual PropertyPtr property( const QString& _name ) const;
  virtual QStringList propertyNames() const;
  
  /**
   * @return a pointer to the mime type '_name' or a pointer to the default
   *         mime type "application/octet-stream". 0L is NEVER returned.
   *
   * @deprecated Use @ref #mimeType instead
   */
  static KMimeType* find( const QString& _name ) { return mimeType( _name ); }
  /**
   * @return a pointer to the mime type '_name' or a pointer to the default
   *         mime type "application/octet-stream". 0L is NEVER returned.
   *
   * @see KServiceType::serviceType
   */
  static KMimeType* mimeType( const QString& _name );
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
  static KMimeType* findByURL( const KURL& _url, mode_t _mode = 0,
			       bool _is_local_file = false, bool _fast_mode = false );

  /**
   * Get all the mimetypes dict. Useful for showing the list of available mimetypes.
   * The returned dict contains a subset of the entries returned by @ref KServiceType::serviceTypes
   */
  static const QDict<KMimeType>& mimeTypes() { return *s_mapMimeTypes; }

protected:
  /**
   * Signal a missing mime type
   */
  static void errorMissingMimeType( const QString& _type );

  /**
   * Check for static variables initialised. Called by constructor
   * and by check().
   */
  static void initStatic();
  /**
   * Called after the registry parsed all mimetype config files.
   * This function makes sure that vital mime types are installed.
   * It must be called by all public static methods
   */
  static void check();
  /**
   * True if check for vital mime types has been done
   */
  static bool s_bChecked;

  QStringList m_lstPatterns;
  
  static QDict<KMimeType>* s_mapMimeTypes;
  static KMimeType* s_pDefaultType;
};

class KFolderType : public KMimeType
{
  K_TYPECODE( TC_KFolderType );

public:
  KFolderType( const QString& _type, const QString& _icon, const QString& _comment,
	       const QStringList& _patterns );
  KFolderType( KSimpleConfig& _cfg ) : KMimeType( _cfg ) { }
  KFolderType( QDataStream& _str ) : KMimeType( _str ) { }
  KFolderType() : KMimeType() { }

  virtual QString icon( const QString& _url, bool _is_local ) const;
  virtual QString icon( const KURL& _url, bool _is_local ) const;
  virtual QString comment( const QString& _url, bool _is_local ) const;
  virtual QString comment( const KURL& _url, bool _is_local ) const;
};

class KDELnkMimeType : public KMimeType
{
  K_TYPECODE( TC_KDELnkMimeType );

public:
  enum ServiceType { ST_MOUNT, ST_UNMOUNT, /* ST_PROPERTIES, */ ST_USER_DEFINED };
		     
  struct Service
  {
    QString m_strName;
    QString m_strIcon;
    QString m_strExec;
    ServiceType m_type;
  };
  
  KDELnkMimeType( const QString& _type, const QString& _icon, const QString& _comment,
		  const QStringList& _patterns );
  KDELnkMimeType( KSimpleConfig& _cfg ) : KMimeType( _cfg ) { }
  KDELnkMimeType( QDataStream& _str ) : KMimeType( _str ) { }
  KDELnkMimeType() : KMimeType() { }

  virtual QString icon( const QString& _url, bool _is_local ) const;
  virtual QString icon( const KURL& _url, bool _is_local ) const;
  virtual QString comment( const QString& _url, bool _is_local ) const;
  virtual QString comment( const KURL& _url, bool _is_local ) const;

  static QValueList<Service> builtinServices( const KURL& _url );
  static QValueList<Service> userDefinedServices( const KURL& _url );

  /**
   * @param _url is the URL of the kdelnk file. The URL must be local, otherwise
   *             nothing will happen.
   */
  static void executeService( const QString& _url, KDELnkMimeType::Service& _service );

  /**
   * Invokes the default action for the kdelnk file. If the kdelnk file
   * is not local, then only false is returned. Otherwise we would create
   * a security problem. Only types Link and Mimetype could be followed.
   *
   * @return true on success and false on failure.
   *
   * @see KRun::runURL
   */
  static bool run( const QString& _url, bool _is_local );

protected:
  static bool runFSDevice( const QString& _url, KSimpleConfig &cfg );
  static bool runApplication( const QString& _url, KSimpleConfig &cfg );
  static bool runLink( const QString& _url, KSimpleConfig &cfg );
  static bool runMimeType( const QString& _url, KSimpleConfig &cfg );
};

class KExecMimeType : public KMimeType
{
  K_TYPECODE( TC_KExecMimeType );

public:
  KExecMimeType( const QString& _type, const QString& _icon, const QString& _comment,
		 const QStringList& _patterns );
  KExecMimeType( KSimpleConfig& _cfg ) : KMimeType( _cfg ) { }
  KExecMimeType( QDataStream& _str ) : KMimeType( _str ) { }
  KExecMimeType() : KMimeType() { }
};

#endif

