/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000-2001 Dawit Alemayehu <adawit@kde.org>
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
 */

#ifndef __KIO_AUTHINFO_H
#define __KIO_AUTHINFO_H

#include <qmap.h>
#include <qvaluelist.h>
#include <kurl.h>


namespace KIO {

/**
 * This class is intended to make it easier to prompt for, cache
 * and retrieve authorization information.
 *
 * When using this class to cache, retrieve or prompt authentication
 * information, you only need to set the necessary attributes. For
 * example, to check whether a password is already cached, the only
 * required information is the URL of the resource and optionally
 * whether or not a path match should be performed.  Similarly, to
 * prompt for password you only need to optionally set the prompt,
 * username (if already supplied), comment and commentLabel fields.
 *
 * <u>SPECIAL NOTE:</u> If you extend this class to add additional
 * paramters do not forget to overload the stream insertion and
 * extraction operators ("<<" and ">>") so that the added data can
 * be correctly serialzed.
 *
 * @short A two way messaging class for passing authentication information.
 * @author Dawit Alemayehu <adawit@kde.org>
 */
class AuthInfo
{
    friend QDataStream& operator<< (QDataStream& s, const AuthInfo& a);
    friend QDataStream& operator>> (QDataStream& s, AuthInfo& a);

public:
   /**
    * Default constructor.
    */
   AuthInfo();

   /**
    * Copy constructor.
    */
   AuthInfo( const AuthInfo& info );

   /**
    * Overloaded equal to operator.
    */
   AuthInfo& operator=( const AuthInfo& info );

   /**
    * Use this method to check if the object was modified.
    * @return true if the object has been modified
    */
   bool isModified() const { return modified; }

   /**
    * Use this method to indicate that this object has been modified.
    * @param flag true to mark the object as modified, false to clear
    */
   void setModified( bool flag ) { modified = flag; }

   /**
    * The URL for which authentication is to be stored.
    *
    * This field is required when attempting to cache authorization
    * and retrieve it.  However, it is not needed when prompting
    * the user for authorization info.
    *
    * This setting is @em required except when prompting the
    * user for password.
    */
   KURL url;

   /**
    * This setting is @em required for caching.
    */
   QString username;

   /**
    * This setting is @em required for caching.
    */
   QString password;

   /**
    * Information to be displayed when prompting
    * the user for authentication information.
    *
    * <u>NOTE:</u>If this field is not set, the authentication
    * dialog simply displays the preset default prompt.
    *
    * This setting is @em optional and empty by default.
    */
   QString prompt;

   /**
    * The text to displayed in the title bar of
    * the password prompting dialog.
    *
    * <u>NOTE:</u>If this field is not set, the authentication
    * dialog simply displays the preset default caption.
    *
    * This setting is @em optional and empty by default.
    */
   QString caption;

   /**
    * Additional comment to be displayed when prompting
    * the user for authentication information.
    *
    * This field allows you to display a short (no more than
    * 80 characters) extra description in the password prompt
    * dialog.  For example, this field along with the
    * @ref commentLabel can be used to describe the server that
    * requested the authentication:
    *
    *  <pre>
    *  Server:   Squid Proxy @ foo.com
    *  </pre>
    *
    * where "Server:" is the commentLabel and the rest is the
    * actual comment.  Note that it is always better to use
    * the @p commentLabel field as it will be placed properly
    * in the dialog rather than to include it within the actual
    * comment.
    *
    * This setting is @em optional and empty by default.
    */
   QString comment;

   /**
    * Descriptive label to be displayed in front of the
    * comment when prompting the user for password.
    *
    * This setting is @em optional and only applicable when
    * the @ref #comment field is also set.
    */
   QString commentLabel;

   /**
    * A unique identifier that allows caching of multiple
    * passwords for different resources in the same server.
    *
    * Mostly this setting is applicable to the HTTP protocol
    * whose authentication scheme explicitly defines the use
    * of such a unique key.  However, any protocol that can
    * generate or supply a unique id can effectively use it
    * to distinguish passwords.
    *
    * (If you are instead interested in caching the authentication
    * info for multiple users to the same server, refer to
    * multipleUserCaching below)
    *
    * This setting is @em optional and not set by default.
    */
   QString realmValue;

   /**
    * Field to store any extra authentication information for
    * protocols that need it (ex: http).
    *
    * This setting is @em optional and mostly applicable for HTTP
    * protocol.  However, any protocol can make use of it to
    * store extra info.
    */
   QString digestInfo;

   /**
    * Flag that, if set, indicates whether a path match should be
    * performed when requesting for cached authorization.
    *
    * A path is deemed to be a match if it is equal to or is a subset
    * of the cached path.  For example, if stored path is "/foo/bar"
    * and the request's path set to "/foo/bar/acme", then it is a match
    * whereas it would not if the request's path was set to "/foo".
    *
    * This setting is @em optional and false by default.
    */
   bool verifyPath;

   /**
    * Flag which if set forces the username field to be read-only.
    *
    * This setting is @em optional and false by default.
    */
   bool readOnly;

   /**
    * Flag to indicate the persistence of the given password.
    *
    * This is a two-way flag, when set before calling openPassDlg
    * it makes the "keep Password" check box visible to the user.
    * In return the flag will indicate the state of the check box.
    * By default if the flag is checked the password will be cached
    * for the entire life of the current KDE session otherwise the
    * cached password is deleted right after the application using
    * it has been closed.
    */
    bool keepPassword;

protected:
    bool modified;
private:
    class AuthInfoPrivate* d;
};

QDataStream& operator<< (QDataStream& s, const AuthInfo& a);
QDataStream& operator>> (QDataStream& s, AuthInfo& a);

/**
 * A Singleton class that provides access to passwords
 * stored in .netrc and kionetrc for automatic login
 * purposes.
 *
 * @short An interface to kionetrc and the ftp .netrc files
 * @author Dawit Alemayehu <adawit@kde.org>
 */
class NetRC
{
public:

  /**
   * Specifies the mode to be used when searching for a
   * matching automatic login info for a given site :
   *
   * @li exactOnly        search entries with exact host name matches.
   * @li defaultOnly      search entries that are specified as "default".
   * @li presetOnly       search entries that are specified as "preset".
   *
   * @see lookup
   */
  enum LookUpMode
  {
      exactOnly = 0x0002,
      defaultOnly = 0x0004,
      presetOnly = 0x0008
  };

  /**
   * Contains auto login information.
   * @see lookup()
   */
  struct AutoLogin
  {
    QString type;
    QString machine;
    QString login;
    QString password;
    QMap<QString, QStringList> macdef;
  };

  /**
   * A reference to the instance of the class.
   * @return the class
   */
  static NetRC* self();

  /**
   * Looks up the @p login information for the given @p url.
   *
   * @param url the url whose login information will be checked
   * @param login the login information will be writte here
   * @param userealnetrc if true, use $HOME/.netrc fle
   * @param type the type of the login. If null, the @p url's protocol
   *        will be taken
   * @param mode the @ref LookUpMode flags (ORed) for the query
   */
  bool lookup( const KURL& url, AutoLogin& login, bool userealnetrc = false,
               QString type = QString::null,
               int mode = (exactOnly|defaultOnly) );
  /**
   * Reloads the auto login information.
   */
  void reload() { isDirty = true; }

protected:
  QString extract( const char*, const char*, int& );
  int openf( const QString& );
  void parse( int );

private:
  NetRC();
  ~NetRC();

private:
  bool isDirty;

  typedef QValueList<AutoLogin> LoginList;
  typedef QMap<QString, LoginList> LoginMap;
  LoginMap loginMap;

  static NetRC* instance;
  class NetRCPrivate;
  NetRCPrivate* d;
};
};
#endif
