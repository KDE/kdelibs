/* This file is part of the KDE libraries
    Copyright (C) 2008, 2009 Andreas Hartmetz <ahartmetz@gmail.com>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef HTTPAUTHENTICATION_H
#define HTTPAUTHENTICATION_H

#ifndef HTTP_H_ // if we're included from http.cpp all necessary headers are already included
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QList>
#include <kio/authinfo.h>
#endif

namespace KIO {
class AuthInfo;
}

class KAbstractHttpAuthentication
{
public:
    KAbstractHttpAuthentication()
    {
        reset();
    }

    static QByteArray bestOffer(const QList<QByteArray> &offers);
    static KAbstractHttpAuthentication *newAuth(const QByteArray &offer);

    virtual ~KAbstractHttpAuthentication() {}

    // reset to state after default construction
    void reset();
    // the authentication scheme: "Digest", "Basic", "NTLM"
    virtual QByteArray scheme() const = 0;
    // initiate next step with challenge string c
    void setChallenge(const QByteArray &c, const KUrl &resource, const QByteArray &httpMethod);
    // KIO compatible data to find cached credentials. Note that username and/or password
    // as well as UI text will NOT be filled in.
    virtual void fillKioAuthInfo(KIO::AuthInfo *ai) const = 0;
    // what to do in response to challenge
    virtual void generateResponse(const QString &user,
                                  const QString &password) = 0;

    // the following accessors contain useful data after 
    // clients process the following fields top to bottom: highest priority is on top

    // malformed challenge and similar problems - it is advisable to reconnect
    bool isError() const { return m_isError; }
    // do not assume wrong credentials if authentication does not succeed.
    // another roundtrip is needed for internal reasons.
    bool retryWithSameCredentials() const { return m_retryWithSameCredentials; }
    // force keep-alive connection because the authentication method requires it
    bool forceKeepAlive() const { return m_forceKeepAlive; }
    // force disconnection because the authentication method requires it
    bool forceDisconnect() const { return m_forceDisconnect; }

    // insert this into the next request header after "Authorization: " or "Proxy-Authorization: "
    QString headerFragment() const { return m_headerFragment; }
    // this is mainly for GUI shown to the user
    QString realm() const;

protected:
    void authInfoBoilerplate(KIO::AuthInfo *a) const;
    void generateResponseCommon(const QString &user, const QString &password);

    QByteArray m_scheme;    // this is parsed from the header and not necessarily == scheme().
    QList<QByteArray> m_challenge;
    KUrl m_resource;
    QByteArray m_httpMethod;

    bool m_isError;
    bool m_retryWithSameCredentials;
    bool m_forceKeepAlive;
    bool m_forceDisconnect;
    QString m_headerFragment;

    QString m_username;
    QString m_password;
};


class KHttpBasicAuthentication : public KAbstractHttpAuthentication
{
public:
    virtual QByteArray scheme() const;
    virtual void fillKioAuthInfo(KIO::AuthInfo *ai) const;
    virtual void generateResponse(const QString &user, const QString &password);
private:
    friend class KAbstractHttpAuthentication;
    KHttpBasicAuthentication()
     : KAbstractHttpAuthentication() {}
};


class KHttpDigestAuthentication : public KAbstractHttpAuthentication
{
public:
    virtual QByteArray scheme() const;
    virtual void fillKioAuthInfo(KIO::AuthInfo *ai) const;
    virtual void generateResponse(const QString &user, const QString &password);
private:
    friend class KAbstractHttpAuthentication;
    KHttpDigestAuthentication()
     : KAbstractHttpAuthentication() {}
};


class KHttpNtlmAuthentication : public KAbstractHttpAuthentication
{
public:
    virtual QByteArray scheme() const;
    virtual void fillKioAuthInfo(KIO::AuthInfo *ai) const;
    virtual void generateResponse(const QString &user, const QString &password);
private:
    friend class KAbstractHttpAuthentication;
    KHttpNtlmAuthentication()
     : KAbstractHttpAuthentication() {}
};

#endif // HTTPAUTHENTICATION_H