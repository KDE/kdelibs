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

#include "authinfo.h"

using namespace KIO;

AuthInfo::AuthInfo()
{
    modified = false;
    readOnly = false;
    verifyPath = false;
    keepPassword = false;
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
    Q_UINT8 verify = 0;
    Q_UINT8 ro = 0;
    Q_UINT8 keep = 0;
    Q_UINT8 mod  = 0;
   
    s >> a.url >> a.username >> a.password >> a.prompt >> a.caption
      >> a.comment >> a.commentLabel >> a.realmValue >> a.digestInfo
      >> verify >> ro >> keep >> mod;
    a.verifyPath = (verify != 0);
    a.readOnly = (ro != 0);
    a.keepPassword = (keep != 0);
    a.modified = (mod != 0);
    return s;
}
