/* This file is part of the KDE libraries
    Copyright (C) 1999 Torben Weis <weis@kde.org>
 
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
 
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __kurl_h__
#define __kurl_h__

#include <qstring.h>
#include <qvaluelist.h>

/**
 * Mention that KURL has some restrictions regarding the path 
 * encoding. KURL works intern with the decoded path and
 * and encoded query. For example in
 * <pre>
 * http://localhost/cgi-bin/test%20me.pl?cmd=Hello%20you
 * </pre>
 * would result in a decoded path "/cgi-bin/test me.pl"
 * and in the encoded query "cmd=Hello%20you".
 * Since path is internally always encoded you may NOT use
 * "%00" in tha path while this is ok for the query.
 */
class KURL
{
  typedef QValueList<KURL> List;
public:
  /**
   * @param _url is considered to be encoded. You can pass strings like
   *             "/home/weis", in this case the protocol "file" is assumed.
   *             This is dangerous since even this simple path is assumed to be
   *             encoded. For example "/home/Torben%20Weis" will be decoded to
   *             "/home/Torben Weis". This means: If you have a usual UNIX like
   *             path, you have to use @ref encode first before you pass it to KURL.
   */
  KURL();
  KURL( const QString& _url );
  KURL( const KURL& _u );
  /**
   * @param _rel_url is considered to be encoded.
   */
  KURL( const KURL& _u, const QString& _rel_url );
  
  QString protocol() const { return m_strProtocol; }
  void setProtocol( const QString& _txt ) { m_strProtocol = _txt; }

  QString user() const { return m_strUser; }
  void setUser( const QString& _txt ) { m_strUser = _txt; }
  bool hasUser() const { return !m_strUser.isEmpty(); }

  QString pass() const { return m_strPass; }
  void setPass( const QString& _txt ) { m_strPass = _txt; }
  bool hasPass() const { return !m_strPass.isEmpty(); }

  QString host() const { return m_strHost; }
  void setHost( const QString& _txt ) { m_strHost = _txt; }
  bool hasHost() const { return !m_strHost.isEmpty(); }

  int port() const { return m_iPort; }
  void setPort( int _p ) { m_iPort = _p; }
  
  /**
   * @return the current decoded path. This does NOT include the query.
   *
   */
  QString path() const  { return m_strPath; }
  /**
   * @param _trailing may be ( -1, 0 +1 ). -1 strips a trailing '/', +1 adds
   *                  a trailing '/' if there is none yet and 0 returns the
   *                  path unchanged. If the URL has no path, then no '/' is added
   *                  anyway. And on the other side: if the path is "/", then this
   *                  character won't be stripped. Reason: "ftp://weis@host" means something
   *                  completely different than "ftp://weis@host/". So adding or stripping
   *                  the '/' would really alter the URL, while "ftp://host/path" and
   *                  "ftp://host/path/" mean the same directory.
   *
   * @return the current decoded path. This does NOT include the query.
   */
  QString path( int _trailing ) const;
  /**
   * _txt is considered to be decoded. This means: %3f does not become decoded
   *      and the ? does not indicate the start of the query part.
   *      The query is not changed by this function.
   */
  void setPath( const QString& _txt ) { m_strPath = _txt; }  
  bool hasPath() const { return !m_strPath.isEmpty(); }

  /**
   * This is useful for HTTP. It looks first for '?' and decodes then.
   * The encoded path is the concatenation of the current path and the query.
   */
  void setEncodedPathAndQuery( const QString& _txt );
  /**
   * @return the concatenation if the encoded path , '?' and the encoded query.
   *
   * @param _no_empty_path If set to true then an empty path is substituted by "/".
   */
  QString encodedPathAndQuery( int _trailing = 0, bool _no_empty_path = FALSE );

  /**
   * @param _txt is considered to be encoded. This has a good reason:
   * The query may contain the 0 character.
   */
  void setQuery( const QString& _txt ) { m_strQuery_encoded = _txt; }
  /**
   * @return the encoded query. This has a good reason: The query may contain the 0 character.
   */
  QString query() const { return m_strQuery_encoded; }
  
  /**
   * The reference is NEVER decoded automatically.
   */
  QString ref() const { return m_strRef_encoded; }
  /**
   * Set the reference part (everything after '#')
   * @param _txt is considered encoded.
   */
  void setRef( const QString& _txt ) { m_strRef_encoded = _txt; }
  /**
   * @return TRUE of the reference part of the URL is not empty. In an URL like
   *         tar:/kde/README#http://www.kde.org/kdebase.tgz it would return TRUE, too.
   */
  bool hasRef() const { return !m_strRef_encoded.isEmpty(); }
  
  /**
   * @return the HTML style reference. The HTML style reference can only be the
   *         last of all references. For example in tar:/#gzip:/decompress#file:/home/x.tgz#ref1
   *         the return value would be ref because it is the last reference and follows
   *         a source protocol. In contrast tar:/#gzip:/decompress#file:/home/x.tgz has no
   *         HTML style reference at all since file:/home/x.tgz is a sub URL to the filter
   *         protocol gzip. The returned string is, in contrast to @ref #ref already decoded.
   */
  QString htmlRef() const;
  /**
   * Sets the HTML style reference.
   *
   * @param _ref is considered to be NOT encoded in contrast to @ref #setRef
   *
   * @see #htmlRef
   */
  void setHTMLRef( const QString& _ref );
  /**
   * @return true if the URL has a HTML style reference.
   *
   * @see #htmlRef
   */
  bool hasHTMLRef() const;

  /**
   * @return true if the url is malformed. This function does NOT test
   *         wether suburls are wellformed, too.
   */
  bool isMalformed() const  { return m_bIsMalformed; }

  /**
   * @return true if the file is a plain local file and has no filter protocols
   *         attached to it.
   */
  bool isLocalFile() const;
  /**
   * @return true if the file has at least one sub URL.
   *         Use @ref split to get the sub URLs.
   *
   * The function test wether the protocol is a filter protocol and wether
   * the reference is not empty. For performance reasons it does NOT test
   * wether the reference is in turn a well formed URL.
   *
   * @see #isFilterProtocol
   */
  bool hasSubURL() const;

  /**
   * Assumes that the current path is a directory. '_txt' is appended to the
   * current path. The function adds '/' if needed while concatenating.
   * This means it does not matter whether the current path has a trailing
   * '/' or not. If there is none, it becomes appended. If '_txt'
   * has a leading '/' then this one is stripped.
   *
   * @param _txt is considered to be decoded
   */
  void addPath( const QString& _txt );
  /**
   * In comparison to @ref addPath this function does not assume that the current path
   * is a directory. This is only assumed if the current path ends with '/'.
   *
   * @param _txt is considered to be decoded. If the current path ends with '/'
   *             then '_txt' ist just appended, otherwise all text behind the last '/'
   *             in the current path is erased and '_txt' is appended then. It does
   *             not matter whether '_txt' starts with '/' or not.
   */
  void setFileName( const QString&_txt );

  /**
   * @return the filename of the current path. The returned string is decoded.
   *
   * @ref _ignore_trailing_slash_in_path tells whether a trailing '/' should be ignored.
   *                                     This means that the function would return "torben" for
   *                                     <tt>file:/hallo/torben/</tt> and <tt>file:/hallo/torben</tt>.
   *                                     If the flag is set to false, then everything behind the last '/'
   *                                     is considered to be the filename.
   */
  QString filename( bool _ignore_trailing_slash_in_path = TRUE );
  /**
   * @return the directory part of the current path. Everything between the last and the second last '/'
   *         is returned. For example <tt>file:/hallo/torben/</tt> would return "/hallo/torben/" while
   *         <tt>file:/hallo/torben</tt> would return "hallo/". The returned string is decoded.
   *
   * @param _strip_trailing_slash_from_result tells whether the returned result should end with '/' or not.
   *                                          If the path is empty or just "/" then this flag has no effect.
   * @param _ignore_trailing_slash_in_path means that <tt>file:/hallo/torben</tt> and 
   *                                       <tt>file:/hallo/torben/"</tt> would both return <tt>/hallo/</tt>
   *                                       or <tt>/hallo</tt> depending on the other flag
   */
  QString directory( bool _strip_trailing_slash_from_result = TRUE,
		     bool _ignore_trailing_slash_in_path = TRUE );

  /**
   * Changes directory by descending into the given directory.
   * It is assumed the current URL represents a directory.
   * If dir starts with a "/" the
   * current URL will be "protocol://host/dir" otherwise dir will
   * be appended to the path. _dir can be ".."
   * This function wont strip protocols. That means: When you are in
   * tar:/#file:/dir/dir2/my.tgz and you do cd("..") then you will
   * still be in tar:/#file:/dir/dir2/my.tgz.
   *
   * @param zapRef if true, delete the HTML style reference.
   */
  bool cd( const QString& _dir, bool zapRef = TRUE );
  
  /**
   * @return the complete encoded URL.
   */
  QString url() const;
  /*
   * convenience method
   * @return the complete decoded URL, for instance to be displayed to the user.
   */
  QString decodedURL() const { QString s = url( 0 ); decode( s ); return s; }

  /**
   * @return the complete encoded URL.
   *
   * @param _trailing may be ( -1, 0 +1 ). -1 strips a trailing '/' from the path, +1 adds
   *                  a trailing '/' if there is none yet and 0 returns the
   *                  path unchanged.
   */
  QString url( int _trailing ) const;

  /**
   * This function is useful to implement the "Up" button in a file manager for example.
   * @ref #cd does never strip a sub protocol. That means: if you are in
   * tar:/#gzip:/decompress#file:/home/x.tgz and hit the up button you expect to see
   * file:/home. The algotithm tries to go up on the left most URL. If that is not
   * possible it strips the left most URL. It continues stripping URLs as they use
   * stream protocols. If it finds the first protocol implementing a directory structure,
   * in this case "file", it tries to step up there and so on ....
   * One more example: tar:/#gzip:/decompress#tar:/dir/x.tgz#gzip:/decompress#http://www/my.tgz
   * will be returned as tar:/dir#gzip:/decompress#http://www/my.tgz.
   *
   * @param _zapRef tells wether the HTML style reference should be stripped
   */
  KURL upURL( bool _zapRef = TRUE ) const;
  
  KURL& KURL::operator=( const KURL& _u );
  KURL& KURL::operator=( const QString& _url );

  bool KURL::operator==( const KURL& _u ) const;
  bool KURL::operator==( const QString& _u ) const;
  bool KURL::operator!=( const KURL& _u ) const { return !( *this == _u ); }
  bool KURL::operator!=( const QString& _u ) const { return !( *this == _u ); }

  /**
   * This function should be used if you want to ignore trailing '/' characters.
   *
   * @see path
   */
  bool cmp( KURL &_u, bool _ignore_trailing = FALSE );
  
  /**
   * Splits nested URLs like tar:/kdebase#gzip:/decompress#file:/home/weis/kde.tgz.
   * An URL like tar:/kde/README.html#http://www.kde.org#ref1 will be split in
   * tar:/kde/README.html#ref1 and http://www.kde.org. That is because http is
   * a source protocol and not a filter protocol. That means in turn that "#ref1"
   * is a HTML style reference and noy a new Sub-URL. Since HTML style references mark
   * a certain position in a document this reference is appended to the first URL.
   * The idea behind that is, that browsers for example only look at the first URL while
   * the rest is not of interest to them.
   *
   * @return an empty list on error or the list of splitted URLs.
   *
   * @param _url is the URL that has to be split.
   */
  static List split( const QString& _url );
  /**
   * A convenience function
   */
  static List split( const KURL& _url );
  /**
   * Reverses @ref #split. Only the first URL may have a reference. This reference
   * is considered to be HTML like and is appended at the end of the resulting
   * joined URL.
   */
  static QString join( const List& _list );
  
  /**
   * Decode the string, this means decoding "%20" into a space for example. Note that "%00" is
   * not handled correctly here.
   */
  static void decode( QString& _url );
  /**
   * Reverse of @ref decode
   */
  static void encode( QString& _url );
  
protected:
  void reset();
  void parse( const QString& _url );
  
  static char hex2int( char _char );
  
private:
  QString m_strProtocol;
  QString m_strUser;
  QString m_strPass;
  QString m_strHost;
  QString m_strPath;
  QString m_strRef_encoded;
  QString m_strQuery_encoded;
  
  bool m_bIsMalformed;
  int m_iPort;
};

/**
 * Compares both URLs. They are parsed, splitted and compared. Two malformed URLs
 * with the same string representation are nevertheless considered to be
 * unequal. That means no malformed URL equals anything else.
 */
bool urlcmp( const QString& _url1, const QString& _url2 );
/**
 * Compares both URLs. They are parsed, splitted and compared. Two malformed URLs
 * with the same string representation are nevertheless considered to be
 * unequal. That means no malformed URL equals anything else.
 *
 * @param _ignore_ref if true disables comparison of HTML style references.
 * @param _ignore_trailing is described in @ref KURL::cmp
 */
bool urlcmp( const QString& _url1, const QString& _url2, bool _ignore_trailing, bool _ignore_ref );

#endif










