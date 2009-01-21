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

#include <config-gssapi.h>

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

    // reset to state after default construction.
    void reset();
    // the authentication scheme: "Negotiate", "Digest", "Basic", "NTLM"
    virtual QByteArray scheme() const = 0;
    // initiate authentication with challenge string (from HTTP header) c
    virtual void setChallenge(const QByteArray &c, const KUrl &resource, const QByteArray &httpMethod);
    // return value updated by setChallenge(); if this is false user and password passed
    // to generateResponse will be ignored and may be empty.
    bool needCredentials() const { return m_needCredentials; }
    // KIO compatible data to find cached credentials. Note that username and/or password
    // as well as UI text will NOT be filled in.
    virtual void fillKioAuthInfo(KIO::AuthInfo *ai) const = 0;
    // what to do in response to challenge
    virtual void generateResponse(const QString &user,
                                  const QString &password) = 0;

    // the following accessors return useful data after generateResponse() has been called.
    // clients process the following fields top to bottom: highest priority is on top

    // malformed challenge and similar problems - it is advisable to reconnect
    bool isError() const { return m_isError; }
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
    bool m_needCredentials;
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
    virtual void setChallenge(const QByteArray &c, const KUrl &resource, const QByteArray &httpMethod);
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
    virtual void setChallenge(const QByteArray &c, const KUrl &resource, const QByteArray &httpMethod);
    virtual void fillKioAuthInfo(KIO::AuthInfo *ai) const;
    virtual void generateResponse(const QString &user, const QString &password);
private:
    friend class KAbstractHttpAuthentication;
    KHttpNtlmAuthentication()
     : KAbstractHttpAuthentication() {}
};


#ifdef HAVE_LIBGSSAPI
class KHttpNegotiateAuthentication : public KAbstractHttpAuthentication
{
public:
    virtual QByteArray scheme() const;
    virtual void setChallenge(const QByteArray &c, const KUrl &resource, const QByteArray &httpMethod);
    virtual void fillKioAuthInfo(KIO::AuthInfo *ai) const;
    virtual void generateResponse(const QString &user, const QString &password);
private:
    friend class KAbstractHttpAuthentication;
    KHttpNegotiateAuthentication()
     : KAbstractHttpAuthentication() {}
};
#endif // HAVE_LIBGSSAPI

#endif // HTTPAUTHENTICATION_H
