#ifndef __kio_authinfo_h__
#define __kio_authinfo_h__

#include <qdatastream.h>
#include <qstring.h>

#include <kurl.h>

namespace KIO
{

/**
 * This class is intended to make it easier to prompt for,
 * cache and retrieve authorization information.
 *
 * NOTE: If you extend this class to add additional paramters
 * do not forget to overload the stream insertion and extraction
 * operators ("<<" and ">>") so that the added data can be
 * correctly serialzed.
 *
 * @short Object to hold authorization information
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
    */
   bool isModified() const { return modified; }

   /**
    * Use this method to indicate that this object has been modified.
    */
   void setModified( bool flag ) { modified = flag; }

   /**
    * URL for which authentication is to be stored.  This
    * field is required when attempting to cache authorization
    * and retrieve it.  However, it is not needed when prompting
    * the user for authorization info. [required except when
    * prompting for password]
    */
   KURL url;

   /**
    * The username supplied by the user. [required for caching].
    */
   QString username;

   /**
    * Password specified by the user. [required for caching]
    */
   QString password;

   /**
    * Information to display to user in the password
    * dialog. [optional]
    */
   QString prompt;

   /**
    * Text displayed in the title bar of password
    * dialog. [optional]
    */
   QString caption;

   /**
    * additional comment to show to user. [optional]
    */
   QString comment;

   /**
    * Text to be used as additional comment (eg: "Command:"). [optional]
    */
   QString commentLabel;

   /**
    * Field used to store a unique identity so that multiple
    * passwords can be cached for one resource (URI). [optional]
    */
   QString realmValue;

   /**
    * Field to store any extra information for protocols that
    * need it. [optional]
    */
   QString digestInfo;

   /**
    * Flag that, if set, indicates whether a path match should be
    * performed when requesting for cached authorization.  A path
    * is deemed to be a match if it is equal to or is a subset of the
    * cached path.  For example, if stored path is /foo/bar and the
    * request is for the same domain with path set to /foo/bar/acme,
    * then it is a match.  However, if the path is set to /foo, then
    * this will result in the request being rejected. [optional]
    */
   bool verifyPath;

   /**
    * Flag which if set forces the username field to be read-only.
    * This is set to false by default. [optional]
    */
   bool readOnly;

   /**
    * Flag to indicate the persistence of the given password.
    *
    * This is a two-way flag when set before calling openPassDlg
    * makes the "keep Password" check box will visible to the user.
    * In return the flag will indicate the state of the check box.
    * By default if the flag is checked the password will be cached
    * for the entire life of the current KDE session otherwise the
    * cached password is deleted right after the application using
    * it has been closed.
    *
    */
    bool keepPassword;

protected:
    bool modified;
};


};

#endif
