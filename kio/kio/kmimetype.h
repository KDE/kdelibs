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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef __kmimetype_h__
#define __kmimetype_h__

#include <sys/types.h>
#include <sys/stat.h>

#include <qstringlist.h>
#include <qvaluelist.h>
#include <qpixmap.h>
#include <kicontheme.h>

#include <kurl.h>

#include "ksycocatype.h"
#include "kservicetype.h"

class KSimpleConfig;
/**
 * Represent a mime type, like "text/plain", and the data that is associated
 * with it.
 *
 * The starting point you need is often the static methods.
 * @see KServiceType
 */
class KMimeType : public KServiceType
{
  K_SYCOCATYPE( KST_KMimeType, KServiceType )

public:
  typedef KSharedPtr<KMimeType> Ptr;
  typedef QValueList<Ptr> List;
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
   * can use them (e.g. @ref KFolderType uses the URL to return one out of 2 icons)
   *
   * @return The path to the icon associated with this MIME type.
   */
  virtual QString icon( const QString& , bool ) const { return m_strIcon; }

  /**
   * Return the filename of the icon associated with the mimetype.
   *
   * The arguments are unused, but provided so that KMimeType-derived classes
   * can use them (e.g. @ref KFolderType uses the URL to return one out of 2 icons)
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
   * @param _group The icon group where the icon is going to be used.
   * @param _force_size Override globallly configured icon size.
   *        Use 0 for the default size
   * @param _state The icon state, one of: @p KIcon::DefaultState,
   *         @p KIcon::ActiveState or @p KIcon::DisabledState.
   * @param _path Output parameter to get the full path. Seldom needed.
   *              Ignored if 0
   * @return the pixmap of the mime type, can be a default icon if not found
   */
  virtual QPixmap pixmap( KIcon::Group _group, int _force_size = 0, int _state = 0,
                          QString * _path = 0L ) const;

  /**
   * Find the pixmap for a given file of this mimetype.
   *
   * Convenience method that uses @ref icon(), but also locates and
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
   * The same functionality as @ref pixmapForURL(), but this method returns the name
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
  static QString iconForURL( const KURL & _url, mode_t _mode = 0 );

  /**
   * Return the "favicon" (see http://www.favicon.com) for the given @p url,
   * if available. Does NOT attempt to download the favicon, it only returns
   * one that is already available. 
   *
   * If unavailable, returns QString::null.
   * @param url the URL of the favicon
   * @return the name of the favicon, or QString::null
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
   * @ref KRun instead, but this function returns immediately while
   * @ref KRun is async. If no extension matches, then
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
  /**
   * Finds a KMimeType with the given @p _url.
   * This function looks at mode_t first.
   * If that does not help it
   * looks at the extension.  This is fine for FTP, FILE, TAR and
   * friends, but is not for HTTP ( cgi scripts! ). You should use
   * @ref KRun instead, but this function returns immediately while
   * @ref KRun is async. If no extension matches, then
   * the file will be examined if the URL a local file or
   * "application/octet-stream" is returned otherwise.
   *
   * Equivalent to 
   *   <pre>
   *        KURL u; 
   *        u.setPath(path); 
   *        return findByURL( u, mode, true, fast_mode );
   *   </pre>
   * @param path the path to the file
   * @param _mode the mode of the file (used, for example, to identify 
   *              executables)
   * @param _fast_mode If set to true no disk access is allowed to
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
   * @param accurracy If not a null pointer, *accuracy is set to the
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
   * @param accurracy If not a null pointer, *accuracy is set to the
   *          accuracy of the match (which is in the range 0..100)
   * @return a pointer to the KMimeType. application/octet-stream's KMimeType of the 
   *         type can not be found this way.
   */
  static Ptr findByFileContent( const QString &fileName, int *accuracy=0 );

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
  virtual void virtual_hook( int id, void* data );
};

/**
 * Folder mime type. Handles locked folders, for instance.
 * @short Mimetype for a folder (inode/directory)
 */
class KFolderType : public KMimeType
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
  /// @internal
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
class KDEDesktopMimeType : public KMimeType
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
  /// @internal
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
  static QValueList<Service> builtinServices( const KURL& _url );
  /**
   * Returns a list of services defined by the user as possible actions
   * on the given .desktop file.
   * @param path the path to the desktop file describing the services
   * @param bLocalFiles true if those services are to be applied to local files only
   * (if false, services that don't have %u or %U in the Exec line won't be taken into account).
   * @return the list of user deviced actions
   */
  static QValueList<Service> userDefinedServices( const QString& path, bool bLocalFiles );

  /**
   * @param _path is the path of the desktop entry.
   * @deprecated, see the other executeService
   */
  static void executeService( const QString& _path, KDEDesktopMimeType::Service& _service );

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
  virtual QPixmap pixmap(KIcon::Group a, int b, int c, QString *d) const
     { return KMimeType::pixmap(a, b, c, d); }

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
class KExecMimeType : public KMimeType
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
  /// @internal
  KExecMimeType( QDataStream& _str, int offset ) : KMimeType( _str, offset ) { }
protected:
  virtual void virtual_hook( int id, void* data );
};

#endif
