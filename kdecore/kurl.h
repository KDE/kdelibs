/* This file is part of the KDE libraries
 *  Copyright (C) 1999 Torben Weis <weis@kde.org>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef __kurl_h__
#define __kurl_h__ "$Id$"

#include <qstring.h>
#include <qvaluelist.h>

class QUrl;
class QStringList;

class KURLPrivate;
/**
 * Represents and parses a URL.
 *
 * A prototypical URL looks like:
 * <pre>
 *   protocol:/user:password@hostname:port/path/to/file.ext#reference
 * </pre>
 *
 * KURL has some restrictions regarding the path
 * encoding. KURL works internally with the decoded path and
 * and encoded query. For example,
 * <pre>
 * http://localhost/cgi-bin/test%20me.pl?cmd=Hello%20you
 * </pre>
 * would result in a decoded path "/cgi-bin/test me.pl"
 * and in the encoded query "?cmd=Hello%20you".
 * Since path is internally always encoded you may @em not use
 * "%00" in the path, although this is OK for the query.
 *
 *  @author  Torben Weis <weis@kde.org>
 */
class KURL
{
public:
  enum AdjustementFlags
  {
    NoAdjustements = 0,
    StripFileProtocol = 1
  };

  /**
   * KURL::List is a QValueList that contains KURLs with a few
   * convenience methods.
   * @see KURL
   * @see QValueList
   */
  class List : public QValueList<KURL>
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
   *             @ref setPath().
   * @param encoding_hint MIB of original encoding of URL.
   *             @see QTextCodec::mibEnum()
   */
  KURL( const QString& url, int encoding_hint = 0 );
  /**
   * Constructor taking a char * @p url, which is an _encoded_ representation
   * of the URL, exactly like the usual constructor. This is useful when
   * then URL, in its encoded form, is strictly ascii.
   * @param url A encoded URL. If the URL does not have a protocol part, 
   *            "file:" is assumed.
   * @param encoding_hint MIB of original encoding of URL.
   * @see QTextCodec::mibEnum()
   */
  KURL( const char * url, int encoding_hint = 0 );
  /**
   * Constructor taking a QCString @p url, which is an _encoded_ representation
   * of the URL, exactly like the usual constructor. This is useful when
   * then URL, in its encoded form, is strictly ascii.
   * @param url A encoded URL. If the URL does not have a protocol part, 
   *            "file:" is assumed.
   * @param encoding_hint MIB of original encoding of URL.
   * @see QTextCodec::mibEnum()
   */
  KURL( const QCString& url, int encoding_hint = 0 );
  /**
   * Copy constructor.
   * @param u the KURL to copy
   */
  KURL( const KURL& u );
  /**
   * Converts from a @ref QUrl.
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
   * @param encoding_hint MIB of original encoding of URL.
   *             @see QTextCodec::mibEnum()
   */
  KURL( const KURL& _baseurl, const QString& _rel_url, int encoding_hint=0 );

  /**
   * Returns the protocol for the URL (i.e., file, http, etc.).
   * @return the protocol of the URL, does not include the colon. If the
   *         URL is malformed, QString::null will be returned.
   **/
  QString protocol() const { return m_bIsMalformed ? QString::null : m_strProtocol; }
  /**
   * Sets the protocol for the URL (i.e., file, http, etc.)
   * @param _txt the new protocol of the URL (without colon)
   **/
  void setProtocol( const QString& _txt );

  /**
   * Returns the decoded user name (login, user id, ...) included in the URL.
   * @return the user name or QString::null if there is no user name
   **/
  QString user() const { return m_strUser; }
  /**
   * Sets the user name (login, user id, ...) included in the URL.
   *
   * Special characters in the user name will appear encoded in the URL.
   * @param _txt the name of the user or QString::null to remove the user
   **/
  void setUser( const QString& _txt );
  /**
   * Test to see if this URL has a user name included in it.
   * @return true if the URL has an non-empty user name
   **/
  bool hasUser() const { return !m_strUser.isEmpty(); }

  /**
   * Returns the decoded password (corresponding to @ref user()) included in the URL.
   * @return the password or QString::null if it does not exist
   **/
  QString pass() const { return m_strPass; }
  /**
   * Sets the password (corresponding to @ref user()) included in the URL.
   *
   * Special characters in the password will appear encoded in the URL.
   * Note that a password can only appear in a URL string if you also set 
   * a user.
   * @param _txt the password to set or QString::null to remove the password
   * @see #setUser
   * @see #hasUser
   **/
  void setPass( const QString& _txt );
  /**
   * Test to see if this URL has a password included in it.
   * @return true if there is a non-empty password set
   **/
  bool hasPass() const { return !m_strPass.isEmpty(); }

  /**
   * Returns the decoded hostname included in the URL.
   * @return the name of the host or QString::null if no host is set
   **/
  QString host() const { return m_strHost; }
  /**
   * Sets the hostname included in the URL.
   *
   * Special characters in the hostname will appear encoded in the URL.
   * @param _txt the new name of the host or QString::null to remove the host
   **/
  void setHost( const QString& _txt );
  /**
   * Test to see if this URL has a hostname included in it.
   * @return true if the URL has a host
   **/
  bool hasHost() const { return !m_strHost.isEmpty(); }

  /**
   * Returns the port number included in the URL.
   * @return the port number. If there is no port number specified in the 
   *         URL, returns 0.
   **/
  unsigned short int port() const { return m_iPort; }
  /**
   * Sets the port number included in the URL.
   * @param _p the new port number or 0 to have no port number
   **/
  void setPort( unsigned short int _p );

  /**
   * Returns the current decoded path. This does @em not include the query.
   * @return the path of the URL (without query), or QString::null if no 
   *         path set. 
   */
  QString path() const  { return m_strPath; }

  /**
   * @param _trailing May be ( -1, 0 +1 ). -1 strips a trailing '/', +1 adds
   *                  a trailing '/' if there is none yet and 0 returns the
   *                  path unchanged. If the URL has no path, then no '/' is added
   *                  anyway. And on the other side: If the path is "/", then this
   *                  character won't be stripped. Reason: "ftp://weis@host" means something
   *                  completely different than "ftp://weis@host/". So adding or stripping
   *                  the '/' would really alter the URL, while "ftp://host/path" and
   *                  "ftp://host/path/" mean the same directory.
   *
   * @return The current decoded path. This does not include the query. Can
   *         be QString::null if no path is set.
   */
  QString path( int _trailing ) const;

  /**
   * Sets the path of the URL. The query is not changed by this function.
   *
   * @param path The new path. This is considered to be decoded. This 
   *             means: %3f does not become decoded
   *             and the ? does not indicate the start of the query part.
   *             Can be QString::null to delete the path.
   */
  void setPath( const QString& path );

  /**
   * Test to see if this URL has a path is included in it.
   * @return true if there is a path
   **/
  bool hasPath() const { return !m_strPath.isEmpty(); }

  /**
   * Resolves "." and ".." components in path.
   * Some servers seem not to like the removal of extra '/'
   * even though it is against the specification in RFC 2396.
   *
   * @param cleanDirSeparator if true, occurrances of consecutive
   * directory separators (e.g. /foo//bar) are cleaned up as well.
   */
  void cleanPath(bool cleanDirSeparator = true);

  /**
   * Add or remove a trailing slash to/from the path.
   * @param _trailing May be ( -1, 0 +1 ). -1 strips a trailing '/', +1 adds
   *                  a trailing '/' if there is none yet and 0 returns the
   *                  path unchanged. If the URL has no path, then no '/' is added
   *                  anyway. And on the other side: If the path is "/", then this
   *                  character won't be stripped. Reason: "ftp://weis@host" means something
   *                  completely different than "ftp://weis@host/". So adding or stripping
   *                  the '/' would really alter the URL, while "ftp://host/path" and
   *                  "ftp://host/path/" mean the same directory.
   */
  void adjustPath(int _trailing);

  /**
   * This is useful for HTTP. It looks first for '?' and decodes then.
   * The encoded path is the concatenation of the current path and the query.
   * @param _txt the new path and query.
   * @param encoding_hint MIB of original encoding of @p _txt .
   * @see QTextCodec::mibEnum()
   */
  void setEncodedPathAndQuery( const QString& _txt, int encoding_hint = 0 );

  /**
   * Sets the (already encoded) path
   * @param _txt the new path
   * @param encoding_hint MIB of original encoding of @p _txt .
   * @see QTextCodec::mibEnum()
   */
  void setEncodedPath(const QString& _txt, int encoding_hint = 0 );

  /**
   * Returns the encoded path and the query.
   *
   * @param _trailing May be ( -1, 0 +1 ). -1 strips a trailing '/', +1 adds
   *                  a trailing '/' if there is none yet and 0 returns the
   *                  path unchanged. If the URL has no path, then no '/' is added
   *                  anyway. And on the other side: If the path is "/", then this
   *                  character won't be stripped. Reason: "ftp://weis@host" means something
   *                  completely different than "ftp://weis@host/". So adding or stripping
   *                  the '/' would really alter the URL, while "ftp://host/path" and
   *                  "ftp://host/path/" mean the same directory.
   * @param _no_empty_path If set to true then an empty path is substituted by "/".
   * @param encoding_hint MIB of desired encoding of URL.
   *             @see QTextCodec::mibEnum()
   * @return The concatenation if the encoded path , '?' and the encoded query.
   *
   */
  QString encodedPathAndQuery( int _trailing = 0, bool _no_empty_path = false, int encoding_hint = 0) const;

  /**
   * @param _txt This is considered to be encoded. This has a good reason:
   * The query may contain the 0 character.
   *
   * The query should start with a '?'. If it doesn't '?' is prepended.
   * @param encoding_hint Reserved, should be 0.
   * @see QTextCodec::mibEnum()
   */
  void setQuery( const QString& _txt, int encoding_hint = 0);

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
  QString ref() const { return m_strRef_encoded; }

  /**
   * Sets the reference part (everything after '#').
   * @param _txt The encoded reference (or QString::null to remove it).
   */
  void setRef( const QString& _txt ) { m_strRef_encoded = _txt; }

  /**
   * Checks whether the URL has a reference part.
   * @return true if the URL has a reference part. In a URL like
   *         http://www.kde.org/kdebase.tar#tar:/README it would
   *         return true, too.
   */
  bool hasRef() const { return !m_strRef_encoded.isNull(); }

  /**
   * Returns the HTML reference (the part of the URL after "#").
   * @return The HTML-style reference.
   * @see #split
   * @see #hasSubURL
   * @see #encodedHtmlRef
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
   *         contrast to @ref setRef(). Use QString::null to remove it.
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
   * Checks whether the URL is well formed.
   * @return false if the URL is malformed. This function does @em not test
   *         whether sub URLs are well-formed, too.
   */
  bool isValid() const  { return !m_bIsMalformed; }
  /**
   * @deprecated
   */
  bool isMalformed() const { return !isValid(); }

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
   * Checks whether the URL has any sub URLs. See @ref #split()
   * for examples for sub URLs.
   * @return true if the file has at least one sub URL.
   * @see #split
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
   * @param _txt The text to add. It is considered to be decoded.
   */
  void addPath( const QString& _txt );

  /**
   * Returns the value of a certain query item.
   *
   * @param _item Item whose value we want
   *
   * @return the value of the given query item name or QString::null if the
   * specified item does not exist.
   */
  QString queryItem( const QString& _item ) const;

  /**
   * Add an additional query item.
   * To replace an existing query item, the item should first be
   * removed with @ref removeQueryItem()
   *
   * @param _item Name of item to add
   * @param _value Value of item to add
   * @param encoding_hint MIB of encoding to use for _value.
   *             @see QTextCodec::mibEnum()
   */
  void addQueryItem( const QString& _item, const QString& _value, int encoding_hint = 0 );

  /**
   * Remove an item from the query.
   *
   * @param _item Item to be removed
   */
  void removeQueryItem( const QString& _item );

  /**
   * Sets the filename of the path.
   * In comparison to @ref addPath() this function does not assume that the current 
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
   *        <tt>file:/hallo/torben/</tt> and <tt>file:/hallo/torben</tt>.
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
   * @param _ignore_trailing_slash_in_path means that <tt>file:/hallo/torben</tt> and
   *                                       <tt>file:/hallo/torben/"</tt> would both return <tt>/hallo/</tt>
   *                                       or <tt>/hallo</tt> depending on the other flag
   * @return The directory part of the current path. Everything between the last and the second last '/'
   *         is returned. For example <tt>file:/hallo/torben/</tt> would return "/hallo/torben/" while
   *         <tt>file:/hallo/torben</tt> would return "hallo/". The returned string is decoded. QString::null is returned when there is no path.
   */
  QString directory( bool _strip_trailing_slash_from_result = true,
		     bool _ignore_trailing_slash_in_path = true ) const;

  /**
   * Changes the directory by descending into the given directory.
   * It is assumed the current URL represents a directory.
   * If @p dir starts with a "/" the
   * current URL will be "protocol://host/dir" otherwise @p _dir will
   * be appended to the path. @p _dir can be ".."
   * This function won't strip protocols. That means that when you are in
   * file:/dir/dir2/my.tgz#tar:/ and you do cd("..") you will
   * still be in file:/dir/dir2/my.tgz#tar:/
   *
   * @param _dir the directory to change to
   * @return true if successful
   */
  bool cd( const QString& _dir );

  /**
   * Returns the URL as string.
   * @return The complete URL, with all escape sequences intact, encoded
   * in a given charset.
   * This is used in particular for encoding URLs in UTF-8 before using them
   * in a drag and drop operation.
   *
   * @param _trailing This may be ( -1, 0 +1 ). -1 strips a trailing '/' from the path, +1 adds
   *                  a trailing '/' if there is none yet and 0 returns the
   *                  path unchanged.
   * @param encoding_hint MIB of encoding to use.
   * @see QTextCodec::mibEnum()
   */
  QString url( int _trailing = 0, int encoding_hint = 0) const;

  /**
   * Returns the URL as string in human-friendly format.
   * Example: http://localhost:8080/test.cgi?test=hello world&name=fred
   * @return A human readable URL, with no non-necessary encodings/escaped
   * characters.
   */
  QString prettyURL( int _trailing = 0) const;
  
  
  /**
   * Returns the URL as string, escaped for HTML.
   * @return A human readable URL, with no non-necessary encodings/escaped
   * characters which is html encoded for safe inclusion in html or
   * rich text.
   */
  QString htmlURL() const;
  
  /**
   * Returns the URL as string, escaped for HTML.
   * Example: http://localhost:8080/test.cgi?test=hello world&name=fred
   * @return A human readable URL, with no non-necessary encodings/escaped
   * characters.
   */
  QString prettyURL( int _trailing, AdjustementFlags _flags) const;
  // ### BIC: Merge the two above

  /**
   * Test to see if the KURL is empty.
   * @return true if the URL is empty
   **/
  bool isEmpty() const;

  /**
   * This function is useful to implement the "Up" button in a file manager for example.
   * @ref cd() never strips a sub-protocol. That means that if you are in
   * file:/home/x.tgz#gzip:/#tar:/ and hit the up button you expect to see
   * file:/home. The algorithm tries to go up on the right-most URL. If that is not
   * possible it strips the right most URL. It continues stripping URLs.
   * @return a URL that is a level higher
   */
  KURL upURL( ) const;

  KURL& operator=( const KURL& _u );
  KURL& operator=( const QString& _url );
  KURL& operator=( const char * _url );
  KURL& operator=( const QUrl & u );

  bool operator==( const KURL& _u ) const;
  bool operator==( const QString& _u ) const;
  bool operator!=( const KURL& _u ) const { return !( *this == _u ); }
  bool operator!=( const QString& _u ) const { return !( *this == _u ); }

  /**
   * The same as equals(), just with a less obvious name.
   * Compares this url with @p u.
   * @param ignore_trailing set to true to ignore trailing '/' characters.
   * @return true if both urls are the same
   * @see operator==. This function should be used if you want to
   * ignore trailing '/' characters.
   * @deprecated
   */
  bool cmp( const KURL &u, bool ignore_trailing = false ) const;

  /**
   * Compares this url with @p u.
   * @param ignore_trailing set to true to ignore trailing '/' characters.
   * @return true if both urls are the same
   * @see operator==. This function should be used if you want to
   * ignore trailing '/' characters.
   * @since 3.1
   */
  bool equals( const KURL &u, bool ignore_trailing = false ) const;

  /**
   * Checks whether the given URL is parent of this URL. 
   * For instance, ftp://host/dir/ is a parent of ftp://host/dir/subdir/subsubdir/.
   * @return true if this url is a parent of @p u (or the same URL as @p u)
   */
  bool isParentOf( const KURL& u ) const;

  /**
   * Splits nested URLs like file:/home/weis/kde.tgz#gzip:/#tar:/kdebase
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
   * @see #hasSubURL
   */
  static List split( const QString& _url );

  /**
   * Splits nested URLs like file:/home/weis/kde.tgz#gzip:/#tar:/kdebase
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
   * @see #hasSubURL
   */
  static List split( const KURL& _url );

  /**
   * Reverses @ref #split(). Only the first URL may have a reference. This reference
   * is considered to be HTML-like and is appended at the end of the resulting
   * joined URL.
   * @param _list the list to join
   * @return the joined URL
   */
  static KURL join( const List& _list );

  /**
   * Convenience function.
   *
   * Convert unicoded string to local encoding and use %-style
   * encoding for all common delimiters / non-ascii characters.
   * @param str String to encode (can be QString::null).
   * @param encoding_hint MIB of encoding to use.
   *             @see QTextCodec::mibEnum()
   * @return the encoded string
   **/
  static QString encode_string(const QString &str, int encoding_hint = 0);

  /**
   * Convenience function.
   *
   * Convert unicoded string to local encoding and use %-style
   * encoding for all common delimiters / non-ascii characters
   * as well as the slash '/'.
   * @param str String to encode
   * @param encoding_hint MIB of encoding to use.
   *             @see QTextCodec::mibEnum()
   **/
  static QString encode_string_no_slash(const QString &str, int encoding_hint = 0);

  /**
   * Convenience function.
   *
   * Decode %-style encoding and convert from local encoding to unicode.
   *
   * Reverse of encode_string()
   * @param str String to decode (can be QString::null).
   * @param encoding_hint MIB of original encoding of @p str .
   *             @see QTextCodec::mibEnum()
   **/
  static QString decode_string(const QString &str, int encoding_hint = 0);

  /**
   * Convenience function.
   *
   * Returns whether '_url' is likely to be a "relative" URL instead of
   * an "absolute" URL.
   * @param _url URL to examine
   * @return true when the URL is likely to be "relative", false otherwise.
   */
  static bool isRelativeURL(const QString &_url);

#ifdef KDE_NO_COMPAT
private:
#endif
  QString filename( bool _ignore_trailing_slash_in_path = true ) const
  {
    return fileName(_ignore_trailing_slash_in_path);
  }

protected:
  void reset();
  void parse( const QString& _url, int encoding_hint = 0);

private:
  QString m_strProtocol;
  QString m_strUser;
  QString m_strPass;
  QString m_strHost;
  QString m_strPath;
  QString m_strRef_encoded;
  QString m_strQuery_encoded;
  bool m_bIsMalformed : 1;
  int freeForUse      : 7;
  unsigned short int m_iPort;
  QString m_strPath_encoded;

  friend QDataStream & operator<< (QDataStream & s, const KURL & a);
  friend QDataStream & operator>> (QDataStream & s, KURL & a);
private:
  KURLPrivate* d;
};

/**
 * Compares URLs. They are parsed, split and compared.
 * Two malformed URLs with the same string representation
 * are nevertheless considered to be unequal.
 * That means no malformed URL equals anything else.
 */
bool urlcmp( const QString& _url1, const QString& _url2 );

/**
 * Compares URLs. They are parsed, split and compared.
 * Two malformed URLs with the same string representation
 * are nevertheless considered to be unequal.
 * That means no malformed URL equals anything else.
 *
 * @param _ignore_trailing Described in @ref KURL::cmp
 * @param _ignore_ref If true, disables comparison of HTML-style references.
 */
bool urlcmp( const QString& _url1, const QString& _url2, bool _ignore_trailing, bool _ignore_ref );

QDataStream & operator<< (QDataStream & s, const KURL & a);
QDataStream & operator>> (QDataStream & s, KURL & a);

#endif
