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
#include <qlist.h>

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
  typedef QList<KURL> List;
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
  KURL( const char *_url );
  KURL( QString& _url );
  KURL( const KURL& _u );
  /**
   * @param _rel_url is considered to be encoded.
   */
  KURL( const KURL& _u, const char *_rel_url );
  
  const char* protocol() const { return m_strProtocol.data(); }
  void setProtocol( const char *_txt ) { m_strProtocol = _txt; }

  const char* user() const { return m_strUser.data(); }
  void setUser( const char *_txt ) { m_strUser = _txt; }
  bool hasUser() const { return !m_strUser.isEmpty(); }

  const char* pass() const { return m_strPass.data(); }
  void setPass( const char *_txt ) { m_strPass = _txt; }
  bool hasPass() const { return !m_strPass.isEmpty(); }

  const char* host() const { return m_strHost.data(); }
  void setHost( const char *_txt ) { m_strHost = _txt; }
  bool hasHost() const { return !m_strHost.isEmpty(); }

  int port() const { return m_iPort; }
  void setPort( int _p ) { m_iPort = _p; }
  
  /**
   * @return the current decoded path. This does NOT include the query.
   *
   */
  const char* path() const  { return m_strPath.data(); }
  /**
   * @param _trailing may be ( -1, 0 +1 ). -1 strips a trailing '/', +1 adds
   *                  a trailing '/' if there is none yet and 0 returns the
   *                  path unchanged. If the URL has no path, then no '/' is added
   *                  anyways. And on the other side: If the path is "/", then this
   *                  character wont be stripped. Reason: "ftp://weis@host" means something
   *                  completly different than "ftp://weis@host/". So adding or stripping
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
  void setPath( const char *_txt ) { m_strPath = _txt; }  
  bool hasPath() const { return !m_strPath.isEmpty(); }

  /**
   * This is useful for HTTP. It looks first for '?' and decodes then.
   * The encoded path is the concatenation of the current path and the query.
   */
  void setEncodedPathAndQuery( const char *_txt );
  /**
   * @return the concatenation if the encoded path , '?' and the encoded query.
   *
   * @param _no_empty_path If set to true then an empty path is substituted by "/".
   */
  QString encodedPathAndQuery( int _trailing = 0, bool _no_empty_path = false );

  /**
   * @param _txt is considered to be encoded. This has a good reason: The query may contain the 0 character.
   */
  void setQuery( const char *_txt ) { m_strQuery_encoded = _txt; }
  /**
   * @return the encoded query. This has a good reason: The query may contain the 0 character.
   */
  const char* query() { return m_strQuery_encoded.data(); }
  
  /**
   * The reference is NEVER decoded automatically.
   */
  const char* ref() { return m_strRef_encoded.data(); }
  /**
   * Set the reference part (everything after '#')
   * @param _txt is considered encoded.
   */
  void setRef( const char *_txt ) { m_strRef_encoded = _txt; }
  bool hasRef() const { return !m_strRef_encoded.isEmpty(); }
  
  bool isMalformed() const  { return m_bIsMalformed; }

  /* @return true if the file is a plain local file (no sub protocol) */
  bool isLocalFile();
  /* @return true if the file has at least one sub URL.
     Use @ref split to get the sub URLs. */
  bool hasSubURL() const;

  /**
   * Assumes that the current path is a directory. '_txt' is appended to the
   * current path. The function adds '/' if needed while concatenating.
   * This means it does not matter wether the current path has a trailing
   * '/' or not. If there is none, it becomes appended. If '_txt'
   * has a leading '/' then this one is stripped.
   *
   * @param _txt is considered to be decoded
   */
  void addPath( const char *_txt );
  /**
   * In comparison to @ref addPath this function does not assume that the current path
   * is a directory. This is only assumed if the current path ends with '/'.
   *
   * @param _txt is considered to be decoded. If the current path ends with '/'
   *             then '_txt' ist just appended, otherwise all text behind the last '/'
   *             in the current path is erased and '_txt' is appended then. It does
   *             not matter wether '_txt' starts with '/' or not.
   */
  void setFileName( const char *_txt );

  /**
   * @return the filename of the current path. The returned string is decoded.
   *
   * @ref _ignore_trailing_slash_in_path tells wether a trailing '/' should be ignored.
   *                                     This means that the function would return "torben" for
   *                                     <tt>file:/hallo/torben/</tt> and <tt>file:/hallo/torben</tt>.
   *                                     If the flag is set to false, then everything behind the last '/'
   *                                     is considered to be the filename.
   */
  QString filename( bool _ignore_trailing_slash_in_path = true );
  /**
   * @return the directory part of the current path. Everything between the last and the second last '/'
   *         is returned. For example <tt>file:/hallo/torben/</tt> would return "/hallo/torben/" while
   *         <tt>file:/hallo/torben</tt> would return "hallo/". The returned string is decoded.
   *
   * @param _strip_trailing_slash_from_result tells wether the returned result should end with '/' or not.
   *                                          If the path is empty or just "/" then this flag has no effect.
   * @param _ignore_trailing_slash_in_path means that <tt>file:/hallo/torben</tt> and 
   *                                       <tt>file:/hallo/torben/"</tt> would both return <tt>/hallo/torben/</tt>
   *                                       or <tt>/hallo/torben</tt> depending on the other flag
   */
  QString directory( bool _strip_trailing_slash_from_result = true, bool _ignore_trailing_slash_in_path = true );
  
  /**
   * @return the complete encoded URL.
   */
  QString url() const;
  /**
   * @return the complete encoded URL.
   *
   * @param _trailing may be ( -1, 0 +1 ). -1 strips a trailing '/' from the path, +1 adds
   *                  a trailing '/' if there is none yet and 0 returns the
   *                  path unchanged.
   */
  QString url( int _trailing ) const;

  KURL& KURL::operator=( const KURL& _u );
  KURL& KURL::operator=( const char* _url );
  KURL& KURL::operator=( QString& _url );

  bool KURL::operator==( const KURL& _u ) const;
  bool KURL::operator==( const char* _u ) const;
  /**
   * This function should be used if you want to ignore trailing '/' characters.
   *
   * @see path
   */
  bool cmp( KURL &_u, bool _ignore_trailing = false );
  
  /**
   * Splits nested URLs like file:/home/weis/kde.gz#gzip:/decompress#tar:/kdebase.
   *
   * @return false on parse error and true otherwise.
   *
   * @param _list holds all URLs after the call. The URLs are just appended, so you have
   *              to clean the list usually before calling this function.
   */
  static bool split( const char *_url, List& _list );
  /**
   * Reverses @ref #split.
   */
  static void join( List& _list, QString& _dest );
  
  /**
   * Decode the string, this means decoding "%20" into a space for example. Note that "%00" is
   * not handled correctly here.
   */
  static void decode( QString& _url );
  /**
   * Reverse of @ref decode
   */
  static void encode( QString& _url );

  /**
   * Changes directory by descending into the given directory.
   * If dir starts with a "/" the
   * current URL will be "protocol://host/dir" otherwise dir will
   * be appended to the path.
   * If 'zapRef' is true, the reference will be deleted.
   * Provided for compatibility with the previous KURL class.
   */
  bool cd( const QString& _dir, bool zapRef = true);
  
protected:
  void reset();
  void parse( const char* _url );
  
  static char hex2int( char _char );

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

typedef KURL::List KURLList;

bool urlcmp( KURLList& _url1, KURLList& _url2 );
bool urlcmp( const char *_url1, const char *_url2 );
bool urlcmp( const char *_url1, const char *_url2, bool _ignore_trailing, bool _ignore_ref );

#endif
