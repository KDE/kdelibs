
#include "authinfo.h"

using namespace KIO;

AuthInfo::AuthInfo()
{
    verifyPath = false;
    readOnly = false;
    keepPassword = false;
    modified = false;

}

AuthInfo::AuthInfo( const AuthInfo& info )
{
    (*this) = info;
}

AuthInfo& AuthInfo::operator= ( const AuthInfo& info )
{
    url = info.url;
    username = info.username;
    password = info.password;
    prompt = info.prompt;
    caption = info.caption;
    comment = info.comment;
    commentLabel = info.commentLabel;
    realmValue = info.realmValue;
    digestInfo = info.digestInfo;
    verifyPath = info.verifyPath;
    readOnly = info.readOnly;
    keepPassword = info.keepPassword;
    modified = info.modified;
    return *this;
}

QDataStream& KIO::operator<< (QDataStream& s, const AuthInfo& a)
{
    s << a.url << a.username << a.password << a.prompt << a.caption
      << a.comment << a.commentLabel << a.realmValue << a.digestInfo
      << Q_UINT8(a.verifyPath ? 1:0) << Q_UINT8(a.readOnly ? 1:0)
      << Q_UINT8(a.keepPassword ? 1:0) << Q_UINT8(a.modified ? 1:0);
    return s;
}

QDataStream& KIO::operator>> (QDataStream& s, AuthInfo& a)
{
    Q_UINT8 vp = 0;
    Q_UINT8 ro = 0;
    Q_UINT8 kp = 0;
    Q_UINT8 m  = 0;
    s >> a.url >> a.username >> a.password >> a.prompt >> a.caption
      >> a.comment >> a.commentLabel >> a.realmValue >> a.digestInfo
      >> vp >> ro >> kp >> m;
    a.verifyPath = (vp != 0);
    a.readOnly = (ro != 0);
    a.keepPassword = (kp != 0);
    a.modified = (m != 0);
    return s;
}
