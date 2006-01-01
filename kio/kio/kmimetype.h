/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                     David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef __kmimetype_h__
#define __kmimetype_h__

#include <sys/types.h>
#include <sys/stat.h>

#include <qstringlist.h>
#include <QList>
#include <qpixmap.h>

#include <kicontheme.h>
#include <kurl.h>
#include <ksycocatype.h>
#include <kservicetype.h>

class KSimpleConfig;
/**
 * Represent a mime type, like "text/plain", and the data that is associated
 * with it.
 *
 * The starting point you need is often the static methods.
 *
 * KMimeType inherits KServiceType because "text/plain" can be used to find
 * services (apps and components) "which can open text/plain".
 *
 * @see KServiceType
 */
class KIO_EXPORT KMimeType : public KServiceType
{
  K_SYCOCATYPE( KST_KMimeType, KServiceType )

public:
  typedef KSharedPtr<KMimeType> Ptr;
  typedef QList<Ptr> List;
public:
  /**
   * Constructor.
   *
   * You may pass in arguments to create a mimetype with
   * specific properties.
   *
   * @param _fullpath the path to the configuration file (.desktop)
   * @param _type the mime type itself
   * @param _icon the name of the icon that represens the mime type
   * @param _comment a comment describing the mime type
   * @param _patterns a list of file globs that describes the names (or
   *                  extensions) of the files with this mime type
   */
  KMimeType( const QString & _fullpath, const QString& _type, const QString& _icon,
	     const QString& _comment, const QStringList& _patterns );

  /**
   * Construct a mimetype and take all information from a config file.
   * @param _fullpath the path to the configuration file (.desktop)
   */
  KMimeType( const QString & _fullpath );

  /**
   * Construct a mimetype and take all information from a desktop file.
   * @param config the desktop configuration file that describes the mime type
   */
  KMimeType( KDesktopFile *config );

  /**
   * @internal Construct a service from a stream.
   *
   * The stream must already be positionned at the correct offset
   */
  KMimeType( QDataStream& _str, int offset );

  virtual ~KMimeType();

  /**
   * Return the filename of the icon associated with the mimetype.
   *
   * The arguments are unused, but provided so that KMimeType-derived classes
   * can use them (e.g. KFolderType uses the URL to return one out of 2 icons)
   *
   * @return The path to the icon associated with this MIME type.
   */
  virtual QString icon( const QString& , bool ) const { return m_strIcon; }

  /**
   * Return the filename of the icon associated with the mimetype.
   *
   * The arguments are unused, but provided so that KMimeType-derived classes
   * can use them (e.g. KFolderType uses the URL to return one out of 2 icons)
   *
   * @return The path to the icon associated with this MIME type.
   */
  virtual QString icon( const KURL& , bool ) const { return m_strIcon; }

  /**
   * Use this function only if you don't have a special URL
   * for which you search a pixmap.
   *
   * This function is useful to find
   * out, which icon is usually chosen for a certain mime type. Since
   * no URL is passed, it is impossible to obey icon hints in desktop
   * entries for example.
   * @param group The icon group where the icon is going to be used.
   * @param force_size Override globallly configured icon size.
   *        Use 0 for the default size
   * @param state The icon state, one of: @p KIcon::DefaultState,
   *         @p KIcon::ActiveState or @p KIcon::DisabledState.
   * @param path Output parameter to get the full path. Seldom needed.
   *              Ignored if 0
   * @return the pixmap of the mime type, can be a default icon if not found
   */
  virtual QPixmap pixmap( KIcon::Group group, int force_size = 0, int state = 0,
                          QString * path = 0L ) const;

  /**
   * Find the pixmap for a given file of this mimetype.
   *
   * Convenience method that uses icon(), but also locates and
   * load the pixmap.
   *
   * @param _url URL for the file.
   * @param _group The icon group where the icon is going to be used.
   * @param _force_size Override globallly configured icon size.
   *        Use 0 for the default size
   * @param _state The icon state, one of: KIcon::DefaultState,
   * KIcon::ActiveState or KIcon::DisabledState.
   * @param _path Output parameter to get the full path. Seldom needed.
   *              Ignored if 0
   * @return the pixmap of the URL, can be a default icon if not found
   */
  virtual QPixmap pixmap( const KURL& _url, KIcon::Group _group, int _force_size = 0,
	    int _state = 0, QString * _path = 0L ) const;

  /**
   * Convenience method to find the pixmap for a URL.
   *
   * Call this one when you don't know the mimetype.
   *
   * @param _url URL for the file.
   * @param _mode the mode of the file. The mode may modify the icon
   *              with overlays that show special properties of the
   *              icon. Use 0 for default
   * @param _group The icon group where the icon is going to be used.
   * @param _force_size Override globally configured icon size.
   *        Use 0 for the default size
   * @param _state The icon state, one of: KIcon::DefaultState,
   * KIcon::ActiveState or KIcon::DisabledState.
   * @param _path Output parameter to get the full path. Seldom needed.
   *              Ignored if 0
   * @return the pixmap of the URL, can be a default icon if not found
   */
  static QPixmap pixmapForURL( const KURL & _url, mode_t _mode = 0, KIcon::Group _group = KIcon::Desktop,
                               int _force_size = 0, int _state = 0, QString * _path = 0L );


  /**
   * The same functionality as pixmapForURL(), but this method returns the name
   * of the icon to load. You'll have to use KIconLoader to load the pixmap for it.
   * The advantage of this method is that you can store the result, and then use it
   * later on for any kind of size.
   * @param _url URL for the file
   * @param _mode the mode of the file. The mode may modify the icon
   *              with overlays that show special properties of the
   *              icon. Use 0 for default
   * @return the name of the icon. The name of a default icon if there is no icon
   *         for the mime type
   */
  static QString iconNameForURL( const KURL & _url, mode_t _mode = 0 );


  /**
   * @deprecated the method was renamed, use iconNameForURL
   * @param _url URL for the file
   * @param _mode the mode of the file
   */
  static QString iconForURL( const KURL & _url, mode_t _mode = 0 ) KDE_DEPRECATED;

  /**
   * Return the "favicon" (see http://www.favicon.com) for the given @p url,
   * if available. Does NOT attempt to download the favicon, it only returns
   * one that is already available.
   *
   * If unavailable, returns QString().
   * @param url the URL of the favicon
   * @return the name of the favicon, or QString()
   */
  static QString favIconForURL( const KURL& url );

  /**
   * Returns the descriptive comment associated with the MIME type.
   * @return the descriptive comment associated with the MIME type
   */
  QString comment() const { return m_strComment; }

  /**
   * Returns the descriptive comment associated with the MIME type.
   * The arguments are unused, but provided so that KMimeType derived classes
   * can use them.
   *
   * @return The descriptive comment associated with the MIME type, if any.
   */
  virtual QString comment( const QString&, bool ) const { return m_strComment; }

  /**
   * Returns the descriptive comment associated with the MIME type.
   * The arguments are unused, but provided so that KMimeType derived classes
   * can use them.
   *
   * @return The descriptive comment associated with the MIME type, if any.
   */
  virtual QString comment( const KURL&, bool ) const { return m_strComment; }

  /**
   * Retrieve the list of patterns associated with the MIME Type.
   * @return a list of file globs that describe the file names
   *         (or, usually, the extensions) of files with this mime type
   */
  const QStringList& patterns() const { return m_lstPatterns; }

  /**
   * Load the mimetype from a stream.
   * @param qs the stream to load from
   */
  virtual void load( QDataStream &qs );

  /**
   * Save the mimetype to a stream.
   * @param qs the stream to save to
   */
  virtual void save( QDataStream &qs );

  /**
   * Returns the property with the given @p _name.
   * @param _name the name of the property
   * @return the value of the property
   * @see propertyNames()
   */
  virtual QVariant property( const QString& _name ) const;

  /**
   * Retrieves a list of all properties associated with this
   * KMimeType.
   * @return a list of all property names
   * @see property()
   */
  virtual QStringList propertyNames() const;

  /**
   * Retrieve a pointer to the mime type @p _name or a pointer to the default
   *         mime type "application/octet-stream".
   *
   * 0L is @em never returned.
   *
   * @em Very @em important: Don't store the result in a KMimeType* !
   *
   * @param _name the name of the mime type
   * @return the pointer to the KMimeType with the given @p _name, or
   *         a pointer to the application/octet-stream KMimeType if
   *         not found
   * @see KServiceType::serviceType
   */
  static Ptr mimeType( const QString& _name );

  /**
   * Finds a KMimeType with the given @p _url.
   * This function looks at mode_t first.
   * If that does not help it
   * looks at the extension.  This is fine for FTP, FILE, TAR and
   * friends, but is not for HTTP ( cgi scripts! ). You should use
   * KRun instead, but this function returns immediately while
   * KRun is async. If no extension matches, then
   * the file will be examined if the URL a local file or
   * "application/octet-stream" is returned otherwise.
   *
   * @param _url Is the right most URL with a filesystem protocol. It
   *        is up to you to find out about that if you have a nested
   *        URL.  For example
   *        "http://localhost/mist.gz#gzip:/decompress" would have to
   *        pass the "http://..." URL part, while
   *        "file:/tmp/x.tar#tar:/src/test.gz#gzip:/decompress" would
   *        have to pass the "tar:/..." part of the URL, since gzip is
   *        a filter protocol and not a filesystem protocol.
   * @param _mode the mode of the file (used, for example, to identify
   *              executables)
   * @param _is_local_file true if the file is local
   * @param _fast_mode If set to true no disk access is allowed to
   *        find out the mimetype. The result may be suboptimal, but
   *        it is @em fast.
   * @return A pointer to the matching mimetype. 0L is never returned.
   * @em Very @em Important: Don't store the result in a KMimeType* !
   */
  static Ptr findByURL( const KURL& _url, mode_t _mode = 0,
                        bool _is_local_file = false, bool _fast_mode = false );

  static Ptr findByURL( const KURL& _url, mode_t _mode,
                        bool _is_local_file, bool _fast_mode,
		  	bool *accurate);
  /**
   * Finds a KMimeType with the given @p _url.
   * This function looks at mode_t first.
   * If that does not help it
   * looks at the extension.  This is fine for FTP, FILE, TAR and
   * friends, but is not for HTTP ( cgi scripts! ). You should use
   * KRun instead, but this function returns immediately while
   * KRun is async. If no extension matches, then
   * the file will be examined if the URL a local file or
   * "application/octet-stream" is returned otherwise.
   *
   * Equivalent to
   * \code
   * KURL u;
   * u.setPath(path);
   * return findByURL( u, mode, true, fast_mode );
   * \endcode
   *
   * @param path the path to the file
   * @param mode the mode of the file (used, for example, to identify
   *              executables)
   * @param fast_mode If set to true no disk access is allowed to
   *        find out the mimetype. The result may be suboptimal, but
   *        it is @em fast.
   * @return A pointer to the matching mimetype. 0L is never returned.
   */
  static Ptr findByPath( const QString& path, mode_t mode = 0, bool fast_mode = false );

  /**
   * Tries to find out the MIME type of a data chunk by looking for
   * certain magic numbers and characteristic strings in it.
   *
   * @param data the data to examine
   * @param accuracy If not a null pointer, *accuracy is set to the
   *          accuracy of the match (which is in the range 0..100)
   * @return a pointer to the KMimeType. application/octet-stream's KMimeType of the
   *         type can not be found this way.
   */
  static Ptr findByContent( const QByteArray &data, int *accuracy=0 );

  /**
   * Tries to find out the MIME type of a file by looking for
   * certain magic numbers and characteristic strings in it.
   * This function is similar to the previous one. Note that the
   * file name is not used for determining the file type, it is just
   * used for loading the file's contents.
   *
   * @param fileName the path to the file
   * @param accuracy If not a null pointer, *accuracy is set to the
   *          accuracy of the match (which is in the range 0..100)
   * @return a pointer to the KMimeType. application/octet-stream's KMimeType of the
   *         type can not be found this way.
   */
  static Ptr findByFileContent( const QString &fileName, int *accuracy=0 );

  struct Format{
     bool text : 1;
     enum { NoCompression=0, GZipCompression } compression : 4;
     unsigned dummy : 27;
  };

  /**
   * Returns whether a file has an internal format that is human readable,
   * or that would be human readable after decompression.
   * @since 3.2
   */
  static Format findFormatByFileContent( const QString &fileName );

  /**
   * Get all the mimetypes.
   *
   * Useful for showing the list of
   * available mimetypes.
   * More memory consuming than the ones above, don't use unless
   * really necessary.
   * @return the list of all existing KMimeTypes
   */
  static List allMimeTypes();

  /**
   * Returns the name of the default mimetype.
   * Always application/octet-stream, but this method exists
   * for performance purposes.
   * @return the name of the default mime type, always
   *         "application/octet-stream"
   */
  static const QString & defaultMimeType();

  /**
   * Returns the default mimetype.
   * Always application/octet-stream.
   * This can be used to check the result of mimeType(name).
   * @return the "application/octet-stream" mimetype pointer.
   * @since 3.2
   */
  static KMimeType::Ptr defaultMimeTypePtr();

  /**
   * If this mimetype inherits from ("is also") another mimetype,
   * return the name of the parent.
   *
   * For instance a text/x-log is a special kind of text/plain,
   * so the definition of text/x-log can say "X-KDE-IsAlso=text/plain".
   * Or an smb-workgroup is a special kind of inode/directory, etc.
   * This mechanism can also be used to rename mimetypes and preserve compat.
   *
   * Note that this notion doesn't map to the servicetype inheritance mechanism,
   * since an application that handles the specific type doesn't necessarily handle
   * the base type. The opposite is true though.
   *
   * @return the parent mime type, or QString() if not set
   * @since 3.2
   */
  QString parentMimeType() const;

  /**
   * Do not use name()=="somename" anymore, to check for a given mimetype.
   * For mimetype inheritance to work, use is("somename") instead.
   * Warning, do not use inherits(), that's the servicetype inheritance concept!
   * @since 3.2
   */
  bool is( const QString& mimeTypeName ) const;

  /**
   * @internal
   * Determines the mimetype of file based on it's name and returns the
   * matching pattern if any.
   */
  static KMimeType::Ptr diagnoseFileName(const QString &file, QString &pattern);

protected:
  void loadInternal( QDataStream& );
  void init( KDesktopFile * );

  /**
   * Signal a missing mime type.
   * @param _type the missinf mime type
   */
  static void errorMissingMimeType( const QString& _type );

  /**
   * This function makes sure that the default mime type exists.
   */
  static void buildDefaultType();

  /**
   * This function makes sure that vital mime types are installed.
   */
  static void checkEssentialMimeTypes();
  /**
   * true if check for vital mime types has been done.
   */
  static bool s_bChecked;

  QStringList m_lstPatterns;

  static Ptr s_pDefaultType;

protected:
  friend class KServiceTypeFactory;
  int patternsAccuracy() const;

protected:
  virtual void virtual_hook( int id, void* data );
};

/**
 * Folder mime type. Handles locked folders, for instance.
 * @short Mimetype for a folder (inode/directory)
 */
class KIO_EXPORT KFolderType : public KMimeType
{
  K_SYCOCATYPE( KST_KFolderType, KMimeType )

public:
//  KFolderType( const QString & _fullpath, const QString& _type, const QString& _icon, const QString& _comment,
//  	       const QStringList& _patterns );
//  KFolderType( const QString & _fullpath ) : KMimeType( _fullpath ) { }
  /**
   * Construct a folder mimetype and take all information from a desktop file.
   * @param config the desktop configuration file that describes the mime type
   */
  KFolderType( KDesktopFile *config) : KMimeType( config ) { }
  /** \internal */
  KFolderType( QDataStream& _str, int offset ) : KMimeType( _str, offset ) { }

  virtual QString icon( const QString& _url, bool _is_local ) const;
  virtual QString icon( const KURL& _url, bool _is_local ) const;
  virtual QString comment( const QString& _url, bool _is_local ) const;
  virtual QString comment( const KURL& _url, bool _is_local ) const;
protected:
  virtual void virtual_hook( int id, void* data );
};

/**
 * Mime type for desktop files.
 * Handles mount/umount icon, and user-defined properties.
 * @short Mimetype for a .desktop file
 */
class KIO_EXPORT KDEDesktopMimeType : public KMimeType
{
  K_SYCOCATYPE( KST_KDEDesktopMimeType, KMimeType )

public:
  enum ServiceType { ST_MOUNT, ST_UNMOUNT, /* ST_PROPERTIES, */ ST_USER_DEFINED };

  /**
   * Structure representing a service, in the list of services
   * returned by builtinServices and userDefinedServices
   */
  struct Service
  {
    Service() { m_display = true; }
    bool isEmpty() const { return m_strName.isEmpty(); }
    QString m_strName;
    QString m_strIcon;
    QString m_strExec;
    ServiceType m_type;
    bool m_display;
  };
  // KDEDesktopMimeType( const QString & _fullpath, const QString& _type, const QString& _icon,
  //                     const QString& _comment, const QStringList& _patterns );
  // KDEDesktopMimeType( const QString & _fullpath ) : KMimeType( _fullpath ) { }
  /**
   * Construct a desktop mimetype and take all information from a desktop file.
   * @param config the desktop configuration file that describes the mime type
   */
  KDEDesktopMimeType( KDesktopFile *config) : KMimeType( config ) { }
  /** \internal */
  KDEDesktopMimeType( QDataStream& _str, int offset ) : KMimeType( _str, offset ) { }

  virtual QString icon( const QString& _url, bool _is_local ) const;
  virtual QString icon( const KURL& _url, bool _is_local ) const;
  virtual QPixmap pixmap( const KURL& _url, KIcon::Group _group, int _force_size = 0,
                          int _state = 0, QString * _path = 0L ) const;
  virtual QString comment( const QString& _url, bool _is_local ) const;
  virtual QString comment( const KURL& _url, bool _is_local ) const;

  /**
   * Returns a list of services for the given .desktop file that are handled
   * by kio itself. Namely mount/unmount for FSDevice files.
   * @return the list of services
   */
  static QList<Service> builtinServices( const KURL& _url );
  /**
   * Returns a list of services defined by the user as possible actions
   * on the given .desktop file. May include empty actions which represent where
   * visual separators should appear in user-visible representations of those actions,
   * such as separators in a menu.
   * @param path the path to the desktop file describing the services
   * @param bLocalFiles true if those services are to be applied to local files only
   * (if false, services that don't have %u or %U in the Exec line won't be taken into account).
   * @return the list of user deviced actions
   */
  static QList<Service> userDefinedServices( const QString& path, bool bLocalFiles );

  /**
   * Overload of userDefinedServices for speed purposes: it takes a KConfig* so that
   * the caller can check things in the file without having it parsed twice.
   * @since 3.4
   */
  static QList<Service> userDefinedServices( const QString& path, KConfig& config, bool bLocalFiles );

  /**
   * Overload of userDefinedServices but also allows you to pass a list of urls for this file.
   * This allows for the menu to be changed depending on the exact files via
   * the X-KDE-GetActionMenu extension.
   * @since 3.5
   */
  static QList<Service> userDefinedServices( const QString& path, KConfig& config, bool bLocalFiles,  const KURL::List & file_list);

  /**
   * @param path is the path of the desktop entry.
   * @param service the service to execute
   * @deprecated, see the other executeService
   */
  static void executeService( const QString& path, KDEDesktopMimeType::Service& service ) KDE_DEPRECATED;

  /**
   * Execute @p service on the list of @p urls.
   * @param urls the list of urls
   * @param service the service to execute
   */
  static void executeService( const KURL::List& urls, KDEDesktopMimeType::Service& service );

  /**
   * Invokes the default action for the desktop entry. If the desktop
   * entry is not local, then only false is returned. Otherwise we
   * would create a security problem. Only types Link and Mimetype
   * could be followed.
   *
   * @param _url the url to run
   * @param _is_local true if the URL is local, false otherwise
   * @return true on success and false on failure.
   * @see KRun::runURL
   */
  static pid_t run( const KURL& _url, bool _is_local );

protected:
  virtual QPixmap pixmap( KIcon::Group group, int force_size = 0, int state = 0,
                          QString * path = 0L ) const
     { return KMimeType::pixmap( group, force_size, state, path ); }

  static pid_t runFSDevice( const KURL& _url, const KSimpleConfig &cfg );
  static pid_t runApplication( const KURL& _url, const QString & _serviceFile );
  static pid_t runLink( const KURL& _url, const KSimpleConfig &cfg );
  static pid_t runMimeType( const KURL& _url, const KSimpleConfig &cfg );
protected:
  virtual void virtual_hook( int id, void* data );
};

/**
 * The mime type for executable files.
 * @short MimeType for any executable, like /bin/ls
 */
class KIO_EXPORT KExecMimeType : public KMimeType
{
  K_SYCOCATYPE( KST_KExecMimeType, KMimeType )

public:
  // KExecMimeType( const QString & _fullpath, const QString& _type, const QString& _icon,
  //                 const QString& _comment, const QStringList& _patterns );
  // KExecMimeType( const QString & _fullpath ) : KMimeType( _fullpath ) { }
  /**
   * Construct a executable mimetype and take all information from a desktop file.
   * @param config the desktop configuration file that describes the mime type
   */
  KExecMimeType( KDesktopFile *config) : KMimeType( config ) { }
  /** \internal */
  KExecMimeType( QDataStream& _str, int offset ) : KMimeType( _str, offset ) { }
protected:
  virtual void virtual_hook( int id, void* data );
};

#endif
