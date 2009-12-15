// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE libraries
 *  Copyright (C) 1999 Torben Weis <weis@kde.org>
 *  Copyright (C) 2005-2006 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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
 */

#ifndef kurl_h
#define kurl_h

#include <kdecore_export.h>

#include <QtCore/QVariant>
#include <QtCore/QUrl>
#include <QtCore/QMap>

class QStringList;
class QMimeData;

class KUrlPrivate;

// maybe we should encapsulate QUrl instead of inheriting from it.
// this would even allow us to inherit from KUri instead.
// and this way hacks like setPath() would be less ugly, and we could avoid
// half KDE code using setScheme() and the other half using setProtocol(), etc.
// (DF)

/**
 * \class KUrl kurl.h <KUrl>
 *
 * Represents and parses a URL.
 *
 * A prototypical URL looks like:
 * \code
 *   protocol://user:password\@hostname:port/path/to/file.ext#reference
 * \endcode
 *
 * KUrl handles escaping of URLs. This means that the specification
 * of a full URL will differ from the corresponding string that would specify a
 * local file or directory in file-operations like fopen. This is because an URL
 * doesn't allow certain characters and escapes them. (e.g. '#'->"%23", space->"%20")
 * (In a URL the hash-character '#' is used to specify a "reference", i.e. the position
 * within a document).
 *
 * The constructor KUrl(const QString&) expects a string properly escaped,
 * or at least non-ambiguous.
 * If you have the absolute path you should use KUrl::fromPath(const QString&).
 * \code
 *     KUrl kurl = KUrl::fromPath("/bar/#foo#");
 *     QString url = kurl.url();    // -> "file:///bar/%23foo%23"
 * \endcode
 *
 * If you have the URL of a local file or directory and need the absolute path,
 * you would use toLocalFile().
 * \code
 *    KUrl url( "file:///bar/%23foo%23" );
 *    ...
 *    if ( url.isLocalFile() )
 *       QString path = url.toLocalFile();       // -> "/bar/#foo#"
 * \endcode
 *
 * This must also be considered when you have separated directory and file
 * strings and need to put them together.
 * While you can simply concatenate normal path strings, you must take care if
 * the directory-part is already an escaped URL.
 * (This might be needed if the user specifies a relative path, and your
 * program supplies the rest from elsewhere.)
 *
 * Wrong:
 * \code
 *    QString dirUrl = "file:///bar/";
 *    QString fileName = "#foo#";
 *    QString invalidURL = dirUrl + fileName;   // -> "file:///bar/#foo#" won't behave like you would expect.
 * \endcode
 * Instead you should use addPath():
 * Right:
 * \code
 *    KUrl url( "file:///bar/" );
 *    QString fileName = "#foo#";
 *    url.addPath( fileName );
 *    QString validURL = url.url();    // -> "file:///bar/%23foo%23"
 * \endcode
 *
 * Also consider that some URLs contain the password, but this shouldn't be
 * visible. Your program should use prettyUrl() every time it displays a
 * URL, whether in the GUI or in debug output or...
 *
 * \code
 *    KUrl url( "ftp://name:password@ftp.faraway.org/bar/%23foo%23");
 *    QString visibleURL = url.prettyUrl(); // -> "ftp://name@ftp.faraway.org/bar/%23foo%23"
 * \endcode
 * Note that prettyUrl() doesn't change the character escapes (like "%23").
 * Otherwise the URL would be invalid and the user wouldn't be able to use it in another
 * context.
 *
 */
class KDECORE_EXPORT KUrl : public QUrl // krazy:exclude=dpointer,qclasses (krazy can't deal with embedded classes)
{
public:
  typedef QMap<QString, QString> MetaDataMap;
  enum MimeDataFlags { DefaultMimeDataFlags = 0, NoTextExport = 1 };

  /**
   * \class List kurl.h <KUrl>
   *
   * KUrl::List is a QList that contains KUrls with a few
   * convenience methods.
   * @see KUrl
   * @see QList
   */
  class KDECORE_EXPORT List : public QList<KUrl> //krazy:exclude=dpointer (just some convenience methods)
  {
  public:
    /**
     * Creates an empty List.
     */
      List() { }
      /**
       * Creates a list that contains the given URL as only
       * item.
       * @param url the url to add.
       */
      List(const KUrl &url);
      /**
       * Creates a list that contains the URLs from the given
       * list of strings.
       * @param list the list containing the URLs as strings
       */
      List(const QStringList &list);
      /**
       * Creates a list that contains the URLs from the given QList<KUrl>.
       * @param list the list containing the URLs
       */
      List(const QList<KUrl> &list);
      /**
       * Converts the URLs of this list to a list of strings.
       * @return the list of strings
       */
      QStringList toStringList() const;

      /**
       * Converts this KUrl::List to a QVariant, this allows to use KUrl::List
       * in QVariant() constructor
       */
      operator QVariant() const;

      /**
       * Adds URLs data into the given QMimeData.
       *
       * By default, populateMimeData also exports the URLs as plain text, for e.g. dropping
       * onto a text editor.
       * But in some cases this might not be wanted, e.g. if adding other mime data
       * which provides better plain text data.
       *
       * WARNING: do not call this method multiple times on the same mimedata object,
       * you can add urls only once. But you can add other things, e.g. images, XML...
       *
       * @param mimeData the QMimeData instance used to drag or copy this URL
       * @param metaData KIO metadata shipped in the mime data, which is used for instance to
       * set a correct HTTP referrer (some websites require it for downloading e.g. an image)
       * @param flags set NoTextExport to prevent setting plain/text data into @p mimeData
       * In such a case, setExportAsText( false ) should be called.
       */
      void populateMimeData( QMimeData* mimeData,
                             const KUrl::MetaDataMap& metaData = MetaDataMap(),
                             MimeDataFlags flags = DefaultMimeDataFlags ) const;

      /**
       * Adds URLs into the given QMimeData.
       *
       * This should add both the KDE-style URLs (eg: desktop:/foo) and
       * the "most local" version of the URLs (eg:
       * file:///home/jbloggs/Desktop/foo) to the mimedata.
       *
       * This method should be called on the KDE-style URLs.
       *
       * @code
       * QMimeData* mimeData = new QMimeData();
       *
       * KUrl::List kdeUrls;
       * kdeUrls << "desktop:/foo";
       * kdeUrls << "desktop:/bar";
       *
       * KUrl::List normalUrls;
       * normalUrls << "file:///home/jbloggs/Desktop/foo";
       * normalUrls << "file:///home/jbloggs/Desktop/bar";
       *
       * kdeUrls.populateMimeData(normalUrls, mimeData);
       * @endcode
       *
       * @param mostLocalUrls the "most local" urls
       * @param mimeData      the mime data object to populate
       * @param metaData      KIO metadata shipped in the mime data, which is
       *                      used for instance to set a correct HTTP referrer
       *                      (some websites require it for downloading e.g. an
       *                      image)
       * @param flags         set NoTextExport to prevent setting plain/text
       *                      data into @p mimeData. In such a case,
       *                      <code>setExportAsText(false)</code> should be called.
       * @since 4.2
       */
      void populateMimeData(const KUrl::List& mostLocalUrls,
                            QMimeData* mimeData,
                            const KUrl::MetaDataMap& metaData = MetaDataMap(),
                            MimeDataFlags flags = DefaultMimeDataFlags) const;

      /**
       * Return true if @p mimeData contains URI data
       */
      static bool canDecode( const QMimeData *mimeData );

      /**
        * Return the list of mimeTypes that can be decoded by fromMimeData
        */
      static QStringList mimeDataTypes();

      /**
       * Extract a list of KUrls from the contents of @p mimeData.
       * Decoding will fail if @p mimeData does not contain any URLs, or if at
       * least one extracted URL is not valid.
       * @param mimeData the mime data to extract from; cannot be 0
       * @param metaData optional pointer to a map holding the metadata
       * @return the list of urls
       */
      static KUrl::List fromMimeData( const QMimeData *mimeData, KUrl::MetaDataMap* metaData = 0 );

      /**
       * Flags to be used in fromMimeData.
       * @since 4.2.3
       */
      enum DecodeOptions {
        /**
         * When the mimedata contains both KDE-style URLs (eg: desktop:/foo) and
         * the "most local" version of the URLs (eg: file:///home/dfaure/Desktop/foo),
         * decode it as local urls. Useful in paste/drop operations that end up calling KIO,
         * so that urls from other users work as well.
         */
        PreferLocalUrls,
        /**
         * When the mimedata contains both KDE-style URLs (eg: desktop:/foo) and
         * the "most local" version of the URLs (eg: file:///home/dfaure/Desktop/foo),
         * decode it as the KDE-style URL. Useful in DnD code e.g. when moving icons,
         * and the kde-style url is used as identifier for the icons.
         */
        PreferKdeUrls
      };

      /**
       * Extract a list of KUrls from the contents of @p mimeData.
       * Decoding will fail if @p mimeData does not contain any URLs, or if at
       * least one extracted URL is not valid.
       * @param mimeData the mime data to extract from; cannot be 0
       * @param decodeOptions options for decoding
       * @param metaData optional pointer to a map holding the metadata
       * @return the list of urls
       * @since 4.2.3
       */
      static KUrl::List fromMimeData( const QMimeData *mimeData,
                                      DecodeOptions decodeOptions, // TODO KDE5: = PreferKdeUrls, and merge with above
                                      KUrl::MetaDataMap* metaData = 0 );

  };
  /**
   * Constructs an empty URL.
   */
  KUrl();

  /**
   * Destructs the KUrl object.
   */
  ~KUrl();

  /**
   * Usual constructor, to construct from a string.
   * @param urlOrPath An encoded URL or a path.
   */
  KUrl( const QString& urlOrPath );
  /**
   * Constructor taking a char * @p urlOrPath, which is an _encoded_ representation
   * of the URL, exactly like the usual constructor. This is useful when
   * the URL, in its encoded form, is strictly ascii.
   * @param urlOrPath An encoded URL, or a path.
   */
  explicit KUrl( const char * urlOrPath );
  /**
   * Constructor taking a QByteArray @p urlOrPath, which is an _encoded_ representation
   * of the URL, exactly like the usual constructor. This is useful when
   * the URL, in its encoded form, is strictly ascii.
   * @param urlOrPath An encoded URL, or a path.
   */
  explicit KUrl( const QByteArray& urlOrPath );

  /**
   * Copy constructor.
   * @param u the KUrl to copy
   */
  KUrl( const KUrl& u );
  /**
   * Converts from a QUrl.
   * @param u the QUrl
   */
  KUrl( const QUrl &u ); //krazy:exclude=qclasses
  /**
   * Constructor allowing relative URLs.
   *
   * @param _baseurl The base url.
   * @param _rel_url A relative or absolute URL.
   * If this is an absolute URL then @p _baseurl will be ignored.
   * If this is a relative URL it will be combined with @p _baseurl.
   * Note that _rel_url should be encoded too, in any case.
   * So do NOT pass a path here (use setPath or addPath instead).
   */
  KUrl( const KUrl& _baseurl, const QString& _rel_url );

  /**
   * Returns the protocol for the URL (i.e., file, http, etc.), lowercased.
   * @see QUrl::scheme
   */
  QString protocol() const;

  /**
   * Sets the protocol for the URL (i.e., file, http, etc.)
   * @param proto the new protocol of the URL (without colon)
   */
  void setProtocol( const QString& proto );

  /**
   * Returns the decoded user name (login, user id, ...) included in the URL.
   * @return the user name or QString() if there is no user name
   */
  QString user() const;

  /**
   * Sets the user name (login, user id, ...) included in the URL.
   *
   * Special characters in the user name will appear encoded in the URL.
   * @param user the name of the user or QString() to remove the user
   */
  void setUser( const QString& user );

  /**
   * Test to see if this URL has a user name included in it.
   * @return true if the URL has an non-empty user name
   */
  bool hasUser() const;

  /**
   * Returns the decoded password (corresponding to user()) included in the URL.
   * @return the password or QString() if it does not exist
   **/
  QString pass() const;

  /**
   * Sets the password (corresponding to user()) included in the URL.
   *
   * Special characters in the password will appear encoded in the URL.
   * Note that a password can only appear in a URL string if you also set
   * a user.
   * @param pass the password to set or QString() to remove the password
   * @see setUser
   * @see hasUser
   **/
  void setPass( const QString& pass );

  /**
   * Test to see if this URL has a password included in it.
   * @return true if there is a non-empty password set
   **/
  bool hasPass() const;

  /**
   * Test to see if this URL has a hostname included in it.
   * @return true if the URL has a host
   **/
  bool hasHost() const;

  /**
   * Options to be used in adjustPath
   */
  enum AdjustPathOption
  {
    /**
     * strips any trailing '/'
     */
    RemoveTrailingSlash,

    /**
     * Do not change the path.
     */
    LeaveTrailingSlash,

    /**
     * adds a trailing '/' if there is none yet
     */
    AddTrailingSlash
  };

  /**
   * @param trailing use to add or remove a trailing slash to/from the path. see adjustPath

   * @return The current decoded path. This does not include the query. Can
   *         be QString() if no path is set.
   */
  QString path( AdjustPathOption trailing = LeaveTrailingSlash ) const;

  /**
   * @param trailing use to add or remove a trailing slash to/from the local path. see adjustPath

   * @return The current local path. Can
   *   be QString() if no path is set.
   */
  QString toLocalFile( AdjustPathOption trailing = LeaveTrailingSlash ) const;

  /// \reimp so that KUrl u; u.setPath(path); implies "file" protocol.
  void setPath( const QString& path );

  /**
   * Test to see if this URL has a path is included in it.
   * @return true if there is a path
   **/
  bool hasPath() const;

  /**
   * Options to be used in cleanPath
   */
  enum CleanPathOption
  {
    /**
     * if set, occurrences of consecutive directory separators
     * (e.g. /foo//bar) are cleaned up as well.  (set by default)
     */
    SimplifyDirSeparators = 0x00,

    /**
     * The opposite of SimplifyDirSeparators.
     */
    KeepDirSeparators = 0x01
  };

  Q_DECLARE_FLAGS(CleanPathOptions,CleanPathOption)

  /**
   * Resolves "." and ".." components in path.
   * Some servers seem not to like the removal of extra '/'
   * even though it is against the specification in RFC 2396.
   *
   * @param options use KeepDirSeparators if you don't want to remove consecutive
   *                occurrences of directory separator
   */
  void cleanPath(const CleanPathOption& options = SimplifyDirSeparators);


  /**
   * Add or remove a trailing slash to/from the path.
   *
   * If the URL has no path, then no '/' is added
   * anyway. And on the other side: If the path is "/", then this
   * character won't be stripped. Reason: "ftp://weis\@host" means something
   * completely different than "ftp://weis\@host/". So adding or stripping
   * the '/' would really alter the URL, while "ftp://host/path" and
   * "ftp://host/path/" mean the same directory.
   *
   * @param trailing  RemoveTrailingSlash strips any trailing '/' and
   *                  AddTrailingSlash adds  a trailing '/' if there is none yet
   */
  void adjustPath(AdjustPathOption trailing);

  /**
   * This is useful for HTTP. It looks first for '?' and decodes then.
   * The encoded path is the concatenation of the current path and the query.
   * @param _txt the new path and query.
   */
  void setEncodedPathAndQuery( const QString& _txt );

#if 0
  /**
   * Sets the (already encoded) path
   * @param _txt the new path
   * @see QTextCodec::mibEnum()
   */
  void setEncodedPath(const QString& _txt );
#endif

  /**
   * Option to be used in encodedPathAndQuery
   **/
  enum EncodedPathAndQueryOption
  {
    /**
     * Permit empty path (default)
     */
    PermitEmptyPath=0x00,
    /**
     * If set to true then an empty path is substituted by "/"
     * (this is the opposite of PermitEmptyPath)
     */
    AvoidEmptyPath=0x01
  };
  Q_DECLARE_FLAGS( EncodedPathAndQueryOptions, EncodedPathAndQueryOption)

  /**
   * Returns the encoded path and the query.
   *
   * @param trailing  add or remove a trailing '/', see adjustPath
   * @param options a set of flags from EncodedPathAndQueryOption
   * @return The concatenation of the encoded path , '?' and the encoded query.
   *
   */
  QString encodedPathAndQuery( AdjustPathOption trailing = LeaveTrailingSlash, const EncodedPathAndQueryOptions &options = PermitEmptyPath ) const;

  /**
   * @param query This is considered to be encoded. This has a good reason:
   * The query may contain the 0 character.
   *
   * The query should start with a '?'. If it doesn't '?' is prepended.
   */
  void setQuery( const QString& query );

  /**
   * Returns the query of the URL.
   * The query may contain the 0 character.
   * If a query is present it always starts with a '?'.
   * A single '?' means an empty query.
   * An empty string means no query.
   * @return The encoded query, or QString() if there is none.
   */
  QString query() const;

  /**
   * The reference is @em never decoded automatically.
   * @return the undecoded reference, or QString() if there is none
   */
  QString ref() const;

  /**
   * Sets the reference part (everything after '#').
   * If you have an encoded fragment already (as a QByteArray), you can call setFragment directly.
   * @param fragment the encoded reference (or QString() to remove it).
   */
  void setRef( const QString& fragment );

  /**
   * Checks whether the URL has a reference part.
   * @return true if the URL has a reference part. In a URL like
   *         http://www.kde.org/kdebase.tar#tar:/README it would
   *         return true, too.
   */
  bool hasRef() const;

  /**
   * Returns the HTML reference (the part of the URL after "#").
   * @return The HTML-style reference.
   * @see split
   * @see hasSubUrl
   * @see encodedHtmlRef
   */
  QString htmlRef() const;

  /**
   * Returns the HTML reference (the part of the URL after "#") in
   * encoded form.
   * @return The HTML-style reference in its original form.
   */
  QString encodedHtmlRef() const;

  /**
   * Sets the HTML-style reference.
   *
   * @param _ref The new reference. This is considered to be @em not encoded in
   *         contrast to setRef(). Use QString() to remove it.
   * @see htmlRef()
   */
  void setHTMLRef( const QString& _ref );

  /**
   * Checks whether there is a HTML reference.
   * @return true if the URL has an HTML-style reference.
   * @see htmlRef()
   */
  bool hasHTMLRef() const;

  /**
   * Checks whether the file is local.
   * @return true if the file is a plain local file and has no filter protocols
   *         attached to it.
   */
  bool isLocalFile() const;

  /**
   * Adds encoding information to url by adding a "charset" parameter. If there
   * is already a charset parameter, it will be replaced.
   * @param encoding the encoding to add or QString() to remove the
   *                 encoding.
   */
  void setFileEncoding(const QString &encoding);

  /**
   * Returns encoding information from url, the content of the "charset"
   * parameter.
   * @return An encoding suitable for QTextCodec::codecForName()
   *         or QString() if not encoding was specified.
   */
  QString fileEncoding() const;

  /**
   * Checks whether the URL has any sub URLs. See split()
   * for examples for sub URLs.
   * @return true if the file has at least one sub URL.
   * @see split
   */
  bool hasSubUrl() const;

  /**
   * Adds to the current path.
   * Assumes that the current path is a directory. @p txt is appended to the
   * current path. The function adds '/' if needed while concatenating.
   * This means it does not matter whether the current path has a trailing
   * '/' or not. If there is none, it becomes appended. If @p txt
   * has a leading '/' then this one is stripped.
   *
   * @param txt The text to add. It is considered to be decoded.
   */
  void addPath( const QString& txt );

  /**
   * Options for queryItems. Currently, only one option is
   * defined:
   *
   * @param CaseInsensitiveKeys normalize query keys to lowercase.
   **/
  enum QueryItemsOption { CaseInsensitiveKeys = 1 };
  Q_DECLARE_FLAGS(QueryItemsOptions,QueryItemsOption)

  /**
   * Returns the list of query items as a map mapping keys to values.
   *
   * This does the same as QUrl::queryItems(), except that it
   * decodes "+" into " " in the value, supports CaseInsensitiveKeys,
   * and returns a different data type.
   *
   * @param options any of QueryItemsOption <em>or</em>ed together.
   *
   * @return the map of query items or the empty map if the url has no
   * query items.
   */
  QMap< QString, QString > queryItems( const QueryItemsOptions& options = 0 ) const;
  // #### TODO port the above queryItems to look more like QUrl's
  //using QUrl::queryItems; // temporary

  /**
   * Returns the value of a certain query item.
   *
   * This does the same as QUrl::queryItemValue(), except that it
   * decodes "+" into " " in the value.
   *
   * @param item Item whose value we want
   *
   * @return the value of the given query item name or QString() if the
   * specified item does not exist.
   */
  QString queryItem(const QString &item) const;

  /**
   * Add an additional query item.
   * To replace an existing query item, the item should first be
   * removed with removeQueryItem()
   *
   * @param _item Name of item to add
   * @param _value Value of item to add
   */
  void addQueryItem( const QString& _item, const QString& _value );


  /**
   * Sets the filename of the path.
   * In comparison to addPath() this function does not assume that the current
   * path is a directory. This is only assumed if the current path ends with '/'.
   *
   * Any reference is reset.
   *
   * @param _txt The filename to be set. It is considered to be decoded. If the
   *             current path ends with '/' then @p _txt int just appended, otherwise
   *             all text behind the last '/' in the current path is erased and
   *             @p _txt is appended then. It does not matter whether @p _txt starts
   *             with '/' or not.
   */
  void setFileName( const QString&_txt );

  /**
   * option to be used in fileName and directory
   */
  enum DirectoryOption
  {
    /**
     * This tells whether a trailing '/' should be ignored.
     *
     * If the flag is not set, for both <tt>file:///hallo/torben/</tt> and <tt>file:///hallo/torben</tt>
     * the fileName is "torben" and the path is "hallo"
     *
     * If the flag is set, then everything behind the last '/'is considered to be the filename.
     * So "hallo/torben" will be the path and the filename will be empty.
     */
    ObeyTrailingSlash = 0x02,
    /**
     * tells whether the returned result should end with '/' or not.
     * If the flag is set, '/' is added to the end of the path
     *
     * If the path is empty or just "/" then this flag has no effect.
     *
     * This option should only be used in directory(), it has no effect in fileName()
     */
    AppendTrailingSlash = 0x04,
    /**
     * Opposite of ObeyTrailingSlash  (default)
     * fileName("file:/foo/") and fileName("file:/foo") is "foo" in both cases.
     */
    IgnoreTrailingSlash = 0x01

  };
  Q_DECLARE_FLAGS(DirectoryOptions,DirectoryOption)


  /**
   * Returns the filename of the path.
   * @param options a set of DirectoryOption flags.  (StripTrailingSlashFromResult has no effect)
   * @return The filename of the current path. The returned string is decoded. Null
   *         if there is no file (and thus no path).
   */
  QString fileName( const DirectoryOptions& options = IgnoreTrailingSlash ) const;

  /**
   * Returns the directory of the path.
   * @param options a set of DirectoryOption flags
   * @return The directory part of the current path. Everything between the last and the second last '/'
   *         is returned. For example <tt>file:///hallo/torben/</tt> would return "/hallo/torben/" while
   *         <tt>file:///hallo/torben</tt> would return "hallo/". The returned string is decoded.
   *         QString() is returned when there is no path.
   */
  QString directory( const DirectoryOptions& options = IgnoreTrailingSlash ) const;

  /**
   * Set the directory to @p dir, leaving the filename empty.
   */
  void setDirectory(const QString &dir);

  /**
   * Changes the directory by descending into the given directory.
   * It is assumed the current URL represents a directory.
   * If @p dir starts with a "/" the
   * current URL will be "protocol://host/dir" otherwise @p _dir will
   * be appended to the path. @p _dir can be ".."
   * This function won't strip protocols. That means that when you are in
   * file:///dir/dir2/my.tgz#tar:/ and you do cd("..") you will
   * still be in file:///dir/dir2/my.tgz#tar:/
   *
   * @param _dir the directory to change to
   * @return true if successful
   */
  bool cd( const QString& _dir );

  /**
   * Returns the URL as string, with all escape sequences intact,
   * encoded in a given charset.
   * This is used in particular for encoding URLs in UTF-8 before using them
   * in a drag and drop operation.
   * Please note that the string returned by url() will include
   * the password of the URL. If you want to show the URL to the
   * user, use prettyUrl().
   *
   * @param trailing use to add or remove a trailing slash to/from the path. See adjustPath
   * @return The complete URL, with all escape sequences intact, encoded
   * in a given charset.
   * @see prettyUrl()
   */
  QString url( AdjustPathOption trailing = LeaveTrailingSlash ) const;

  /**
   * Returns the URL as string in human-friendly format.
   * Example:
   * \code
   * http://localhost:8080/test.cgi?test=hello world&name=fred
   * \endcode
   * @param trailing use to add or remove a trailing slash to/from the path. see adjustPath.
   *
   * @return A human readable URL, with no non-necessary encodings/escaped
   * characters. Password will not be shown.
   * @see url()
   */
  QString prettyUrl( AdjustPathOption trailing = LeaveTrailingSlash ) const;

  /**
   * Return the URL as a string, which will be either the URL (as prettyUrl
   * would return) or, when the URL is a local file without query or ref,
   * the path.
   * Use this method, to display URLs to the user.
   * You can give the result of pathOrUrl back to the KUrl constructor, it accepts
   * both paths and urls.
   *
   * @return the new KUrl
   */
  QString pathOrUrl() const;
  /**
   * Overload with @p trailing parameter
   * @param trailing use to add or remove a trailing slash to/from the path. see adjustPath.
   * @since 4.2
   */
  QString pathOrUrl(AdjustPathOption trailing) const; // KDE5: merge with above. Rename to toUrlOrLocalFile?

  /**
   * Returns the URL as a string, using the standard conventions for mime data
   * (drag-n-drop or copy-n-paste).
   * Internally used by KUrl::List::fromMimeData, which is probably what you want to use instead.
   */
  QString toMimeDataString() const;

  /**
   * This function is useful to implement the "Up" button in a file manager for example.
   * cd() never strips a sub-protocol. That means that if you are in
   * file:///home/x.tgz#gzip:/#tar:/ and hit the up button you expect to see
   * file:///home. The algorithm tries to go up on the right-most URL. If that is not
   * possible it strips the right most URL. It continues stripping URLs.
   * @return a URL that is a level higher
   */
  KUrl upUrl( ) const;

  KUrl& operator=( const KUrl& _u );

  // Define those, since the constructors are explicit
  KUrl& operator=( const char * _url ) { *this = KUrl(_url); return *this; }
  KUrl& operator=( const QByteArray& _url ) { *this = KUrl(_url); return *this; }
  KUrl& operator=( const QString& _url ) { *this = KUrl(_url); return *this; }

  bool operator==( const KUrl& _u ) const;
  bool operator==( const QString& _u ) const;
  bool operator!=( const KUrl& _u ) const { return !( *this == _u ); }
  bool operator!=( const QString& _u ) const { return !( *this == _u ); }

  /**
   * Converts this KUrl to a QVariant, this allows to use KUrl
   * in QVariant() constructor
   */
  operator QVariant() const;

  /**
   * The same as equals(), just with a less obvious name.
   * Compares this url with @p u.
   * @param u the URL to compare this one with.
   * @param ignore_trailing set to true to ignore trailing '/' characters.
   * @return True if both urls are the same. If at least one of the urls is invalid,
   * false is returned.
   * @see operator==. This function should be used if you want to
   * ignore trailing '/' characters.
   * @deprecated Use equals() instead.
   */
  KDE_DEPRECATED bool cmp( const KUrl &u, bool ignore_trailing = false ) const;


  /**
   * flags to be used in url compartators function like equal, or urlcmp
   **/
  enum EqualsOption
  {
    /**
     * ignore trailing '/' characters
     */
    CompareWithoutTrailingSlash = 0x01,
    /**
     * disables comparison of HTML-style references.
     */
    CompareWithoutFragment = 0x02
  };
  Q_DECLARE_FLAGS(EqualsOptions,EqualsOption)

  /**
   * Compares this url with @p u.
   * @param u the URL to compare this one with.
   * @param options a set of EqualsOption flags
   * @return True if both urls are the same. If at least one of the urls is invalid,
   * false is returned.
   * @see operator==. This function should be used if you want to
   * ignore trailing '/' characters.
   */
  bool equals( const KUrl &u, const EqualsOptions& options=0 ) const;

  /**
   * Checks whether the given URL is parent of this URL.
   * For instance, ftp://host/dir/ is a parent of ftp://host/dir/subdir/subsubdir/.
   * @return true if this url is a parent of @p u (or the same URL as @p u)
   *
   */
  bool isParentOf( const KUrl& u ) const;
    // (this overload of the QUrl method allows to use the implicit KUrl constructors)
    // but also the equality test

  /**
   * Splits nested URLs like file:///home/weis/kde.tgz#gzip:/#tar:/kdebase
   * A URL like http://www.kde.org#tar:/kde/README.hml#ref1 will be split in
   * http://www.kde.org and tar:/kde/README.html#ref1.
   * That means in turn that "#ref1" is an HTML-style reference and not a new sub URL.
   * Since HTML-style references mark
   * a certain position in a document this reference is appended to every URL.
   * The idea behind this is that browsers, for example, only look at the first URL while
   * the rest is not of interest to them.
   *
   *
   * @param _url The URL that has to be split.
   * @return An empty list on error or the list of split URLs.
   * @see hasSubUrl
   */
  static List split( const QString& _url );

  /**
   * Splits nested URLs like file:///home/weis/kde.tgz#gzip:/#tar:/kdebase
   * A URL like http://www.kde.org#tar:/kde/README.hml#ref1 will be split in
   * http://www.kde.org and tar:/kde/README.html#ref1.
   * That means in turn that "#ref1" is an HTML-style reference and not a new sub URL.
   * Since HTML-style references mark
   * a certain position in a document this reference is appended to every URL.
   * The idea behind this is that browsers, for example, only look at the first URL while
   * the rest is not of interest to them.
   *
   * @return An empty list on error or the list of split URLs.
   *
   * @param _url The URL that has to be split.
   * @see hasSubUrl
   */
  static List split( const KUrl& _url );

  /**
   * Reverses split(). Only the first URL may have a reference. This reference
   * is considered to be HTML-like and is appended at the end of the resulting
   * joined URL.
   * @param _list the list to join
   * @return the joined URL
   */
  static KUrl join( const List& _list );

  /**
   * Creates a KUrl object from a QString representing an absolute path.
   * KUrl url( somePath ) does the same, but this method is more explicit
   * and avoids the path-or-url detection in the KUrl constructor.
   *
   * @param text the path
   * @return the new KUrl
   */
  static KUrl fromPath( const QString& text );

  /**
   * \deprecated
   * Since KDE4 you can pass both urls and paths to the KUrl constructors.
   * Use KUrl(text) instead.
   */
  static KDE_DEPRECATED KUrl fromPathOrUrl( const QString& text );

  /**
   * Creates a KUrl from a string, using the standard conventions for mime data
   * (drag-n-drop or copy-n-paste).
   * Internally used by KUrl::List::fromMimeData, which is probably what you want to use instead.
   */
  static KUrl fromMimeDataByteArray( const QByteArray& str );

  /**
   * Adds URL data into the given QMimeData.
   *
   * By default, populateMimeData also exports the URL as plain text, for e.g. dropping
   * onto a text editor.
   * But in some cases this might not be wanted, e.g. if adding other mime data
   * which provides better plain text data.
   *
   * WARNING: do not call this method multiple times, use KUrl::List::populateMimeData instead.
   *
   * @param mimeData the QMimeData instance used to drag or copy this URL
   * @param metaData KIO metadata shipped in the mime data, which is used for instance to
   * set a correct HTTP referrer (some websites require it for downloading e.g. an image)
   * @param flags set NoTextExport to prevent setting plain/text data into @p mimeData
   * In such a case, setExportAsText( false ) should be called.
   */
  void populateMimeData( QMimeData* mimeData,
                         const MetaDataMap& metaData = MetaDataMap(),
                         MimeDataFlags flags = DefaultMimeDataFlags ) const;

  /**
   * Convert unicoded string to local encoding and use %-style
   * encoding for all common delimiters / non-ascii characters.
   * @param str String to encode (can be QString()).
   * @return the encoded string
   *
   * @deprecated use QUrl::toPercentEncoding instead, but note that it
   * returns a QByteArray and not a QString. Which makes sense since
   * everything is 7 bit (ascii) after being percent-encoded.
   */
  static KDE_DEPRECATED QString encode_string(const QString &str) {
    return QString::fromLatin1( QUrl::toPercentEncoding( str ).constData() ); //krazy:exclude=qclasses
  }

  /**
   * Convert unicoded string to local encoding and use %-style
   * encoding for all common delimiters / non-ascii characters
   * as well as the slash '/'.
   * @param str String to encode
   *
   * @deprecated use QUrl::toPercentEncoding(str,"/") instead, but note that it
   * returns a QByteArray and not a QString. Which makes sense since
   * everything is 7 bit (ascii) after being percent-encoded.
   *
   */
  static KDE_DEPRECATED QString encode_string_no_slash(const QString &str) {
      return QString::fromLatin1( QUrl::toPercentEncoding( str, "/" ).constData() ); //krazy:exclude=qclasses
  }

  /**
   * Decode %-style encoding and convert from local encoding to unicode.
   * Reverse of encode_string()
   * @param str String to decode (can be QString()).
   *
   * @deprecated use QUrl::fromPercentEncoding(encodedURL) instead, but
   * note that it takes a QByteArray and not a QString. Which makes sense since
   * everything is 7 bit (ascii) when being percent-encoded.
   *
   */
  static KDE_DEPRECATED QString decode_string(const QString &str) {
      return QUrl::fromPercentEncoding( str.toLatin1() ); //krazy:exclude=qclasses
  }


  /**
   * Convenience function.
   *
   * Returns whether '_url' is likely to be a "relative" URL instead of
   * an "absolute" URL.
   *
   * This is mostly meant for KUrl(url, relativeUrl).
   *
   * If you are looking for the notion of "relative path" (foo) vs "absolute path" (/foo),
   * use QUrl::isRelative() instead.
   * Indeed, isRelativeUrl() returns true for the string "/foo" since it doesn't contain a protocol,
   * while KUrl("/foo").isRelative() is false since the KUrl constructor turns it into file:///foo.
   * The two methods basically test the same thing, but this one takes a string (which is faster)
   * while the class method requires a QUrl/KUrl which gives a more expected result, given
   * the "magic" in the KUrl constructor.
   *
   * @param _url URL to examine
   * @return true when the URL is likely to be "relative", false otherwise.
   */
  static bool isRelativeUrl(const QString &_url);

  /**
   * Convenience function
   *
   * Returns a "relative URL" based on @p base_url that points to @p url.
   *
   * If no "relative URL" can be created, e.g. because the protocol
   * and/or hostname differ between @p base_url and @p url an absolute
   * URL is returned.
   * Note that if @p base_url represents a directory, it should contain
   * a trailing slash.
   * @param base_url the URL to derive from
   * @param url new URL
   * @see adjustPath()
   */
  static QString relativeUrl(const KUrl &base_url, const KUrl &url);

  /**
   * Convenience function
   *
   * Returns a relative path based on @p base_dir that points to @p path.
   * @param base_dir the base directory to derive from
   * @param path the new target directory
   * @param isParent A pointer to a boolean which, if provided, will be set to reflect
   * whether @p path has @p base_dir is a parent dir.
   */
  static QString relativePath(const QString &base_dir, const QString &path, bool *isParent=0);

private:
  void _setQuery( const QString& query );
  void _setEncodedUrl(const QByteArray& url);
  QString toString() const; // forbidden, use url(), prettyUrl(), or pathOrUrl() instead.
  operator QString() const; // forbidden, use url(), prettyUrl(), or pathOrUrl() instead.
private:
  KUrlPrivate* const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KUrl::EncodedPathAndQueryOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(KUrl::CleanPathOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(KUrl::QueryItemsOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(KUrl::EqualsOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(KUrl::DirectoryOptions)

Q_DECLARE_METATYPE(KUrl)
Q_DECLARE_METATYPE(KUrl::List)

/**
 * \relates KUrl
 * Compares URLs. They are parsed, split and compared.
 * Two malformed URLs with the same string representation
 * are nevertheless considered to be unequal.
 * That means no malformed URL equals anything else.
 */
KDECORE_EXPORT bool urlcmp( const QString& _url1, const QString& _url2 );

/**
 * \relates KUrl
 * Compares URLs. They are parsed, split and compared.
 * Two malformed URLs with the same string representation
 * are nevertheless considered to be unequal.
 * That means no malformed URL equals anything else.
 *
 * @param _url1 A reference URL
 * @param _url2 A URL that will be compared with the reference URL
 * @param options a set of KUrl::EqualsOption flags
 */
KDECORE_EXPORT bool urlcmp( const QString& _url1, const QString& _url2, const KUrl::EqualsOptions& options );

KDECORE_EXPORT uint qHash(const KUrl& kurl);

#endif
