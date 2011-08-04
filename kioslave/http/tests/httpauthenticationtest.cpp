/* This file is part of the KDE libraries
    Copyright (C) 2011 Dawit Alemayehu <adawit@kde.org>

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

#define ENABLE_HTTP_AUTH_NONCE_SETTER
#include "httpauthentication.cpp"

#define QTEST_HTTPAUTH_PARSING(input, scheme, result) \
        do { QByteArray chosenHeader, chosenScheme; \
             QList<QByteArray> parsingResult, expectedResult; \
             parseAuthHeader(QByteArray(input), &chosenHeader, &chosenScheme, &parsingResult); \
             QCOMPARE(QByteArray(scheme), chosenScheme); \
             QByteArray r(result); if (!r.isEmpty()) expectedResult = r.split(','); \
             QCOMPARE(parsingResult, expectedResult); } \
        while(0)

#define QTEST_HTTPAUTH_SELECTION(input, expectedOffer) \
        do { QByteArray offer; \
            parseAuthHeader(QByteArray(input), &offer, 0, 0); \
            QCOMPARE(offer, QByteArray(expectedOffer)); } \
        while(0)

#define QTEST_HTTPAUTH_RESPONSE(input, expectedResponse, user, pass, url, cnonce) \
        do { QByteArray bestOffer; \
             parseAuthHeader(QByteArray(input), &bestOffer, 0, 0); \
             KAbstractHttpAuthentication* authObj = KAbstractHttpAuthentication::newAuth(bestOffer); \
             QVERIFY(authObj); \
             if (!QByteArray(cnonce).isEmpty()) authObj->setDigestNonceValue(QByteArray(cnonce)); \
             authObj->setChallenge(bestOffer, KUrl(url), "GET"); \
             authObj->generateResponse(QString(user), QString(pass)); \
             QCOMPARE(authObj->headerFragment().trimmed(), QByteArray(expectedResponse)); \
             delete authObj; } \
        while(0)

QTEST_KDEMAIN(HTTPAuthenticationTest, NoGUI)

static void parseAuthHeader(const QByteArray& header,
                            QByteArray* bestOffer,
                            QByteArray* scheme,
                            QList<QByteArray>* result)
{
    const QList<QByteArray> authHeaders = KAbstractHttpAuthentication::splitOffers(QList<QByteArray>() << header);
    QByteArray chosenHeader = KAbstractHttpAuthentication::bestOffer(authHeaders);

    if (bestOffer) {
        *bestOffer = chosenHeader;
    }

    if (!scheme && !result) {
      return;
    }

    QByteArray authScheme;
    const QList<QByteArray> parseResult = parseChallenge(chosenHeader, &authScheme);

    if (scheme) {
        *scheme = authScheme;
    }

    if (result) {
        *result = parseResult;
    }
}

void HTTPAuthenticationTest::testHeaderParsing()
{
    // Tests cases from http://greenbytes.de/tech/tc/httpauth/
    QTEST_HTTPAUTH_PARSING("Basic realm=\"foo\"", "Basic", "realm,foo");
    QTEST_HTTPAUTH_PARSING("Basic realm=foo", "Basic", "realm,foo");
    QTEST_HTTPAUTH_PARSING("Basic , realm=\"foo\"", "Basic", "realm,foo");
    QTEST_HTTPAUTH_PARSING("Basic, realm=\"foo\"", "", "");
    QTEST_HTTPAUTH_PARSING("Basic", "Basic", "");
    QTEST_HTTPAUTH_PARSING("Basic realm = \"foo\"", "Basic", "realm,foo");
    QTEST_HTTPAUTH_PARSING("Basic realm=\"\\f\\o\\o\"", "Basic", "realm,foo");
    QTEST_HTTPAUTH_PARSING("Basic realm=\"\\\"foo\\\"\"", "Basic", "realm,\"foo\"");
    QTEST_HTTPAUTH_PARSING("Basic realm=\"foo\", bar=\"xyz\"", "Basic", "realm,foo,bar,xyz");
    QTEST_HTTPAUTH_PARSING("Basic bar=\"xyz\", realm=\"foo\"", "Basic", "bar,xyz,realm,foo");
    QTEST_HTTPAUTH_PARSING("Basic realm=\"basic\", Newauth realm=\"newauth\"", "Basic", "realm,basic");
    QTEST_HTTPAUTH_PARSING("Basic realm=\"basic\", Newauth realm=\"newauth\"", "Basic", "realm,basic");

    // Misc. test cases
    QTEST_HTTPAUTH_PARSING("NTLM   ", "NTLM", "");
    QTEST_HTTPAUTH_PARSING("Basic realm=\"", "Basic", "");
    QTEST_HTTPAUTH_PARSING("bAsic bar\t =\t\"baz\", realm =\t\"foo\"", "bAsic", "bar,baz,realm,foo");
    QTEST_HTTPAUTH_PARSING("Basic realm=foo , , ,  ,, bar=\"baz\"\t,", "Basic", "realm,foo,bar,baz");
    QTEST_HTTPAUTH_PARSING("Basic realm=foo, bar = baz", "Basic", "realm,foo,bar,baz");
    QTEST_HTTPAUTH_PARSING("Basic realm=foo bar = baz", "Basic", "realm,foo");
    // quotes around text, every character needlessly quoted
    QTEST_HTTPAUTH_PARSING("Basic realm=\"\\\"\\f\\o\\o\\\"\"", "Basic", "realm,\"foo\"");
    // quotes around text, quoted backslashes
    QTEST_HTTPAUTH_PARSING("Basic realm=\"\\\"foo\\\\\\\\\"", "Basic", "realm,\"foo\\\\");
    // quotes around text, quoted backslashes, quote hidden behind them
    QTEST_HTTPAUTH_PARSING("Basic realm=\"\\\"foo\\\\\\\"\"", "Basic", "realm,\"foo\\\"");
    // invalid quoted text
    QTEST_HTTPAUTH_PARSING("Basic realm=\"\\\"foo\\\\\\\"", "Basic", "");
    // ends in backslash without quoted value
    QTEST_HTTPAUTH_PARSING("Basic realm=\"\\\"foo\\\\\\", "Basic", "");
}

void HTTPAuthenticationTest::testAuthenticationSelection()
{
#ifdef HAVE_LIBGSSAPI
    QTEST_HTTPAUTH_SELECTION("Negotiate , Digest , NTLM , Basic", "Negotiate");
#endif
    QTEST_HTTPAUTH_SELECTION("Digest , NTLM , Basic , NewAuth", "Digest");
    QTEST_HTTPAUTH_SELECTION("NTLM , Basic , NewAuth", "NTLM");
    QTEST_HTTPAUTH_SELECTION("Basic , NewAuth", "Basic");
    QTEST_HTTPAUTH_SELECTION("NTLM   , Basic realm=foo, bar = baz, NTLM", "NTLM");

    // Unknown schemes always return blank, i.e. auth request should be ignored
    QTEST_HTTPAUTH_SELECTION("Newauth realm=\"newauth\"", "");
    QTEST_HTTPAUTH_SELECTION("NewAuth , NewAuth2", "");
}

void HTTPAuthenticationTest::testAuthentication()
{
    // Test cases from  RFC 2617...
    QTEST_HTTPAUTH_RESPONSE("Basic realm=\"WallyWorld\"",
                            "Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==",
                            "Aladdin", "open sesame",,);
    QTEST_HTTPAUTH_RESPONSE("Digest realm=\"testrealm@host.com\", qop=\"auth,auth-int\", nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"",
                            "Digest username=\"Mufasa\", realm=\"testrealm@host.com\", nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", uri=\"/dir/index.html\", algorithm=MD5, qop=auth, cnonce=\"0a4f113b\", nc=00000001, response=\"6629fae49393a05397450978507c4ef1\", opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"",
                            "Mufasa", "Circle Of Life",
                            "http://www.nowhere.org/dir/index.html", "0a4f113b");
}
