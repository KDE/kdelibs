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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef __kurl_h__
#define __kurl_h__

#include <qstring.h>
#include <qvaluelist.h>
#include "kdelibs_export.h"

class QUrl;
class QStringList;
template <typename K, typename V> class QMap;

class KURLPrivate;

// Defines that file-urls look like file:///path/file instead of file:/path/file
#define KURL_TRIPLE_SLASH_FILE_PROT

/**
 * @brief Represents and parses a URL
 *
 * A prototypical URL looks like:
 * @code
 *   protocol://user:password@hostname:port/path/to/file.ext#reference
 * @endcode
 *
 * KURL handles escaping of URLs. This means that the specification
 * of a full URL will differ from the corresponding string that would specify a
 * local file or directory in file-operations like fopen. This is because an URL
 * doesn't allow certain characters and escapes them.
 *
 * For examle:
 * - '#' -> "%23"
 *  (In a URL the hash-character @c '#' is used to specify a "reference", i.e.
 *  the position within a document)
 * - space -> "%20"
 *
 * The constructor KURL(const QString&) expects a string properly escaped,
 * or at least non-ambiguous.
 * For instance a local file or directory <tt>"/bar/#foo#"</tt> would have the
 * URL <tt>"file:///bar/%23foo%23"</tt>.
 * If you have the absolute path and need the URL-escaping you should create
 * KURL via the default-constructor and then call setPath(const QString&):
 * @code
 *     KURL kurl;
 *     kurl.setPath( "/bar/#foo#" );
 *     QString url = kurl.url();    // -> "file:///bar/%23foo%23"
 * @endcode
 *
 * If you have the URL of a local file or directory and need the absolute path,
 * you would use path().
 * @code
 *    KURL url( "file:///bar/%23foo%23" );
 *    ...
 *    if ( url.isLocalFile() )
 *       QString path = url.path();       // -> "/bar/#foo#"
 * @endcode
 *
 * The other way round: if the user can enter a string, that can be either a
 * path or a URL, then you need to use KURL::fromPathOrURL() to build a KURL.
 *
 * This must also be considered, when you have separated directory and file
 * strings and need to put them together.
 * While you can simply concatenate normal path strings, you must take care if
 * the directory-part is already an escaped URL.
 * (This might be needed if the user specifies a relative path, and your
 * program supplies the rest from elsewhere.)
 *
 * Wrong:
 * @code
 *    QString dirUrl = "file:///bar/";
 *    QString fileName = "#foo#";
 *    QString invalidURL = dirUrl + fileName;   // -> "file:///bar/#foo#" won't behave like you would expect.
 * @endcode
 * Instead you should use addPath().
 *
 * Right:
 * @code
 *    KURL url( "file:///bar/" );
 *    QString fileName = "#foo#";
 *    url.addPath( fileName );
 *    QString validURL = url.url();    // -> "file:///bar/%23foo%23"
 * @endcode
 *
 * Also consider that some URLs contain the password, but this shouldn't be
 * visible. Your program should use prettyURL() every time it displays a
 * URL, whether in the GUI or in debug output or...
 *
 * @code
 *    KURL url( "ftp://name:password@ftp.faraway.org/bar/%23foo%23");
 *    QString visibleURL = url.prettyURL(); // -> "ftp://name@ftp.faraway.org/bar/%23foo%23"
 * @endcode
 * Note that prettyURL() doesn't change the character escapes (like <tt>"%23"</tt>).
 * Otherwise the URL would be invalid and the user wouldn't be able to use it in another
 * context.
 *
 * KURL has some restrictions regarding the path
 * encoding. KURL works internally with the decoded path and
 * and encoded query. For example,
 * @code
 *    http://localhost/cgi-bin/test%20me.pl?cmd=Hello%20you
 * @endcode
 * would result in a decoded path <tt>"/cgi-bin/test me.pl"</tt>
 * and in the encoded query <tt>"?cmd=Hello%20you"</tt>.
 * Since path is internally always encoded you may @em not use
 * <tt>"%00"</tt> in the path, although this is OK for the query.
 *
 *  @author  Torben Weis <weis@kde.org>
 */
class KDECORE_EXPORT KURL
{
public:
  /**
   * Flags to choose how file: URLs are treated when creating their QString
   * representation with prettyURL(int,AdjustementFlags)
   *
   * However it is recommended to use pathOrURL() instead of this variant of prettyURL()
   */
  enum AdjustementFlags
  {
     /**
     * Do not treat file: URLs differently
     */
    NoAdjustements = 0,
    /**
     * Strip the file: protocol from the string, i.e. return only the path and
     * filename as a local path
     */
    StripFileProtocol = 1
  };

  /**
   * Defines the type of URI we are processing.
   */
  enum URIMode
  {
    /**
     * Automatically detected. Using this mode, an appropriate processing
     * mode will be selected when the URI is first processed.
     */
    Auto,
    /**
     * Invalid URI. This is something that can't be parsed as a URI at all.
     * The contents are accessible through the protocol() method.
     */
    Invalid,
    /**
     * Raw URI. This type of URI should not be processed in any way.
     * Contents are accessible through the path() method.
     */
    RawURI,
    /**
     * Standards compliant URL. Process as a syntactically correct URL.
     */
    URL,
    /**
     * Mailto URI. path() contains an email address which should have its
     * domain part processed as a DNS name. The email address is accessible
     * through the path() method.
     */
    Mailto
  };

  /**
   * KURL::List is a QValueList that contains KURLs with a few
   * convenience methods.
   * @see KURL
   * @see QValueList
   */
  class KDECORE_EXPORT List : public QValueList<KURL>
  {
  public:
    /**
     * Creates an empty List.
     */
      List() { }
      /**
       * @brief Creates a list that contains the given URL as only item
       *
       * @param url the URL to add
       */
      List(const KURL &url);
      /**
       * @brief Creates a list that contains the URLs from the given list
       *
       * This equivalent to iterating over the input list and using each item
       * as the argument to KURL's constructor, i.e. the resulting list will
       * have as many elements as the input list, but not all entries might
       * be valid.
       *
       * @param list the list containing the URLs as strings
       *
       * @see KURL(const QString &, int)
       */
      List(const QStringList &list);
      /**
       * @brief Converts the URLs of this list to a list of strings
       *
       * This is equivalent to iterating over the list and calling url() on
       * each item.
       * If you need a list of user visible URLs, i.e. not containing password
       * information, iterate over the list yourself and call prettyURL() on
       * each item instead.
       *
       * @return the list of strings
       *
       * @see KURL::url()
       */
      QStringList toStringList() const;
  };
  /**
   * @brief Constructs an empty URL
   *
   * The created instance will also be invalid, see isValid()
   */
  KURL();

  /**
   * @brief Destructs the KURL object
   */
  ~KURL();

  /**
   * @brief Usual constructor, to construct from a string
   *
   * @warning It is dangerous to feed UNIX filenames into this function,
   * this will work most of the time but not always.
   *
   * For example <tt>"/home/Torben%20Weis"</tt> will be considered a URL
   * pointing to the file <tt>"/home/Torben Weis"</tt> instead
   * of to the file <tt>"/home/Torben%20Weis"</tt>.
   *
   * This means that if you have a usual UNIX like path you should not use
   * this constructor. Instead use fromPathOrURL()
   *
   * @param url a URL, not a filename. If the URL does not have a protocol
   *            part, @c "file:" is assumed
   * @param encoding_hint MIB of original encoding of URL.
   *        See QTextCodec::mibEnum()
   *
   * @see fromPathOrURL()
   */
  KURL( const QString& url, int encoding_hint = 0 );
  /**
   * @brief Constructor taking an URL encoded in a C string
   *
   * Constructor taking a char * @p url, which is an @em encoded representation
   * of the URL, exactly like the usual constructor. This is useful when
   * the URL, in its encoded form, is strictly ASCII.
   *
   * @warning It is dangerous to feed UNIX filenames into this function,
   * this will work most of the time but not always.
   *
   * For example <tt>"/home/Torben%20Weis"</tt> will be considered a URL
   * pointing to the file <tt>"/home/Torben Weis"</tt> instead
   * of to the file <tt>"/home/Torben%20Weis"</tt>.
   *
   * This means that if you have a usual UNIX like path you should not use
   * this constructor. Instead use fromPathOrURL()
   *
   * @param url an encoded URL. If the URL does not have a protocol part,
   *            @c "file:" is assumed
   * @param encoding_hint MIB of original encoding of URL.
   *        See QTextCodec::mibEnum()
   *
   * @see fromPathOrURL()
   * @see QString::fromLatin1()
   */
  KURL( const char * url, int encoding_hint = 0 );
  /**
   * @brief Constructor taking an URL encoded in a QCString
   *
   * Constructor taking a QCString @p url, which is an @em encoded
   * representation of the URL, exactly like the usual constructor. This is
   * useful when the URL, in its encoded form, is strictly ASCII.
   *
   * @warning It is dangerous to feed UNIX filenames into this function,
   * this will work most of the time but not always.
   *
   * For example <tt>"/home/Torben%20Weis"</tt> will be considered a URL
   * pointing to the file <tt>"/home/Torben Weis"</tt> instead
   * of to the file <tt>"/home/Torben%20Weis"</tt>.
   *
   * This means that if you have a usual UNIX like path you should not use
   * this constructor. Instead use fromPathOrURL()
   *
   * @param url A encoded URL. If the URL does not have a protocol part,
   *            @c "file:" is assumed
   * @param encoding_hint MIB of original encoding of URL.
   *        See QTextCodec::mibEnum()
   *
   * @see fromPathOrURL()
   * @see QString::fromLatin1()
   */
  KURL( const QCString& url, int encoding_hint = 0 );

  /**
   * @brief Copy constructor
   *
   * @param u the KURL to copy
   */
  KURL( const KURL& u );
  /**
   * @brief Constructor taking a Qt URL
   *
   * Converts from a Qt URL.
   *
   * @param u the QUrl
   */
  KURL( const QUrl &u );
  /**
   * @brief Constructor allowing relative URLs
   *
   * @warning It is dangerous to feed UNIX filenames into this function,
   * this will work most of the time but not always.
   *
   * For example <tt>"/home/Torben%20Weis"</tt> will be considered a URL
   * pointing to the file <tt>"/home/Torben Weis"</tt> instead
   * of to the file <tt>"/home/Torben%20Weis"</tt>.
   *
   * This means that if you have a usual UNIX like path you should not use
   * this constructor. Instead use fromPathOrURL()
   *
   * @param _baseurl The base url.
   * @param _rel_url A relative or absolute URL.
   *        If this is an absolute URL then @p _baseurl will be ignored.
   *        If this is a relative URL it will be combined with @p _baseurl.
   *        Note that @p _rel_url should be encoded too, in any case.
   *        So do NOT pass a path here (use setPath() or addPath() or
   *        fromPathOrURL() instead)
   * @param encoding_hint MIB of original encoding of URL.
   *        See QTextCodec::mibEnum()
   *
   * @see fromPathOrURL()
   */
  KURL( const KURL& _baseurl, const QString& _rel_url, int encoding_hint=0 );

  /**
   * @brief Returns the protocol for the URL
   *
   * Examples for a protocol string are @c "file", @c "http", etc. but also
   * @c "mailto:" and other pseudo protocols.
   *
   * @return the protocol of the URL, does not include the colon. If the
   *         URL is malformed, @c QString::null will be returned
   *
   * @see setProtocol()
   * @see isValid()
   */
  QString protocol() const { return m_bIsMalformed ? QString::null : m_strProtocol; }
  /**
   * @brief Sets the protocol for the URL
   *
   * Examples for a protocol string are @c "file", @c "http", etc. but also
   * @c "mailto:" and other pseudo protocols.
   *
   * @param _txt the new protocol of the URL (without colon)
   *
   * @see protocol()
   */
  void setProtocol( const QString& _txt );

  /**
   * @brief Returns the URI processing mode for the URL
   *
   * @return the URI processing mode set for this URL
   *
   * @see URIMode
   * @see uriModeForProtocol()
   *
   * @since 3.2
   */
  int uriMode() const;

  /**
   * @brief Returns the decoded user name (login, user id, etc) included in
   *        the URL
   *
   * @return the user name or @c QString::null if there is no user name
   *
   * @see setUser()
   * @see hasUser()
   */
  QString user() const { return m_strUser; }
  /**
   * @brief Sets the user name (login, user id, etc) to include in the URL
   *
   * Special characters in the user name will appear encoded in the URL.
   * If there is a password associated with the user, it can be set using
   * setPass().
   *
   * @param _txt the name of the user or @c QString::null to remove the user
   *
   * @see user()
   * @see hasUser()
   * @see hasPass()
   */
  void setUser( const QString& _txt );
  /**
   * @brief Tests if this URL has a user name included in it
   *
   * @return @c true if the URL has an non-empty user name
   *
   * @see user()
   * @see setUser()
   * @see hasPass()
   */
  bool hasUser() const { return !m_strUser.isEmpty(); }

  /**
   * @brief Returns the decoded password (corresponding to user()) included
   *        in the URL
   *
   * @note a password can only appear in a URL string if you also set
   * a user, see setUser().
   *
   * @return the password or @c QString::null if it does not exist
   *
   * @see setPass()
   * @see hasPass()
   * @see hasUser()
   */
  QString pass() const { return m_strPass; }
  /**
   * @brief Sets the password (corresponding to user()) to include in the URL
   *
   * Special characters in the password will appear encoded in the URL.
   * @note a password can only appear in a URL string if you also set
   * a user, see setUser().
   *
   * @param _txt the password to set or @c QString::null to remove the password
   *
   * @see pass()
   * @see hasPass()
   * @see hasUser()
   */
  void setPass( const QString& _txt );
  /**
   * @brief Tests if this URL has a password included in it
   *
   * @note a password can only appear in a URL string if you also set
   * a user, see setUser().
   *
   * @return @c true if there is a non-empty password set
   *
   * @see pass()
   * @see setPass()
   * @see hasUser()
   */
  bool hasPass() const { return !m_strPass.isEmpty(); }

  /**
   * @brief Returns the decoded hostname included in the URL
   *
   * @return the name of the host or @c QString::null if no host is set
   *
   * @see setHost()
   * @see hasHost()
   */
  QString host() const { return m_strHost; }

  /**
   * @brief Sets the hostname to include in the URL
   *
   * Special characters in the hostname will appear encoded in the URL.
   *
   * @param _txt the new name of the host or QString::null to remove the host
   *
   * @see host()
   * @see hasHost()
   */
  void setHost( const QString& _txt );
  /**
   * @brief Tests if this URL has a hostname included in it
   *
   * @return @c true if the URL has a non-empty host
   *
   * @see host()
   * @see setHost()
   */
  bool hasHost() const { return !m_strHost.isEmpty(); }

  /**
   * @brief Returns the port number included in the URL
   *
   * @return the port number or @c 0 if there is no port number specified in
   *         the URL
   *
   * @see setPort()
   * @see host()
   */
  unsigned short int port() const { return m_iPort; }
  /**
   * @brief Sets the port number to include in the URL
   *
   * @param _p the new port number or @c 0 to have no port number
   *
   * @see port()
   * @see setHost()
   */
  void setPort( unsigned short int _p );

  /**
   * @brief Returns the current decoded path
   *
   * This does @em not include the query.
   *
   * @return the path of the URL (without query), or @c QString::null if no
   *         path is set
   *
   * @see path(int)
   * @see setPath()
   * @see hasPath()
   */
  QString path() const  { return m_strPath; }

  /**
   * @brief Returns the current decoded path
   *
   * This does @em not include the query, see query() for accessing it.
   *
   * The @p _trailing parameter allows to ensure the existance or absence of
   * the last (trailing) @c '/' character in the path.
   * If the URL has no path, then no @c '/' is added anyway.
   * And on the other side: if the path is just @c "/", then this character
   * won't be stripped.
   *
   * Reason: <tt>"ftp://weis@host"</tt> means something completely different
   * than <tt>"ftp://weis@host/"</tt>.
   * So adding or stripping the '/' would really alter the URL, while
   * <tt>"ftp://host/path"</tt> and <tt>"ftp://host/path/"</tt> mean the same
   * directory.
   *
   * @param _trailing May be ( @c -1, @c 0, @c +1 ). @c -1 strips a trailing
   *                  @c '/', @c +1 adds a trailing @c '/' if there is none yet
   *                  and @c 0 returns the path unchanged
   *
   * @return the path of the URL (without query), or @c QString::null if no
   *         path is set
   *
   * @see path()
   * @see setPath()
   * @see hasPath()
   * @see adjustPath()
   */
  QString path( int _trailing ) const;

  /**
   * @brief Sets the decoded path of the URL
   *
   * This does @em not changed the  query, see setQuery() for that.
   *
   * The @p path is considered to be decoded, i.e. characters not allowed in
   * path, for example @c '?' will be encoded and does not indicate the
   * beginning of the query part. Something that might look encoded,
   * like @c "%3f" will not become decoded.
   *
   * @param path the new, decoded, path or @c QString::null to remove the path
   *
   * @see path()
   * @see path(int)
   * @see hasPath()
   */
  void setPath( const QString& path );

  /**
   * @brief Tests if this URL has a path included in it
   *
   * @return @c true if there is a non-empty path
   *
   * @see path()
   * @see setPath()
   */
  bool hasPath() const { return !m_strPath.isEmpty(); }

  /**
   * @brief Resolves @c "." and @c ".." components in path
   *
   * Some servers seem not to like the removal of extra @c '/'
   * even though it is against the specification in RFC 2396.
   *
   * @param cleanDirSeparator if @c true, occurrences of consecutive
   *        directory separators (e.g. <tt>"/foo//bar"</tt>) are cleaned up as
   *        well
   *
   * @see hasPath()
   * @see adjustPath()
   */
  void cleanPath(bool cleanDirSeparator = true);

  /**
   * @brief Adds or removes a trailing slash to/from the path
   *
   * The @p _trailing parameter allows to ensure the existance or absence of
   * the last (trailing) @c '/' character in the path.
   * If the URL has no path, then no @c '/' is added anyway.
   * And on the other side: if the path is just @c "/", then this character
   * won't be stripped.
   *
   * Reason: <tt>"ftp://weis@host"</tt> means something completely different
   * than <tt>"ftp://weis@host/"</tt>.
   * So adding or stripping the '/' would really alter the URL, while
   * <tt>"ftp://host/path"</tt> and <tt>"ftp://host/path/"</tt> mean the same
   * directory.
   *
   * @param _trailing May be ( @c -1, @c 0, @c +1 ). @c -1 strips a trailing
   *                  @c '/', @c +1 adds a trailing @c '/' if there is none yet
   *                  and @c 0 returns the path unchanged
   *
   * @see hasPath()
   * @see cleanPath()
   */
  void adjustPath(int _trailing);

  /**
   * @brief Sets both path and query of the URL in their encoded form
   *
   * This is useful for HTTP. It looks first for @c '?' and decodes then,
   * see setEncodedPath().
   * The encoded path is the concatenation of the current path and the query.
   *
   * @param _txt the new encoded path and encoded query
   * @param encoding_hint MIB of original encoding of @p _txt .
   *        See QTextCodec::mibEnum()
   *
   * @see encodedPathAndQuery()
   * @see setPath()
   * @see setQuery()
   */
  void setEncodedPathAndQuery( const QString& _txt, int encoding_hint = 0 );

  /**
   * @brief Sets the (already encoded) path of the URL
   *
   * @param _txt the new encoded path
   * @param encoding_hint MIB of original encoding of @p _txt .
   *        See QTextCodec::mibEnum()
   *
   * @see setEncodedPathAndQuery()
   * @see setPath()
   */
  void setEncodedPath(const QString& _txt, int encoding_hint = 0 );

  /**
   * @brief Returns the encoded path and the query
   *
   * The @p _trailing parameter allows to ensure the existance or absence of
   * the last (trailing) @c '/' character in the path.
   * If the URL has no path, then no @c '/' is added anyway.
   * And on the other side: if the path is just @c "/", then this character
   * won't be stripped.
   *
   * Reason: <tt>"ftp://weis@host"</tt> means something completely different
   * than <tt>"ftp://weis@host/"</tt>.
   * So adding or stripping the '/' would really alter the URL, while
   * <tt>"ftp://host/path"</tt> and <tt>"ftp://host/path/"</tt> mean the same
   * directory.
   *
   * @param _trailing May be ( @c -1, @c 0, @c +1 ). @c -1 strips a trailing
   *                  @c '/', @c +1 adds a trailing @c '/' if there is none yet
   *                  and @c 0 returns the path unchanged
   * @param _no_empty_path if set to @c true then an empty path is substituted
   *        by @c "/"
   * @param encoding_hint MIB of desired encoding of URL.
   *        See QTextCodec::mibEnum()
   *
   * @return the concatenation of the encoded path , @c '?' and the
   *         encoded query
   *
   * @see setEncodedPathAndQuery()
   * @see path()
   * @see query()
   */
  QString encodedPathAndQuery( int _trailing = 0, bool _no_empty_path = false, int encoding_hint = 0) const;

  /**
   * @brief Sets the encoded query of the URL
   *
   * The query should start with a @c '?'. If it doesn't @c '?' is prepended.
   *
   * @param _txt this is considered to be encoded. This has a good reason:
   *             the query may contain the @c '0' character
   *
   * @param encoding_hint MIB of the encoding. Reserved, should be @c 0 .
   *        See QTextCodec::mibEnum()
   *
   * @see query()
   */
  void setQuery( const QString& _txt, int encoding_hint = 0);

  /**
   * @brief Returns the encoded query of the URL
   *
   * The query may contain the @c '0' character.
   * If a query is present it always starts with a @c '?'.
   * A single @c '?' means an empty query.
   * An empty string means no query.
   *
   * @return the encoded query or @c QString::null if there is none
   *
   * @see setQuery()
   */
  QString query() const;

  /**
   * @brief Returns the encoded reference of the URL
   *
   * The reference is @em never decoded automatically.
   *
   * @return the undecoded reference, or @c QString::null if there is none
   *
   * @see setRef()
   * @see hasRef()
   * @see htmlRef()
   */
  QString ref() const { return m_strRef_encoded; }

  /**
   * @brief Sets the encoded reference part (everything after @c '#')
   *
   * This is considered to be encoded, i.e. characters that are not allowed
   * as part of the reference will @em not be encoded.
   *
   * @param _txt the encoded reference or @c QString::null to remove it
   *
   * @see ref()
   * @see hasRef()
   */
  void setRef( const QString& _txt ) { m_strRef_encoded = _txt; }

  /**
   * @brief Tests if the URL has a reference part
   *
   * @return @c true if the URL has a reference part. In a URL like
   *         <tt>"http://www.kde.org/kdebase.tar#tar:/README"</tt> it would
   *         return @c true as well
   *
   * @see ref()
   * @see setRef()
   */
  bool hasRef() const { return !m_strRef_encoded.isNull(); }

  /**
   * @brief Returns decoded the HTML-style reference
   *        (the part of the URL after @c '#')
   *
   * @return the HTML-style reference
   *
   * @see encodedHtmlRef()
   * @see setHTMLRef()
   * @see hasHTMLRef()
   * @see split()
   * @see hasSubURL()
   * @see ref()
   */
  QString htmlRef() const;

  /**
   * @brief Returns the encoded HTML-style reference
   *        (the part of the URL after @c '#')
   *
   * @return the HTML-style reference in its original, encoded, form
   *
   * @see htmlRef()
   * @see setHTMLRef()
   * @see hasHTMLRef()
   */
  QString encodedHtmlRef() const;

  /**
   * @brief Sets the decoded HTML-style reference
   *
   * @param _ref the new reference. This is considered to be @em not encoded in
   *         contrast to setRef(). Use @c QString::null to remove it
   *
   * @see htmlRef()
   * @see hasHTMLRef()
   */
  void setHTMLRef( const QString& _ref );

  /**
   * @brief Tests if there is an HTML-style reference
   *
   * @return @c true if the URL has an HTML-style reference
   *
   * @see htmlRef()
   * @see encodedHtmlRef()
   * @see setHTMLRef()
   * @see hasRef()
   */
  bool hasHTMLRef() const;

  /**
   * @brief Tests if the URL is well formed
   *
   * @return @c false if the URL is malformed. This function does @em not test
   *         whether sub URLs are well-formed as well
   */
  bool isValid() const  { return !m_bIsMalformed; }
  /**
   * @brief Tests if the URL is malformed
   *
   * @return @c true if the URL is malformed. This function does @em not test
   *         whether sub URLs are well-formed as well
   *
   * @deprecated Use !isValid() instead
   *
   * @see isValid()
   */
  KDE_DEPRECATED bool isMalformed() const { return !isValid(); }

  /**
   * @brief Tests if the file is local
   *
   * @return @c true if the file is a plain local file and has no filter
   *         protocols attached to it
   */
  bool isLocalFile() const;

  /**
   * @brief Adds file encoding information
   *
   * Adds encoding information to the URL by adding a @c "charset" parameter.
   * If there is already a charset parameter, it will be replaced.
   *
   * @param encoding the encoding to add or @c QString::null to remove the
   *                 encoding
   *
   * @see fileEncoding()
   * @see QTextCodec::codecForName()
   */
  void setFileEncoding(const QString &encoding);

  /**
   * @brief Returns encoding information of the URL
   *
   * The encoding information is the content of the @c "charset" parameter.
   *
   * @return an encoding suitable for QTextCodec::codecForName()
   *         or @c QString::null if not encoding was specified
   */
  QString fileEncoding() const;

  /**
   * @brief Tests if the URL has any sub URLs
   *
   * See split() for examples for sub URLs.
   *
   * @return @c true if the file has at least one sub URL
   *
   * @see split()
   */
  bool hasSubURL() const;

  /**
   * @brief Adds to the current path
   *
   * Assumes that the current path is a directory. @p _txt is appended to the
   * current path. The function adds @c '/' if needed while concatenating.
   * This means it does not matter whether the current path has a trailing
   * @c '/' or not. If there is none, it becomes appended. If @p _txt
   * has a leading @c '/' then this one is stripped.
   *
   * @param txt the text to add. It is considered to be decoded
   *
   * @see setPath()
   * @see hasPath()
   */
  void addPath( const QString& txt );

  /**
   * @brief Returns the value of a certain query item
   *
   * @param item item whose value we want
   *
   * @return the value of the given query item name or @c QString::null if the
   *         specified item does not exist
   *
   * @see addQueryItem()
   * @see removeQueryItem()
   * @see queryItems()
   * @see query()
   */
  QString queryItem( const QString& item ) const;

  /**
   * @brief Returns the value of a certain query item
   *
   * @param item item whose value we want
   * @param encoding_hint MIB of encoding of query.
   *        See QTextCodec::mibEnum()
   *
   * @return the value of the given query item name or @c QString::null if the
   *         specified item does not exist
   *
   * @see addQueryItem()
   * @see removeQueryItem()
   * @see queryItems()
   * @see query()
   */
  QString queryItem( const QString& item, int encoding_hint ) const;

  /**
   * Options for queryItems()
   *
   * @since 3.1
   */
  enum QueryItemsOptions
  {
    /**
     * Normalize query keys to lowercase
     */
    CaseInsensitiveKeys = 1
  };

  /**
   * @internal, override for the below function
   */
  QMap< QString, QString > queryItems( int options=0 ) const;

  /**
   * @brief Returns the list of query items as a map mapping keys to values
   *
   * @param options any of QueryItemsOptions <em>OR</em>ed together
   * @param encoding_hint MIB of encoding of query.
   *        See QTextCodec::mibEnum()
   *
   * @return the map of query items or the empty map if the URL has no
   *         query items
   *
   * @see queryItem()
   * @see addQueryItem()
   * @see removeQueryItem()
   * @see query()
   *
   * @since 3.1
   */
  QMap< QString, QString > queryItems( int options, int encoding_hint ) const;

  /**
   * @brief Adds an additional query item
   *
   * To replace an existing query item, the item should first be
   * removed with removeQueryItem()
   *
   * @param _item name of item to add
   * @param _value value of item to add
   * @param encoding_hint MIB of encoding to use for _value.
   *        See QTextCodec::mibEnum()
   *
   * @see queryItem()
   * @see queryItems()
   * @see query()
   */
  void addQueryItem( const QString& _item, const QString& _value, int encoding_hint = 0 );

  /**
   * @brief Removea an item from the query
   *
   * @param _item name of item to remove
   *
   * @see addQueryItem()
   * @see queryItem()
   * @see queryItems()
   * @see query()
   */
  void removeQueryItem( const QString& _item );

  /**
   * @brief Sets the filename of the path
   *
   * In comparison to addPath() this function does not assume that the current
   * path is a directory. This is only assumed if the current path ends
   * with @c '/'.
   *
   * If the current path ends with @c '/' then @p _txt is just appended,
   * otherwise all text behind the last @c '/' in the current path is erased
   * and @p _txt is appended then. It does not matter whether @p _txt starts
   * with @c '/' or not.
   *
   * Any reference is reset.
   *
   * @param _txt the filename to be set. It is considered to be decoded
   *
   * @see fileName()
   * @see setDirectory()
   * @see setPath()
   */
  void setFileName( const QString&_txt );

  /**
   * @brief Returns the filename of the path
   *
   * @p _ignore_trailing_slash_in_path tells whether a trailing @c '/' should
   * be ignored. This means that the function would return @c "torben" for
   * <tt>"file:///hallo/torben/"</tt> and <tt>"file:///hallo/torben"</tt>.
   *
   * @param _ignore_trailing_slash_in_path if set to @c false, then everything
   *        behind the last @c '/' is considered to be the filename
   *
   * @return the filename of the current path. The returned string is decoded.
   *         @c QString::null if there is no file (and thus no path)
   *
   * @see setFileName()
   * @see directory()
   * @see path()
   */
  QString fileName( bool _ignore_trailing_slash_in_path = true ) const;

  /**
   * @brief Returns the directory of the path
   *
   * The directory is everything between the last and the second last @c '/'
   * is returned. For example <tt>"file:///hallo/torben/"</tt> would return
   * <tt>"/hallo/torben/"</tt> while <tt>"file:///hallo/torben"</tt> would
   * return <tt>"hallo/"</tt>.
   *
   * @p _ignore_trailing_slash_in_path tells whether a trailing @c '/' should
   * be ignored. This means that the function would return @c "/hallo"
   * (or @c "/hallo" depending on @p _strip_trailing_slash_from_result) for
   * <tt>"file:///hallo/torben/"</tt> and <tt>"file:///hallo/torben"</tt>.
   *
   * @param _strip_trailing_slash_from_result tells whether the returned result
   *        should end with @c '/' or not. If the path is empty or just @c "/"
   *        then this flag has no effect
   * @param _ignore_trailing_slash_in_path if set to @c false, then everything
   *        behind the last @c '/' is considered to be the filename
   *
   * @return the directory part of the current path or @c QString::null when
   *         there is no path. The returned string is decoded
   *
   * @see setDirectory()
   * @see fileName()
   * @see path()
   */
  QString directory( bool _strip_trailing_slash_from_result = true,
		     bool _ignore_trailing_slash_in_path = true ) const;

  /**
   * @brief Sets the directory of the path, leaving the filename empty
   *
   * @param dir the decoded directory to set
   *
   * @see directory()
   * @see setFileName()
   * @see setPath()
   */
  void setDirectory(const QString &dir);

  /**
   * @brief Changes the directory by descending into the given directory
   *
   * It is assumed the current URL represents a directory.
   * If @p _dir starts with a @c '/' the current URL will be
   * <tt>"protocol://host/dir"</tt> otherwise @p _dir will be appended to the
   * path. @p _dir can be @c ".."
   *
   * This function won't strip protocols. That means that when you are in
   * <tt>"file:///dir/dir2/my.tgz#tar:/"</tt> and you do <tt>cd("..")</tt> you
   * will still be in <tt>"file:///dir/dir2/my.tgz#tar:/"</tt>
   *
   * @param _dir the directory to change to
   * @return @c true if successful
   *
   * @see directory()
   * @see path()
   */
  bool cd( const QString& _dir );

  /**
   * @brief Returns the URL as string, with all escape sequences intact,
   *        encoded in a given charset
   *
   * This is used in particular for encoding URLs in UTF-8 before using them
   * in a drag and drop operation.
   *
   * @note that the string returned by url() will include the password of the
   * URL. If you want to show the URL to the user, use prettyURL().
   *
   * The @p _trailing parameter allows to ensure the existance or absence of
   * the last (trailing) @c '/' character in the path.
   * If the URL has no path, then no @c '/' is added anyway.
   * And on the other side: if the path is just @c "/", then this character
   * won't be stripped.
   *
   * Reason: <tt>"ftp://weis@host"</tt> means something completely different
   * than <tt>"ftp://weis@host/"</tt>.
   * So adding or stripping the '/' would really alter the URL, while
   * <tt>"ftp://host/path"</tt> and <tt>"ftp://host/path/"</tt> mean the same
   * directory.
   *
   * @param _trailing May be ( @c -1, @c 0, @c +1 ). @c -1 strips a trailing
   *                  @c '/', @c +1 adds a trailing @c '/' if there is none yet
   *                  and @c 0 returns the path unchanged
   * @param encoding_hint MIB of encoding to use.
   *        See QTextCodec::mibEnum()
   *
   * @return the complete URL, with all escape sequences intact, encoded
   *         in a given charset
   *
   * @see prettyURL()
   * @see pathOrURL()
   * @see htmlURL()
   */
  QString url( int _trailing = 0, int encoding_hint = 0) const;

  /**
   * @brief Returns the URL as string in human-friendly format
   *
   * Example:
   * @code
   * http://localhost:8080/test.cgi?test=hello world&name=fred
   * @endcode
   *
   * Does @em not contain the password if the URL has one, use url() if you
   * need to have it in the string.
   *
   * The @p _trailing parameter allows to ensure the existance or absence of
   * the last (trailing) @c '/' character in the path.
   * If the URL has no path, then no @c '/' is added anyway.
   * And on the other side: if the path is just @c "/", then this character
   * won't be stripped.
   *
   * Reason: <tt>"ftp://weis@host"</tt> means something completely different
   * than <tt>"ftp://weis@host/"</tt>.
   * So adding or stripping the '/' would really alter the URL, while
   * <tt>"ftp://host/path"</tt> and <tt>"ftp://host/path/"</tt> mean the same
   * directory.
   *
   * @param _trailing May be ( @c -1, @c 0, @c +1 ). @c -1 strips a trailing
   *                  @c '/', @c +1 adds a trailing @c '/' if there is none yet
   *                  and @c 0 returns the path unchanged
   * @return a human readable URL, with no non-necessary encodings/escaped
   *         characters. Password will not be shown
   *
   * @see url()
   * @see pathOrURL()
   */
  QString prettyURL( int _trailing = 0) const;

  /**
   * @brief Returns the URL as string in human-friendly format
   * Example:
   * @code
   * http://localhost:8080/test.cgi?test=hello world&name=fred
   * @endcode
   *
   * Does @em not contain the password if the URL has one, use url() if you
   * need to have it in the string.
   *
   * The @p _trailing parameter allows to ensure the existance or absence of
   * the last (trailing) @c '/' character in the path.
   * If the URL has no path, then no @c '/' is added anyway.
   * And on the other side: if the path is just @c "/", then this character
   * won't be stripped.
   *
   * Reason: <tt>"ftp://weis@host"</tt> means something completely different
   * than <tt>"ftp://weis@host/"</tt>.
   * So adding or stripping the '/' would really alter the URL, while
   * <tt>"ftp://host/path"</tt> and <tt>"ftp://host/path/"</tt> mean the same
   * directory.
   *
   * @param _trailing May be ( @c -1, @c 0, @c +1 ). @c -1 strips a trailing
   *                  @c '/', @c +1 adds a trailing @c '/' if there is none yet
   *                  and @c 0 returns the path unchanged
   * @param _flags if StripFileProtocol, @c "file://" will be stripped.
   *        The use of this method is now discouraged, better use pathOrURL().
   *
   * @return a human readable URL, with no non-necessary encodings/escaped
   *         characters. Password will not be shown
   *
   * @see prettyURL()
   * @see url()
   * @see pathOrURL()
   */
  QString prettyURL( int _trailing, AdjustementFlags _flags) const;
  // ### BIC: Merge the two above + spell it as "Adjustment"
  // Or remove completely, and let people use pathOrURL() instead

  /**
   * @brief Returns the URL as a string depending if it is a local file
   *
   * It will be either the URL (as prettyURL() would return) or, when the URL
   * is a local file without query or ref, the path().
   *
   * Use this method, together with its opposite, fromPathOrURL(),
   * to display and even let the user edit URLs.
   *
   * @return the path or URL string depending on its properties
   *
   * @see prettyURL()
   * @see path()
   * @see url()
   * @see isLocalFile()
   *
   * @since 3.4
   */
  QString pathOrURL() const;

  /**
   * @brief Returns the URL as string, escaped for HTML
   *
   * @return a human readable URL, with no non-necessary encodings/escaped
   *         characters which is HTML encoded for safe inclusion in HTML or
   *         rich text. Password will not be shown.
   *
   * @see prettyURL()
   * @see url()
   * @see pathOrURL()
   */
  QString htmlURL() const;


  /**
   * @brief Tests if the KURL is empty
   *
   * An empty URL has neither path nor protocol set.
   *
   * @return @c true if the URL is empty
   *
   * @see hasPath()
   * @see protocol()
   * @see isValid()
   */
  bool isEmpty() const;

  /**
   * @brief Returns the URL that is the best possible candidate for on level
   *        higher in the path hierachy
   *
   * This function is useful to implement the "Up" button in a file manager for
   * example.
   * cd() never strips a sub-protocol. That means that if you are in
   * <tt>"file:///home/x.tgz#gzip:/#tar:/"</tt> and hit the up button you
   * expect to see <tt>"file:///home"</tt>. The algorithm tries to go up on the
   * right-most URL. If that is not possible it strips the right most URL. It
   * continues stripping URLs until it can go up.
   *
   * @return a URL that is a level higher
   *
   * @see cd()
   * @see split()
   * @see hasSubURL()
   * @see path()
   */
  KURL upURL( ) const;

  /**
   * @brief Tests if this URL is less than the given URL
   *
   * The current URL is consideres <tt>"less than"</tt> then @p _u if
   * (tested in this order):
   * - it is not valid but @p _u is. See isValid()
   * - its protocol is "less than" @p _u's protocol. See protocol()
   * - its host is "less than" @p _u's host. See host()
   * - its port is "less than" @p _u's port. See port()
   * - its path is "less than" @p _u's path. See path()
   * - its encoded query is "less than" @p _u's encoded query. See query()
   * - its endoded reference is "less than" @p _u's encoded reference.
   *   See ref()
   * - its username is "less than" @p _u's username. See user()
   * - its password is "less than" @p _u's password. See pass()
   *
   * Examples:
   * @code
   * KURL url1;
   * KURL url2;
   *
   * bool lessThan = url1 < url2; // false. Both invalid, no protocols
   *
   * url2.setProtocol( QString::null );
   * lessThan = url1 < url2;            // true. url2 is valid because of setProtocol()
   *
   * url1.setProtocol( QString::null );
   * lessThan = url1 < url2;            // false. Both valid and everything empty
   *
   * url1.setProtocol( "http" );
   * url2.setProtocol( "https" );
   * lessThan = url1 < url2;            // true. "http" < "https"
   *
   * url2.setHost( "api.kde.org" );
   * url2.setProtocol( "http" );
   * url2.setProtocol( "www.kde.org" );
   * lessThan = url1 < url2;            // true. protocols equal and "api" < "www"
   *
   * url1.setProtocol( "https" );
   * url2.setProtocol( "http" );
   * lessThan = url1 < url2;            // false. "https" > "http". host doesn't matter yet
   * @endcode
   *
   * @param _u the URL to compare to
   *
   * @return @c true if the URL is less than @p _u. Otherwise @c false
   *         (equal or greater than)
   *
   * @see operator==()
   * @see QString::compare()
   */
  bool operator<(const KURL& _u) const;

  /**
   * @brief Copies the values of the given URL into this one
   *
   * Just assigns each member using the member's assignment operator.
   *
   * @param _u the URL to take the values from
   *
   * @return a reference to this URL (*this)
   *
   * @see equals()
   */
  KURL& operator=( const KURL& _u );

  /**
   * @brief Assigns the URL, given as a string, to this one
   *
   * This will reset the current URL and parse the given string.
   * See the similar constructor for known limitations.
   *
   * @param _url the QString to parse for values
   *
   * @return a reference to this URL (*this)
   *
   * @see equals()
   * @see KURL(const QString &, int)
   */
  KURL& operator=( const QString& _url );

  /**
   * @brief Assigns the URL, given as a C string, to this one
   *
   * This will reset the current URL and parse the given string.
   * See the similar constructor for known limitations.
   *
   * @param _url the C string to parse for values
   *
   * @return a reference to this URL (*this)
   *
   * @see equals()
   * @see KURL(const char *, int)
   */
  KURL& operator=( const char * _url );

  /**
   * @brief Assigns the URL, given as a Qt URL, to this one
   *
   * This will reset the current URL and parse the given string.
   *
   * @param u the Qt URL to take the values from
   *
   * @return a reference to this URL (*this)
   *
   * @see equals()
   * @see KURL(const QUrl &)
   */
  KURL& operator=( const QUrl & u );

  /**
   * @brief Tests if this URL is equal to the given one
   *
   * Tests each member for equality unless one of the URLs is invalid
   * in which case they are not considered equal (even if both are invalid).
   *
   * Same as equals() when used with @p ignore_trailing set to
   * @c false (default)
   *
   * @param _u the URL to compare to
   *
   * @return @c true if equal and neither this URL nor @p _u is malformed.
   *         Otherwise @c false
   *
   * @see equals()
   * @see isValid()
   * @see operator!=()
   * @see operator<()
   */
  bool operator==( const KURL& _u ) const;

  /**
   * @brief Tests if this URL is equal to the one given as a string
   *
   * Creates a KURL instance for @p _u and compares with that using
   * the equality operator for two KURLs.
   *
   * See the respective constructor for known limitations.
   *
   * @param _u the string to compare to
   *
   * @return @c true if equal and neither this URL nor @p _u is malformed.
   *         Otherwise @c false
   *
   * @see KURL(const QString &, int)
   * @see operator==(const KURL &)
   * @see equals()
   * @see isValid()
   * @see operator!=()
   * @see operator<()
   */
  bool operator==( const QString& _u ) const;

  /**
   * @brief Tests if this URL is different from the given one
   *
   * Tests by negating the result of operator==()
   *
   * @param _u the URL to compare to
   *
   * @return the negated result of operator==()
   *
   * @see operator==()
   * @see operator<()
   */
  bool operator!=( const KURL& _u ) const { return !( *this == _u ); }

  /**
   * @brief Tests if this URL is different from the one given as a string
   *
   * Tests by negating the result of operator==(const QString &)
   *
   * @param _u the URL to compare to
   *
   * @return the negated result of operator==(const QString &)
   *
   * @see operator==(const QString &)
   * @see operator<()
   */
  bool operator!=( const QString& _u ) const { return !( *this == _u ); }

  /**
   * @brief Compares this URL with another one
   *
   * The same as equals(), just with a less obvious name.
   *
   * @param u the URL to compare this one with
   * @param ignore_trailing set to @c true to ignore trailing @c '/' characters
   *
   * @return @c true if both URLs are the same
   *
   * @see operator==. This function should be used if you want to
   *      ignore trailing @c '/' characters
   *
   * @deprecated Use equals() instead.
   */
  bool cmp( const KURL &u, bool ignore_trailing = false ) const KDE_DEPRECATED;

  /**
   * @brief Compares this URL with another one
   *
   * @param u the URL to compare this one with
   * @param ignore_trailing set to @c true to ignore trailing @c '/' characters
   *
   * @return @c true if both urls are the same
   *
   * @see operator==. This function should be used if you want to
   *      ignore trailing @c '/' characters
   *
   * @since 3.1
   */
  bool equals( const KURL &u, bool ignore_trailing = false ) const; // TODO KDE4: add bool _ignore_ref = false

  /**
   * @brief Tests if the given URL is parent of this URL
   *
   * For instance, <tt>"ftp://host/dir/"</tt> is a parent of
   * <tt>"ftp://host/dir/subdir/subsubdir/"</tt>.
   *
   * @return @c true if this URL is a parent of @p u (or the same URL as @p u)
   *
   * @see equals()
   * @see cd()
   */
  bool isParentOf( const KURL& u ) const;

  /**
   * @brief Splits nested URLs into a list of URLs
   *
   * Example for a nested URL:
   * @code
   * file:///home/weis/kde.tgz#gzip:/#tar:/kdebase
   * @endcode
   * A URL like <tt>"http://www.kde.org#tar:/kde/README.hml#ref1"</tt> will be
   * split in <tt>"http://www.kde.org#ref1"</tt> and
   * <tt>"tar:/kde/README.html#ref1"</tt>.
   *
   * That means in turn that @c "#ref1" is an HTML-style reference and not a
   * new sub URL. Since HTML-style references mark a certain position in a
   * document this reference is appended to every URL.
   *
   * The idea behind this is that browsers, for example, only look at the first
   * URL while the rest is not of interest to them.
   *
   * @param _url the URL that has to be split
   *
   * @return an empty list on error or the list of split URLs
   *
   * @see hasSubURL()
   * @see KURL(const QString&, int)
   * @see join()
   */
  static List split( const QString& _url );

  /**
   * @brief Splits nested URLs into a list of URLs
   *
   * Example for a nested URL:
   * @code
   * file:///home/weis/kde.tgz#gzip:/#tar:/kdebase
   * @endcode
   * A URL like <tt>"http://www.kde.org#tar:/kde/README.hml#ref1"</tt> will be
   * split in <tt>"http://www.kde.org#ref1"</tt> and
   * <tt>"tar:/kde/README.html#ref1"</tt>.
   *
   * That means in turn that @c "#ref1" is an HTML-style reference and not a
   * new sub URL. Since HTML-style references mark a certain position in a
   * document this reference is appended to every URL.
   *
   * The idea behind this is that browsers, for example, only look at the first
   * URL while the rest is not of interest to them.
   *
   * @param _url the URL that has to be split
   *
   * @return an empty list on error or the list of split URLs
   *
   * @see hasSubURL()
   * @see join()
   */
  static List split( const KURL& _url );

  /**
   * @brief Joins a list of URLs into a single URL with sub URLs
   *
   * Reverses split(). Only the first URL may have a reference. This reference
   * is considered to be HTML-like and is appended at the end of the resulting
   * joined URL.
   *
   * @param _list the list to join
   *
   * @return the joined URL or an invalid URL if the list is empty
   *
   * @see split()
   */
  static KURL join( const List& _list );

  /**
   * @brief Creates a KURL object from a QString representing either an
   *        absolute path or a real URL
   *
   * Use this method instead of
   * @code
   * QString someDir = ...
   * KURL url = someDir;
   * @endcode
   *
   * Otherwise some characters (e.g. the '#') won't be encoded properly.
   *
   * @param text the string representation of the URL to convert
   *
   * @return the new KURL
   *
   * @see pathOrURL()
   * @see KURL(const QString&, int)
   *
   * @since 3.1
   */
  static KURL fromPathOrURL( const QString& text );

  /**
   * @brief Encodes a string for use in URLs
   *
   * Convenience function.
   *
   * Convert unicoded string to local encoding and use %%-style
   * encoding for all common delimiters / non-ascii characters.
   *
   * @param str the string to encode (can be @c QString::null)
   * @param encoding_hint MIB of encoding to use.
   *        See QTextCodec::mibEnum()
   *
   * @return the encoded string
   *
   * @see encode_string_no_slash()
   * @see decode_string()
   */
  static QString encode_string(const QString &str, int encoding_hint = 0);

  /**
   * @brief Encodes a string for use in URLs
   *
   * Convenience function.
   *
   * Convert unicoded string to local encoding and use %%-style
   * encoding for all common delimiters and non-ascii characters
   * as well as the slash @c '/'.
   *
   * @param str the string to encode (can be @c QString::null)
   * @param encoding_hint MIB of encoding to use.
   *        See QTextCodec::mibEnum()
   *
   * @see encode_string()
   * @see decode_string()
   */
  static QString encode_string_no_slash(const QString &str, int encoding_hint = 0);

  /**
   * @brief Decodes a string as used in URLs
   *
   * Convenience function.
   *
   * Decode %-style encoding and convert from local encoding to unicode.
   *
   * Reverse of encode_string()
   *
   * @param str the string to decode (can be @c QString::null)
   * @param encoding_hint MIB of original encoding of @p str .
   *        See QTextCodec::mibEnum()
   *
   * @return the decoded string
   *
   * @see encode_string()
   * @see encode_string_no_slash()
   */
  static QString decode_string(const QString &str, int encoding_hint = 0);

  /**
   * @brief Tests if a given URL is a relative as opposed to an absolute URL
   *
   * Convenience function.
   *
   * Returns whether @p _url is likely to be a "relative" URL instead of
   * an "absolute" URL.
   *
   * @param _url the URL to examine
   * @return @c true when the URL is likely to be "relative",
   *         @c false otherwise
   *
   * @see relativeURL()
   */
  static bool isRelativeURL(const QString &_url);

  /**
   * @brief Creates an URL relative to a base URL for a given input URL
   *
   * Convenience function
   *
   * Returns a "relative URL" based on @p base_url that points to @p url.
   *
   * If no "relative URL" can be created, e.g. because the protocol
   * and/or hostname differ between @p base_url and @p url an absolute
   * URL is returned.
   *
   * @note if @p base_url represents a directory, it should contain
   *       a trailing slash
   *
   * @param base_url the URL to derive from
   * @param url the URL to point to relatively from @p base_url
   * @param encoding_hint MIB of original encoding of @p str .
   *        See QTextCodec::mibEnum()
   *
   * @see isRelativeURL()
   * @see relativePath()
   * @see adjustPath()
   */
  static QString relativeURL(const KURL &base_url, const KURL &url, int encoding_hint = 0);

  /**
   * @brief Creates a path relative to a base path for a given input path
   *
   * Convenience function
   *
   * Returns a relative path based on @p base_dir that points to @p path.
   *
   * @param base_dir the base directory to derive from
   * @param path the new target directory
   * @param isParent an optional pointer to a boolean which, if provided, will
   *        be set to reflect whether @p path has @p base_dir as a parent dir
   *
   * @see relativeURL()
   */
  static QString relativePath(const QString &base_dir, const QString &path, bool *isParent=0);

  /**
   * @brief Determines which URI mode is suitable for processing URIs of a
   *        given protocol
   *
   * @param protocol the protocol name. See protocol()
   *
   * @return the URIMode suitable for the given protocol
   *
   * @see uriMode()
   *
   * @since 3.2
   */
  static URIMode uriModeForProtocol(const QString& protocol);

#ifdef KDE_NO_COMPAT
private:
#endif
  /**
   * @deprecated change code to call fileName()
   */
	QString filename( bool _ignore_trailing_slash_in_path = true ) const
  {
    return fileName(_ignore_trailing_slash_in_path);
  }

protected:
  /**
   * @brief Resets the members to their "null" state
   *
   * All QString members get reset to @c QString::null, the port to @c 0
   * the URIMode to @c Auto and the URL becomes invalid.
   *
   * This is like assigning a null URL, but more efficient as it doesn't
   * require the temporary object.
   *
   * Called by constructors, assignment operators and the parse methods in case
   * of a parsing error.
   *
   * @see isValid()
   * @see isEmpty()
   */
  void reset();
  
  /**
   * @brief Parses the given string and fills the URL's values on success
   *
   * Treats the string as an URL.
   *
   * @param _url the string to parse
   * @param encoding_hint MIB of original encoding of @p str .
   *        See QTextCodec::mibEnum()
   */
  void parseURL( const QString& _url, int encoding_hint = 0 );
  /**
   * @brief Parses the given string and fills the URL's values on success
   *
   * Treats the string as a generic URI.
   *
   * @param _url the string to parse
   * @param encoding_hint MIB of original encoding of @p str .
   *        See QTextCodec::mibEnum()
   */
  void parseRawURI( const QString& _url, int encoding_hint = 0 );
  /**
   * @brief Parses the given string and fills the URL's values on success
   *
   * Treats the string as a @c "mailto:" URI.
   *
   * @param _url the string to parse
   * @param encoding_hint MIB of original encoding of @p str .
   *        See QTextCodec::mibEnum()
   */
  void parseMailto( const QString& _url, int encoding_hint = 0 );
  /**
   * @brief Parses the given string and fills the URL's values on success
   *
   * @param _url the string to parse
   * @param encoding_hint MIB of original encoding of @p str .
   *        See QTextCodec::mibEnum()
   */
  void parse( const QString& _url, int encoding_hint = 0 );

private:
  void _setQuery( const QString& _txt, int encoding_hint = 0);

  QString m_strProtocol;
  QString m_strUser;
  QString m_strPass;
  QString m_strHost;
  QString m_strPath;
  QString m_strRef_encoded;
  QString m_strQuery_encoded;
  bool m_bIsMalformed : 1;
  enum URIMode m_iUriMode : 3;
  uint freeForUse     : 4;
  unsigned short int m_iPort;
  QString m_strPath_encoded;

  friend KDECORE_EXPORT QDataStream & operator<< (QDataStream & s, const KURL & a);
  friend KDECORE_EXPORT QDataStream & operator>> (QDataStream & s, KURL & a);
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
KDECORE_EXPORT bool urlcmp( const QString& _url1, const QString& _url2, bool _ignore_trailing, bool _ignore_ref );

KDECORE_EXPORT QDataStream & operator<< (QDataStream & s, const KURL & a);
KDECORE_EXPORT QDataStream & operator>> (QDataStream & s, KURL & a);

#endif
