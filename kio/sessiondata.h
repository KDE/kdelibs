/* This file is part of the KDE project
   Copyright (C) 2000 Dawit Alemayehu <adawit@kde.org

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place -
   Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef __KIO_SESSIONDATA_H
#define __KIO_SESSIONDATA_H

#include <qobject.h>

namespace KIO  {

class SessionData : public QObject
{
    Q_OBJECT

public:
    SessionData();
    ~SessionData();

public slots:
    void slotAuthData( const QCString&, const QCString&, bool );
    void slotDeleteAuthData( const QCString& );
    void slotSessionCookieData( const QString&, int );
    void slotDelSessionCookieData( int );

private:
    struct AuthData;
    struct CookieData;
    class AuthDataList;
    class CookieDataList;

    AuthDataList* authData;
    CookieDataList* cookieData;
};

};

#endif
