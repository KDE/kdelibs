/* This file is part of the KDE libraries
    Copyright (c) 2008 Andreas Hartmetz <ahartmetz@gmail.com>

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

#include "httpheadertokenizetest.h"

#include <qtest_kde.h>

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <kdebug.h>

#include <parsinghelpers.h>

#include "httpheadertokenizetest.moc"

#include <parsinghelpers.cpp>

QTEST_KDEMAIN(HeaderTokenizeTest, NoGUI)

/* some possible fields that can be used for test headers
        {"accept-ranges", false},
        {"cache-control", true},
        {"connection", true},
        {"content-disposition", false}, //is multi-valued in a way, but with ";" separator!
        {"content-encoding", true},
        {"content-language", true},
        {"content-length", false},
        {"content-location", false},
        {"content-md5", false},
        {"content-type", false},
        {"date", false},
        {"dav", true}, //RFC 2518
        {"etag", false},
        {"expires", false},
        {"keep-alive", false}, //RFC 2068
        {"last-modified", false},
        {"link", false}, //RFC 2068, multi-valued with ";" separator
        {"location", false},
*/

//no use testing many different headers, just a couple each of the multi-valued
//and the single-valued group to make sure that corner cases work both if there
//are already entries for the header and if there are no entries.
static const char messyHeader[] =
"\n"
"accept-ranges:foo\r\n"
"connection: one\r\n"
" t_\r\n"
" wo,\r\n"
"\tthree\r\n"
"accept-ranges:42\n"
"accept-Ranges:\tmaybe \r"
" or not\n"
"CoNNectIoN:four, , ,,   , \r\n"
" :fi:ve\r\n"
":invalid stuff\r\n"
"\tinvalid: connection:close\t\r"
"connection: Six, seven ,, , eight\r"   //one malformed newline...
"\n\r ";   //two malformed newlines; end of header. also observe the trailing space.

//tab separates values, newline separates header lines. the first word is the key.
static const char messyResult[] =
"accept-ranges\tfoo\t42\tmaybe   or not\n"
"connection\tone   t_   wo\tthree\tfour\t:fi:ve\tSix\tseven\teight";


static const char redirectHeader[] =
//"HTTP/1.1 302 Moved Temporarily\r\n"
"Location: http://www.hertz.de/rentacar/index.jsp?bsc=t&targetPage=reservationOnHomepage.jsp\r\n"
"Connection:close\r\n"
"Cache-Control: no-cache\r\n"
"Pragma: no-cache\r\n"
"\r\n";

static const char redirectResult[] =
"cache-control\tno-cache\n"
"connection\tclose\n"
"location\thttp://www.hertz.de/rentacar/index.jsp?bsc=t&targetPage=reservationOnHomepage.jsp\n"
"pragma\tno-cache";

static const int bufSize = 4096;
char buffer[bufSize];

void HeaderTokenizeTest::testMessyHeader()
{
    //Copy the header into a writable buffer
    for (int i = 0; i < bufSize; i++) {
        buffer[i] = 0;
    }
    strcpy(buffer, messyHeader);

    HeaderTokenizer tokenizer(buffer);
    int tokenizeEnd = tokenizer.tokenize(0, strlen(messyHeader));
    QCOMPARE(tokenizeEnd, (int)(strlen(messyHeader) - 1));

    // If the output of the tokenizer contains all the terms that should be there and
    // exactly the number of terms that should be there then it's exactly correct.
    // We are lax wrt trailing whitespace, by the way. It does neither explicitly matter
    // nor not matter according to the standard. Internal whitespace similarly should not
    // matter but we have to be exact because the tokenizer does not move strings around,
    // it only overwrites \r and \n in case of line continuations.

    typedef QPair<int, int> intPair;    //foreach is a macro and does not like commas

    int nValues = 0;
    foreach (const QByteArray &ba, QByteArray(messyResult).split('\n')) {
        QList<QByteArray> values = ba.split('\t');
        QByteArray key = values.takeFirst();
        nValues += values.count();

        QList<QByteArray> comparisonValues;
        foreach (const intPair &be, tokenizer.value(key).beginEnd) {
            comparisonValues.append(QByteArray(buffer + be.first, be.second - be.first));
        }

        QCOMPARE(comparisonValues.count(), values.count());
        for (int i = 0; i < values.count(); i++) {
            QVERIFY(comparisonValues[i].startsWith(values[i]));
        }
    }

    int nValues2 = 0;
    HeaderTokenizer::ConstIterator it = tokenizer.constBegin();
    for (; it != tokenizer.constEnd(); ++it) {
        nValues2 += it.value().beginEnd.count();
    }
    QCOMPARE(nValues2, nValues);

    return; //comment out for parsed header dump to stdout

    it = tokenizer.constBegin();
    for (; it != tokenizer.constEnd(); ++it) {
        if (!it.value().beginEnd.isEmpty()) {
            kDebug() << it.key() << ":";
        }
        foreach (const intPair &be, it.value().beginEnd) {
            kDebug() << "  " << QByteArray(buffer + be.first, be.second - be.first);
        }
    }
}

void HeaderTokenizeTest::testRedirectHeader()
{
    //Copy the header into a writable buffer
    for (int i = 0; i < bufSize; i++) {
        buffer[i] = 0;
    }
    strcpy(buffer, redirectHeader);

    HeaderTokenizer tokenizer(buffer);
    int tokenizeEnd = tokenizer.tokenize(0, strlen(redirectHeader));
    QCOMPARE(tokenizeEnd, (int)strlen(redirectHeader));

    typedef QPair<int, int> intPair;

    int nValues = 0;
    foreach (const QByteArray &ba, QByteArray(redirectResult).split('\n')) {
        QList<QByteArray> values = ba.split('\t');
        QByteArray key = values.takeFirst();
        nValues += values.count();

        QList<QByteArray> comparisonValues;
        foreach (const intPair &be, tokenizer.value(key).beginEnd) {
            comparisonValues.append(QByteArray(buffer + be.first, be.second - be.first));
        }

        QCOMPARE(comparisonValues.count(), values.count());
        for (int i = 0; i < values.count(); i++) {
            QVERIFY(comparisonValues[i].startsWith(values[i]));
        }
    }

    int nValues2 = 0;
    HeaderTokenizer::ConstIterator it = tokenizer.constBegin();
    for (; it != tokenizer.constEnd(); ++it) {
        nValues2 += it.value().beginEnd.count();
    }
    QCOMPARE(nValues2, nValues);

    // Fix compiler warning
    (void)contentDispositionParser;
}
