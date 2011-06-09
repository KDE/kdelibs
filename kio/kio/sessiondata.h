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
   write to the Free Software Foundation, Inc., 51 Franklin Street,
   Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KIO_SESSIONDATA_H
#define KIO_SESSIONDATA_H

#include <QtCore/QObject>
#include <kio/global.h>


namespace KIO  {

// KDE5 TODO: remove the KIO_EXPORT and rename to _p.h, this is really internal only

/**
 * @internal
 */
class KIO_EXPORT SessionData : public QObject
{
    Q_OBJECT

public:
    SessionData();
    ~SessionData();

    virtual void configDataFor( KIO::MetaData &configData, const QString &proto,
                                const QString &host );
    virtual void reset();

//    struct AuthData;

private:
    class AuthDataList;
    friend class AuthDataList;
//    AuthDataList* authData;

private:
    class SessionDataPrivate;
    SessionDataPrivate* const d;
};

} // namespace

#endif
