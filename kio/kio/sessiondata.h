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
#include <kio/global.h>

namespace KIO  {

class SlaveConfig;


/**
 *
 */
class SessionData : public QObject
{
    Q_OBJECT

public:
    SessionData();
    ~SessionData();

    virtual void configDataFor( KIO::MetaData &configData, const QString &proto,
                                const QString &host );
    virtual void reset();

    struct AuthData;
public slots:
    void slotAuthData( const QCString&, const QCString&, bool );
    void slotDelAuthData( const QCString& );

private:
    class AuthDataList;
    friend class AuthDataList;
    AuthDataList* authData;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class SessionDataPrivate;
    SessionDataPrivate* d;
};

}; // namespace

#endif
