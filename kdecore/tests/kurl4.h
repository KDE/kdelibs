/* This file is part of the KDE libraries
 *  Copyright (C) 1999 Torben Weis <weis@kde.org>
 *  Copyright (C) 2005 David Faure <faure@kde.org>
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
 **/

#ifndef kurl_h
#define kurl_h

#include <qurl.h>
#include "kdelibs_export.h"
#include <qmap.h>

class QStringList;
class QMimeData;

class KURLPrivate;
#define KURL KURL4

// maybe we should encapsulate QUrl instead of inheriting from it.
// this would even allow us to inherit from KUri instead.
// and this way hacks like setPath() would be less ugly, and we could avoid
// half KDE code using setScheme() and the other half using setProtocol(), etc.
// (DF)
// TODO: fromPath(), since if we use QUrl::fromLocalFile, we then have to make a QUrl->KURL copy.
class KDECORE_EXPORT KURL : public QUrl
{
public:
  typedef QMap<QString, QString> MetaDataMap;
  enum MimeDataFlags { DefaultMimeDataFlags = 0, NoTextExport = 1 };

  /**
   * KURL::List is a QList that contains KURLs with a few
   * convenience methods.
   * @see KURL
   * @see QValueList
   */
  class KDECORE_EXPORT List : public QList<KURL>
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
      List(const KURL &url);
      /**
       * Creates a list that contains the URLs from the given
       * list.
       * @param list the list containing the URLs as strings
       */
      List(const QStringList &list);
      /**
       * Converts the URLs of this list to a list of strings.
       * @return the list of strings
       */
      QStringList toStringList() const;

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
       *
       * @since 4.0
       */
      void populateMimeData( QMimeData* mimeData,
                          const KURL::MetaDataMap& metaData = MetaDataMap(),
                          MimeDataFlags flags = DefaultMimeDataFlags );

      /**
       * Return true if @p mimeData contains URI data
       * @since 4.0
       */
      static bool canDecode( const QMimeData *mimeData );

      /**
       * Extract a list of KURLs from the contents of @p mimeData.
       * Decoding will fail if @p mimeData does not contain any URLs, or if at
       * least one extracted URL is not valid.
       * @param mimeData the mime data to extract from; cannot be 0
       * @param metaData optional pointer to a map holding the metadata
       * @return the list of urls
       * @since 4.0
       */
      static KURL::List fromMimeData( const QMimeData *mimeData, KURL::MetaDataMap* metaData = 0 );

  };
  /**
   * Constructs an empty URL.
   */
  KURL();

  /**
   * Destructs the KURL object.
   */
  ~KURL();

  /**
   * Usual constructor, to construct from a string.
   * @param url A URL, not a filename. If the URL does not have a protocol
   *             part, "file:" is assumed.
   *             It is dangerous to feed unix filenames into this function,
   *             this will work most of the time but not always.
   *             For example "/home/Torben%20Weis" will be considered a URL
   *             pointing to the file "/home/Torben Weis" instead of to the
   *             file "/home/Torben%20Weis".
   *             This means that if you have a usual UNIX like path you
   *             should not use this constructor.
   *             Instead create an empty url and set the path by using
   * setPath().
   */
  KURL( const QString& url );
  /**
   * Constructor taking a char * @p url, which is an _encoded_ representation
   * of the URL, exactly like the usual constructor. This is useful when
   * then URL, in its encoded form, is strictly ascii.
   * @param url A encoded URL. If the URL does not have a protocol part,
   *            "file:" is assumed.
   */
  KURL( const char * url );
  /**
   * Constructor taking a QByteArray @p url, which is an _encoded_ representation
   * of the URL, exactly like the usual constructor. This is useful when
   * then URL, in its encoded form, is strictly ascii.
   * @param url A encoded URL. If the URL does not have a protocol part,
   *            "file:" is assumed.
   */
  KURL( const QByteArray& url );

  /**
   * Copy constructor.
   * @param u the KURL to copy
   */
  KURL( const KURL& u );
  /**
   * Converts from a QUrl.
   * @param u the QUrl
   */
  KURL( const QUrl &u );
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
  KURL( const KURL& _baseurl, const QString& _rel_url );

  /**
   * Returns the protocol for the URL (i.e., file, http, etc.), lowercased.
   * @see QUrl::scheme
   */
  QString protocol() const { return scheme().toLower(); }

  /**
   * Sets the protocol for the URL (i.e., file, http, etc.)
   * @param _txt the new protocol of the URL (without colon)
   */
  void setProtocol( const QString& proto ) { setScheme( proto ); }

  /**
   * Returns the decoded user name (login, user id, ...) included in the URL.
   * @return the user name or QString::null if there is no user name
   */
  QString user() const { return userName(); }

  /**
   * Sets the user name (login, user id, ...) included in the URL.
   *
   * Special characters in the user name will appear encoded in the URL.
   * @param _txt the name of the user or QString::null to remove the user
   */
  void setUser( const QString& user ) { setUserName( user ); }

  /**
   * Test to see if this URL has a user name included in it.
   * @return true if the URL has an non-empty user name
   */
  bool hasUser() const { return !userName().isEmpty(); }

  /**
   * Returns the decoded password (corresponding to user()) included in the URL.
   * @return the password or QString::null if it does not exist
   **/
  QString pass() const { return password(); }

  /**
   * Sets the password (corresponding to user()) included in the URL.
   *
   * Special characters in the password will appear encoded in the URL.
   * Note that a password can only appear in a URL string if you also set
   * a user.
   * @param _txt the password to set or QString::null to remove the password
   * @see setUser
   * @see hasUser
   **/
  void setPass( const QString& pass ) { setPassword( pass ); }

  /**
   * Test to see if this URL has a password included in it.
   * @return true if there is a non-empty password set
   **/
  bool hasPass() const { return !password().isEmpty(); }

  /**
   * Test to see if this URL has a hostname included in it.
   * @return true if the URL has a host
   **/
  bool hasHost() const { return !host().isEmpty(); }

  /**
   * @param _trailing May be ( -1, 0 +1 ). -1 strips a trailing '/', +1 adds
   *                  a trailing '/' if there is none yet and 0 returns the
   *                  path unchanged. If the URL has no path, then no '/' is added
   *                  anyway. And on the other side: If the path is "/", then this
   *                  character won't be stripped. Reason: "ftp://weis\@host" means something
   *                  completely different than "ftp://weis\@host/". So adding or stripping
   *                  the '/' would really alter the URL, while "ftp://host/path" and
   *                  "ftp://host/path/" mean the same directory.
   *
   * @return The current decoded path. This does not include the query. Can
   *         be QString::null if no path is set.
   */
  QString path( int _trailing ) const;
  QString path() const { return QUrl::path(); }

  /// \reimp so that KURL u; u.setPath(path); implies "file" protocol.
  void setPath( const QString& path );

  /**
   * Test to see if this URL has a path is included in it.
   * @return true if there is a path
   **/
  bool hasPath() const { return !path().isEmpty(); }

  /**
   * Resolves "." and ".." components in path.
   * Some servers seem not to like the removal of extra '/'
   * even though it is against the specification in RFC 2396.
   *
   * @param cleanDirSeparator if true, occurrences of consecutive
   * directory separators (e.g. /foo//bar) are cleaned up as well.
   */
  void cleanPath(bool cleanDirSeparator = true);

  /**
   * Add or remove a trailing slash to/from the path.
   * @param _trailing May be -1, 0, or +1. -1 strips any trailing '/', +1 adds
   *                  a trailing '/' if there is none yet and 0 returns the
   *                  path unchanged. If the URL has no path, then no '/' is added
   *                  anyway. And on the other side: If the path is "/", then this
   *                  character won't be stripped. Reason: "ftp://weis\@host" means something
   *                  completely different than "ftp://weis\@host/". So adding or stripping
   *                  the '/' would really alter the URL, while "ftp://host/path" and
   *                  "ftp://host/path/" mean the same directory.
   */
  void adjustPath(int _trailing);

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
   * Returns the encoded path and the query.
   *
   * @param _trailing May be ( -1, 0 +1 ). -1 strips a trailing '/', +1 adds
   *                  a trailing '/' if there is none yet and 0 returns the
   *                  path unchanged. If the URL has no path, then no '/' is added
   *                  anyway. And on the other side: If the path is "/", then this
   *                  character won't be stripped. Reason: "ftp://weis\@host" means something
   *                  completely different than "ftp://weis\@host/". So adding or stripping
   *                  the '/' would really alter the URL, while "ftp://host/path" and
   *                  "ftp://host/path/" mean the same directory.
   * @param _no_empty_path If set to true then an empty path is substituted by "/".
   * @return The concatenation if the encoded path , '?' and the encoded query.
   *
   */
  QString encodedPathAndQuery( int _trailing = 0, bool _no_empty_path = false ) const;

  /**
   * @param _txt This is considered to be encoded. This has a good reason:
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
   * @return The encoded query, or QString::null if there is none.
   */
  QString query() const;

  /**
   * The reference is @em never decoded automatically.
   * @return the undecoded reference, or QString::null if there is none
   */
  QString ref() const { return QUrl::toPercentEncoding( fragment() ); }

  /**
   * Sets the reference part (everything after '#').
   * @param fragment the encoded reference (or QString::null to remove it).
   */
  void setRef( const QString& fragment ) { setFragment( QUrl::fromPercentEncoding( fragment.latin1() ) ); }

  /**
   * Checks whether the URL has a reference part.
   * @return true if the URL has a reference part. In a URL like
   *         http://www.kde.org/kdebase.tar#tar:/README it would
   *         return true, too.
   */
  bool hasRef() const { return !fragment().isNull(); }

  /**
   * Returns the HTML reference (the part of the URL after "#").
   * @return The HTML-style reference.
   * @see split
   * @see hasSubURL
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
   *         contrast to setRef(). Use QString::null to remove it.
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
   * @param encoding the encoding to add or QString::null to remove the
   *                 encoding.
   */
  void setFileEncoding(const QString &encoding);

  /**
   * Returns encoding information from url, the content of the "charset"
   * parameter.
   * @return An encoding suitable for QTextCodec::codecForName()
   *         or QString::null if not encoding was specified.
   */
  QString fileEncoding() const;

  /**
   * Checks whether the URL has any sub URLs. See split()
   * for examples for sub URLs.
   * @return true if the file has at least one sub URL.
   * @see split
   */
  bool hasSubURL() const;

  /**
   * Adds to the current path.
   * Assumes that the current path is a directory. @p _txt is appended to the
   * current path. The function adds '/' if needed while concatenating.
   * This means it does not matter whether the current path has a trailing
   * '/' or not. If there is none, it becomes appended. If @p _txt
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
   *
   * @since 3.1
   **/
  enum QueryItemsOptions { CaseInsensitiveKeys = 1 };

  /**
   * Returns the list of query items as a map mapping keys to values.
   *
   * @param options any of QueryItemsOptions <em>or</em>ed together.
   *
   * @return the map of query items or the empty map if the url has no
   * query items.
   *
   * @since 3.1
   */
  QMap< QString, QString > queryItems( int options ) const;
  // #### TODO port the above queryItems to look more like QUrl's
  using QUrl::queryItems; // temporary

  KDE_DEPRECATED QString queryItem(const QString &key) const {
    return QUrl::queryItemValue( key );
  }

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
   * Returns the filename of the path.
   * @param _ignore_trailing_slash_in_path This tells whether a trailing '/' should
   *        be ignored. This means that the function would return "torben" for
   *        <tt>file:///hallo/torben/</tt> and <tt>file:///hallo/torben</tt>.
   *        If the flag is set to false, then everything behind the last '/'
   *        is considered to be the filename.
   * @return The filename of the current path. The returned string is decoded. Null
   *         if there is no file (and thus no path).
   */
  QString fileName( bool _ignore_trailing_slash_in_path = true ) const;

  /**
   * Returns the directory of the path.
   * @param _strip_trailing_slash_from_result tells whether the returned result should end with '/' or not.
   *                                          If the path is empty or just "/" then this flag has no effect.
   * @param _ignore_trailing_slash_in_path means that <tt>file:///hallo/torben</tt> and
   *                                       <tt>file:///hallo/torben/"</tt> would both return <tt>/hallo/</tt>
   *                                       or <tt>/hallo</tt> depending on the other flag
   * @return The directory part of the current path. Everything between the last and the second last '/'
   *         is returned. For example <tt>file:///hallo/torben/</tt> would return "/hallo/torben/" while
   *         <tt>file:///hallo/torben</tt> would return "hallo/". The returned string is decoded. QString::null is returned when there is no path.
   */
  QString directory( bool _strip_trailing_slash_from_result = true,
		     bool _ignore_trailing_slash_in_path = true ) const;

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
   * user, use prettyURL().
   *
   * @param _trailing This may be ( -1, 0 +1 ). -1 strips a trailing '/' from the path, +1 adds
   *                  a trailing '/' if there is none yet and 0 returns the
   *                  path unchanged.
   * @return The complete URL, with all escape sequences intact, encoded
   * in a given charset.
   * @see prettyURL()
   */
  QString url( int _trailing = 0 ) const;

  /**
   * Returns the URL as string in human-friendly format.
   * Example:
   * \code
   * http://localhost:8080/test.cgi?test=hello world&name=fred
   * \endcode
   * @param _trailing -1 to strip a trailing '/' from the path, +1 adds
   *                  a trailing '/' if there is none yet and 0 returns the
   *                  path unchanged.
   * KDE4: +1 is not supported anymore
   *
   * @return A human readable URL, with no non-necessary encodings/escaped
   * characters. Password will not be shown.
   * @see url()
   */
  QString prettyURL( int _trailing = 0 ) const;

  /**
   * Return the URL as a string, which will be either the URL (as prettyURL
   * would return) or, when the URL is a local file without query or ref,
   * the path.
   * Use this method, together with its opposite, fromPathOrURL(),
   * to display and even let the user edit URLs.
   *
   * @return the new KURL
   * @since 3.4
   */
  QString pathOrURL() const;

  /**
   * Returns the URL as a string, using the standard conventions for mime data
   * (drag-n-drop or copy-n-paste).
   * Internally used by KURL::List::fromMimeData, which is probably what you want to use instead.
   * @since 4.0
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
  KURL upURL( ) const;

#if 0
  KURL& operator=( const KURL& _u );
  KURL& operator=( const QString& _url );
  KURL& operator=( const char * _url );
  KURL& operator=( const QUrl & u );
#endif

  bool operator==( const KURL& _u ) const;
  bool operator==( const QString& _u ) const;
  bool operator!=( const KURL& _u ) const { return !( *this == _u ); }
  bool operator!=( const QString& _u ) const { return !( *this == _u ); }

  /**
   * The same as equals(), just with a less obvious name.
   * Compares this url with @p u.
   * @param u the URL to compare this one with.
   * @param ignore_trailing set to true to ignore trailing '/' characters.
   * @return true if both urls are the same
   * @see operator==. This function should be used if you want to
   * ignore trailing '/' characters.
   * @deprecated Use equals() instead.
   */
  bool cmp( const KURL &u, bool ignore_trailing = false ) const KDE_DEPRECATED;

  /**
   * Compares this url with @p u.
   * @param u the URL to compare this one with.
   * @param ignore_trailing set to true to ignore trailing '/' characters.
   * @return true if both urls are the same
   * @see operator==. This function should be used if you want to
   * ignore trailing '/' characters.
   * @since 3.1
   */
  bool equals( const KURL &u, bool ignore_trailing = false ) const; // TODO KDE4: add bool _ignore_ref = false

  /**
   * Checks whether the given URL is parent of this URL.
   * For instance, ftp://host/dir/ is a parent of ftp://host/dir/subdir/subsubdir/.
   * @return true if this url is a parent of @p u (or the same URL as @p u)
   *
   */
  bool isParentOf( const KURL& u ) const { return QUrl::isParentOf( u ) || equals( u, true ); }
    // (this overload of the QUrl method allows to use the implicit KURL constructors)
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
   * @see hasSubURL
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
   * @see hasSubURL
   */
  static List split( const KURL& _url );

  /**
   * Reverses split(). Only the first URL may have a reference. This reference
   * is considered to be HTML-like and is appended at the end of the resulting
   * joined URL.
   * @param _list the list to join
   * @return the joined URL
   */
  static KURL join( const List& _list );

  /**
   * Creates a KURL object from a QString representing either an absolute path
   * or a real URL. Use this method instead of
   * \code
   * QString someDir = ...
   * KURL url = someDir;
   * \endcode
   *
   * Otherwise some characters (e.g. the '#') won't be encoded properly.
   * @param text the string representation of the URL to convert
   * @return the new KURL
   * @since 3.1
   */
  static KURL fromPathOrURL( const QString& text );

  /**
   * Creates a KURL from a string, using the standard conventions for mime data
   * (drag-n-drop or copy-n-paste).
   * Internally used by KURL::List::fromMimeData, which is probably what you want to use instead.
   * @since 4.0
   */
  static KURL fromMimeDataByteArray( const QByteArray& str );

  /**
   * Adds URL data into the given QMimeData.
   *
   * By default, populateMimeData also exports the URL as plain text, for e.g. dropping
   * onto a text editor.
   * But in some cases this might not be wanted, e.g. if adding other mime data
   * which provides better plain text data.
   *
   * WARNING: do not call this method multiple times, use KURL::List::populateMimeData instead.
   *
   * @param mimeData the QMimeData instance used to drag or copy this URL
   * @param metaData KIO metadata shipped in the mime data, which is used for instance to
   * set a correct HTTP referrer (some websites require it for downloading e.g. an image)
   * @param flags set NoTextExport to prevent setting plain/text data into @p mimeData
   * In such a case, setExportAsText( false ) should be called.
   *
   * @since 4.0
   */
  void populateMimeData( QMimeData* mimeData,
                      const MetaDataMap& metaData = MetaDataMap(),
                      MimeDataFlags flags = DefaultMimeDataFlags );

  /**
   * Convenience function.
   *
   * Convert unicoded string to local encoding and use %-style
   * encoding for all common delimiters / non-ascii characters.
   * @param str String to encode (can be QString::null).
   * @return the encoded string
   */
  static KDE_DEPRECATED QString encode_string(const QString &str) {
      return QLatin1String( QUrl::toPercentEncoding( str ) );
  }

  /**
   * Convenience function.
   *
   * Convert unicoded string to local encoding and use %-style
   * encoding for all common delimiters / non-ascii characters
   * as well as the slash '/'.
   * @param str String to encode
   */
  static KDE_DEPRECATED QString encode_string_no_slash(const QString &str) {
      return QLatin1String( QUrl::toPercentEncoding( str, "/" ) );
  }

  /**
   * Convenience function.
   *
   * Decode %-style encoding and convert from local encoding to unicode.
   *
   * Reverse of encode_string()
   * @param str String to decode (can be QString::null).
   **/
  static KDE_DEPRECATED QString decode_string(const QString &str) {
      return QUrl::fromPercentEncoding( str.latin1() ); // ## TODO check encoding
  }


  /**
   * Convenience function.
   *
   * Returns whether '_url' is likely to be a "relative" URL instead of
   * an "absolute" URL.
   * @param _url URL to examine
   * @return true when the URL is likely to be "relative", false otherwise.
   */
  static bool isRelativeURL(const QString &_url);

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
  static QString relativeURL(const KURL &base_url, const KURL &url);

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
private:
  KURLPrivate* d;
};

/**
 * \relates KURL
 * Compares URLs. They are parsed, split and compared.
 * Two malformed URLs with the same string representation
 * are nevertheless considered to be unequal.
 * That means no malformed URL equals anything else.
 */
KDECORE_EXPORT bool urlcmp( const QString& _url1, const QString& _url2 );

/**
 * \relates KURL
 * Compares URLs. They are parsed, split and compared.
 * Two malformed URLs with the same string representation
 * are nevertheless considered to be unequal.
 * That means no malformed URL equals anything else.
 *
 * @param _url1 A reference URL
 * @param _url2 A URL that will be compared with the reference URL
 * @param _ignore_trailing Described in KURL::cmp
 * @param _ignore_ref If true, disables comparison of HTML-style references.
 */
KDECORE_EXPORT bool urlcmp( const QString& _url1, const QString& _url2, bool _ignore_trailing, bool _ignore_ref ); // KDE4 TODO: new method with QUrl::FormattingOptions instead of two bools

#endif
