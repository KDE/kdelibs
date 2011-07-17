/* This file is part of the KDE libraries
    Copyright (c) 2011 Dawit Alemayehu <adawit@kde.org>

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


#include "httpauthenticationtest.h"

#include <qtest_kde.h>
#include <kdebug.h>
#include <kcodecs.h>
#include <krandom.h>
#include <kconfiggroup.h>
#include <misc/kntlm/kntlm.h>

#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <QtNetwork/QHostInfo>

#include "httpauthentication.cpp"

QTEST_KDEMAIN(HTTPAuthenticationTest, NoGUI)

static void testAuthHeaderFormats(const QByteArray& authHeader,
                                  const QByteArray& expectedScheme,
                                  const QList<QByteArray> expectedValues)
{
    const QList<QByteArray> authHeaders = KAbstractHttpAuthentication::splitOffers(QList<QByteArray>() << authHeader);
    const QByteArray bestOffer = KAbstractHttpAuthentication::bestOffer(authHeaders);

    QByteArray scheme;
    const QList<QByteArray> values = parseChallenge(bestOffer, &scheme);
    QCOMPARE(scheme, expectedScheme);
    QCOMPARE(values, expectedValues);
}

static void testGeneratedAuthResponse(const QByteArray& authHeader,
                                      const QByteArray& expectedResponse,
                                      const QString& user, const QString& password,
                                      const KUrl& resource = KUrl(),
                                      const QByteArray& httpMethod = "GET",
                                      const QByteArray& cnonce = QByteArray())
{
    const QByteArray bestOffer = KAbstractHttpAuthentication::bestOffer(QList<QByteArray>() << authHeader);
    KAbstractHttpAuthentication* authObj = KAbstractHttpAuthentication::newAuth(bestOffer);
    QVERIFY(authObj);
    if (!cnonce.isEmpty()) {
        authObj->setDigestNonceValue(cnonce);
    }
    authObj->setChallenge(bestOffer, resource, httpMethod);
    authObj->generateResponse(user, password);
    QCOMPARE(authObj->headerFragment().trimmed(), expectedResponse);
    delete authObj;
}

static void testAuthBestOffer(const QList<QByteArray>& offers,
                              const QByteArray& expectedBestOffer,
                              QByteArray* offer = 0)
{
    const QByteArray bestOffer = KAbstractHttpAuthentication::bestOffer(offers);
    QCOMPARE(bestOffer, expectedBestOffer);
    if (offer) {
        *offer = bestOffer;
    }
}


static QList<QByteArray> toByteArrayList(const QByteArray& value)
{
    return value.split(',');
}

void HTTPAuthenticationTest::testHeaderParsing()
{
    // Tests cases from http://greenbytes.de/tech/tc/httpauth/
    testAuthHeaderFormats("Basic realm=\"foo\"", "Basic", toByteArrayList("realm,foo"));
    testAuthHeaderFormats("Basic realm=foo", "Basic", toByteArrayList("realm,foo"));
    testAuthHeaderFormats("Basic", "Basic", QList<QByteArray>());
    testAuthHeaderFormats("Basic realm = \"foo\"", "Basic", toByteArrayList("realm,foo"));
    // FIXME: Deal with quoted and escaped values...
    //testAuthHeaderFormats("Basic realm=\"\\f\\o\\o\"", "Basic", toByteArrayList("realm,foo"));
    testAuthHeaderFormats("Basic realm=\"\\\"foo\\\"\"", "Basic", toByteArrayList("realm,foo"));
    testAuthHeaderFormats("Basic realm=\"foo\", bar=\"xyz\"", "Basic", toByteArrayList("realm,foo,bar,xyz"));
    testAuthHeaderFormats("Basic bar=\"xyz\", realm=\"foo\"", "Basic", toByteArrayList("bar,xyz,realm,foo"));
    testAuthHeaderFormats("bAsic bar\t =\t\"baz\", realm =\t\"foo\"", "bAsic", toByteArrayList("bar,baz,realm,foo"));
}

void HTTPAuthenticationTest::testAuthenticationSelection()
{
    // Tests cases from http://greenbytes.de/tech/tc/httpauth/
    testAuthHeaderFormats("Basic realm=\"basic\", Newauth realm=\"newauth\"", "Basic", toByteArrayList("realm,basic"));
    testAuthHeaderFormats("Basic realm=\"basic\", Newauth realm=\"newauth\"", "Basic", toByteArrayList("realm,basic"));

    QByteArray bestOffer;
    QList<QByteArray> authHeaders;
    authHeaders << "NTLM" << "Basic" << "Negotiate" << "Digest" << "UnsupportedAuth";
    testAuthBestOffer(authHeaders, "Negotiate", &bestOffer);
    authHeaders.removeOne(bestOffer);
    testAuthBestOffer(authHeaders, "Digest", &bestOffer);
    authHeaders.removeOne(bestOffer);
    testAuthBestOffer(authHeaders, "NTLM", &bestOffer);
    authHeaders.removeOne(bestOffer);
    testAuthBestOffer(authHeaders, "Basic", &bestOffer);
    authHeaders.removeOne(bestOffer);
    testAuthBestOffer(authHeaders, "", &bestOffer); // Unknown scheme should always return blank!
    QVERIFY(!authHeaders.isEmpty());
    QCOMPARE(authHeaders.first(), QByteArray("UnsupportedAuth"));
}

void HTTPAuthenticationTest::testAuthentication()
{
    // Test cases from  RFC 2617...
    testGeneratedAuthResponse("Basic realm=\"WallyWorld\"",
                              "Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==",
                              QLatin1String("Aladdin"),
                              QLatin1String("open sesame"));
    testGeneratedAuthResponse("Digest realm=\"testrealm@host.com\", qop=\"auth,auth-int\", nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"",
                              "Digest username=\"Mufasa\", realm=\"testrealm@host.com\", nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", uri=\"/dir/index.html\", "
                              "algorithm=MD5, qop=auth, cnonce=\"0a4f113b\", nc=00000001, response=\"6629fae49393a05397450978507c4ef1\", opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"",
                              QLatin1String("Mufasa"),
                              QLatin1String("Circle Of Life"),
                              KUrl("http://www.nowhere.org/dir/index.html"), "GET", "0a4f113b");
}
