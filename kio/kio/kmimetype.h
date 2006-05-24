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
   * Returns the icon associated with this mime type. See also icon(url)
   * which takes an URL and returns a special icon for this URL.
   *
   * For instance for a folder you can get the folder-specific icon instead
   * of the generic folder icon.
   *
   * @return the name of the icon, can be QString().
   */
  QString icon() const { return m_strIcon; } // KDE4: rename to iconName?

  /**
   * Return the filename of the icon associated with the mimetype.
   *
   * The url argument is unused, but is provided so that KMimeType-derived classes
   * can use it (e.g. KFolderType uses the URL to return one out of 2 icons)
   *
   * @return The path to the icon associated with this MIME type.
   */
  virtual QString icon( const KUrl& ) const { return m_strIcon; }
  // TODO KDE4: rename to iconName, to be consistent with iconNameForURL and not sounding like we return a QIcon?

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
   * @param state The icon state, one of: @p K3Icon::DefaultState,
   *         @p K3Icon::ActiveState or @p K3Icon::DisabledState.
   * @param path Output parameter to get the full path. Seldom needed.
   *              Ignored if 0
   * @return the pixmap of the mime type, can be a default icon if not found
   */
  virtual KDE_DEPRECATED QPixmap pixmap( K3Icon::Group group, int force_size = 0, int state = 0,
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
   * @param _state The icon state, one of: K3Icon::DefaultState,
   * K3Icon::ActiveState or K3Icon::DisabledState.
   * @param _path Output parameter to get the full path. Seldom needed.
   *              Ignored if 0
   * @return the pixmap of the URL, can be a default icon if not found
   */
  virtual KDE_DEPRECATED QPixmap pixmap( const KUrl& _url, K3Icon::Group _group, int _force_size = 0,
	    int _state = 0, QString * _path = 0L ) const;

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
  static QString iconNameForURL( const KUrl & _url, mode_t _mode = 0 );


  /**
   * @deprecated the method was renamed, use iconNameForURL
   * @param _url URL for the file
   * @param _mode the mode of the file
   */
  static KDE_DEPRECATED QString iconForURL( const KUrl & _url, mode_t _mode = 0 );

  /**
   * Return the "favicon" (see http://www.favicon.com) for the given @p url,
   * if available. Does NOT attempt to download the favicon, it only returns
   * one that is already available.
   *
   * If unavailable, returns QString().
   * @param url the URL of the favicon
   * @return the name of the favicon, or QString()
   */
  static QString favIconForURL( const KUrl& url );

  /**
   * Returns the descriptive comment associated with the MIME type.
   * The url argument is unused, but provided so that KMimeType derived classes
   * can use it.
   *
   * @return The descriptive comment associated with the MIME type, if any.
   */
  virtual QString comment( const KUrl& = KUrl() ) const { return KServiceType::comment(); }

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
  static Ptr findByURL( const KUrl& _url, mode_t _mode = 0,
                        bool _is_local_file = false, bool _fast_mode = false );

  static Ptr findByURL( const KUrl& _url, mode_t _mode,
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
   * KUrl u;
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
   */
  static KMimeType::Ptr defaultMimeTypePtr();

  /// Return true if this mimetype is the default mimetype
  bool isDefault() const { return this == defaultMimeTypePtr().data(); }

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
   */
  QString parentMimeType() const;

  /**
   * Do not use name()=="somename" anymore, to check for a given mimetype.
   * For mimetype inheritance to work, use is("somename") instead.
   * Warning, do not use inherits(), that's the servicetype inheritance concept!
   */
  bool is( const QString& mimeTypeName ) const;

  /**
   * Determines the extension from a filename (or full path) using the mimetype database.
   * This allows to extract "tar.bz2" for foo.tar.bz2
   * but still return "txt" for my.doc.with.dots.txt
   */
  static QString extractKnownExtension( const QString &fileName );

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
  QString m_strIcon;
  class Private;
  Private* const d;

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
  /**
   * Construct a folder mimetype and take all information from a desktop file.
   * @param config the desktop configuration file that describes the mime type
   */
  KFolderType( KDesktopFile *config) : KMimeType( config ) { }
  /** \internal */
  KFolderType( QDataStream& _str, int offset ) : KMimeType( _str, offset ) { }

  virtual QString icon( const KUrl& _url ) const;
  virtual QString comment( const KUrl& _url ) const;
protected:
  virtual void virtual_hook( int id, void* data );
};

#endif
