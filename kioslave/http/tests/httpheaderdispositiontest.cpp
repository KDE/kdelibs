/* This file is part of the KDE libraries
    Copyright (C) 2010,2011 Rolf Eike Beer <kde@opensource.sf-tec.de>

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

#include "httpheaderdispositiontest.h"

#include <qtest_kde.h>

#include <QtCore/QByteArray>
#include <kdebug.h>

#include <parsinghelpers.h>

#include "httpheaderdispositiontest.moc"

#include <parsinghelpers.cpp>

QTEST_KDEMAIN(HeaderDispositionTest, NoGUI)

static void runTest(const char *header, const char *result)
{
    QMap<QString, QString> parameters = contentDispositionParser(header);

    QList<QByteArray> results = QByteArray(result).split('\n');
    if (strlen(result) == 0)
        results.clear();

    foreach (const QByteArray &ba, results) {
        QList<QByteArray> values = ba.split('\t');
        const QString key(values.takeFirst());

        QVERIFY(parameters.contains(key));

        const QByteArray val = values.takeFirst();
        QVERIFY(values.isEmpty());

        QCOMPARE(parameters[key], QString::fromUtf8(val.constData(), val.length()));
    }

    QCOMPARE(parameters.count(), results.count());
}

static const struct {
	const char *header;
	const char *result;
} testpatterns[] = {
    { "inline",
      "type\tinline" },
    { "inline; filename=\"foo.html\"",
      "type\tinline\n"
      "filename\tfoo.html" },
    { "inline; filename=\"foo.pdf\"",
      "type\tinline\n"
      "filename\tfoo.pdf" },
    { "attachment",
      "type\tattachment" },
    { "ATTACHMENT",
      "type\tattachment" },
    { "attachment; filename=\"foo.html\"",
      "type\tattachment\n"
      "filename\tfoo.html" },
    { "attachment; filename=\"f\\oo.html\"",
      "type\tattachment\n"
      "filename\tfoo.html" },
    { "attachment; foo=\"bar\"; filename=\"foo.html\"",
      "type\tattachment\n"
      "foo\tbar\n"
      "filename\tfoo.html" },
    { "attachment; FILENAME=\"foo.html\"",
      "type\tattachment\n"
      "filename\tfoo.html" },
    { "attachment; filename=\"foo-%41.html\"",
      "type\tattachment\n"
      "filename\tfoo-%41.html" },
    { "attachment; filename=\"foo-%c3%a4-%e2%82%ac.html\"",
      "type\tattachment\n"
      "filename\tfoo-%c3%a4-%e2%82%ac.html" },
    { "attachment; filename =\"foo.html\"",
      "type\tattachment\n"
      "filename\tfoo.html" },
    { "attachment; filename= \"foo.html\"",
      "type\tattachment\n"
      "filename\tfoo.html" },
    { "attachment; creation-date=\"Wed, 12 Feb 1997 16:29:51 -0500\"",
      "type\tattachment\n"
      "creation-date\tWed, 12 Feb 1997 16:29:51 -0500" },
    { "attachment; modification-date=\"Wed, 12 Feb 1997 16:29:51 -0500\"",
      "type\tattachment\n"
      "modification-date\tWed, 12 Feb 1997 16:29:51 -0500" },
    { "foobar",
      "type\tfoobar" },
    { "attachment; filename*=UTF-8''foo-%c3%a4-%e2%82%ac.html",
      "type\tattachment\n"
      "filename\tfoo-ä-€.html" },
// it's not filename, but "filename "
    { "attachment; filename *=UTF-8''foo-%c3%a4.html",
      "type\tattachment" },
    { "attachment; filename*= UTF-8''foo-%c3%a4.html",
      "type\tattachment\n"
      "filename\tfoo-ä.html" },
    { "attachment; filename* =UTF-8''foo-%c3%a4.html",
      "type\tattachment\n"
      "filename\tfoo-ä.html" },
// argument must not be enclosed in double quotes
    { "attachment; filename*=\"UTF-8''foo-%c3%a4.html\"",
      "type\tattachment" },
// no character set given but 8 bit characters
    { "attachment; filename*=''foo-%c3%a4.html",
      "type\tattachment" },
    { "attachment; filename*0=\"foo.\"; filename*1=\"html\"",
      "type\tattachment\n"
      "filename\tfoo.html" },
    { "attachment; filename*0*=UTF-8''foo-%c3%a4; filename*1=\".html\"",
      "type\tattachment\n"
      "filename\tfoo-ä.html" },
// invalid argument, should be ignored
    { "attachment; filename==?ISO-8859-1?Q?foo-=E4.html?=",
      "type\tattachment" },
// there may not be gaps in numbering
    { "attachment; filename*0=\"foo.\"; filename*1=\"html\"; filename*3=\"bar\"",
      "type\tattachment\n"
      "filename\tfoo.html" },
// we must not rely on element ordering
    { "attachment; filename*1=\"html\"; filename*0=\"foo.\"",
      "type\tattachment\n"
      "filename\tfoo.html" },
// first element must have number 0
    { "attachment; filename*1=\"foo.\"; filename*2=\"html\"",
      "type\tattachment" },
// no leading zeros
    { "attachment; filename*0=\"foo\"; filename*01=\".html\"",
      "type\tattachment\n"
      "filename\tfoo" },
// "wrong" element ordering and encoding
    { "attachment; filename*1=\"html\"; filename*0*=us-ascii''foo.",
      "type\tattachment\n"
      "filename\tfoo.html" },
// unknown charset
    { "attachment; filename*=unknown''foo",
      "type\tattachment" },
// no apostrophs
    { "attachment; filename*=foo",
      "type\tattachment" },
// only one apostroph
    { "attachment; filename*=us-ascii'foo",
      "type\tattachment" },
// duplicate filename, both should be ignored and parsing should stop
    { "attachment; filename=foo; filename=bar; foo=bar",
      "type\tattachment" },
// garbage after closing quote, parsing should stop there
    {  "attachment; filename*=''foo; bar=\"f\"oo; baz=foo",
       "type\tattachment\n"
       "filename\tfoo" },
// trailing whitespace should be ignored
    {  "attachment; filename=\"foo\" ; bar=baz",
       "type\tattachment\n"
       "filename\tfoo\n"
       "bar\tbaz" },
// invalid syntax for type
    {  "filename=foo.html",
       "" },
// invalid syntax for type
    {  "inline{; filename=\"foo\"",
       "" },
// invalid syntax for type
    {  "foo bar; filename=\"foo\"",
       "" },
// invalid syntax for type
    {  "foo\tbar; filename=\"foo\"",
       "" },
// invalid syntax
    {  "inline; attachment; filename=foo.html",
       "type\tinline" },
// invalid syntax
    {  "attachment; inline; filename=foo.html",
       "type\tattachment" },
// specification bug in RfC 2616, legal through RfC 2183 and draft-ietf-httpbis-content-disp
    { "attachment; filename=foo.html",
      "type\tattachment\n"
      "filename\tfoo.html" },
// specifying both param and param* is allowed, param* should be taken
    { "attachment; filename=foo.html;filename*=UTF-8''foo-%c3%a4-%e2%82%ac.html",
      "type\tattachment\n"
      "filename\tfoo-ä-€.html" },
// specifying both param and param* is allowed, param* should be taken
    { "attachment; filename*=UTF-8''foo-%c3%a4-%e2%82%ac.html; filename=foo.html",
      "type\tattachment\n"
      "filename\tfoo-ä-€.html" },
// missing closing quote, so parameter is broken
    { "attachment; filename=\"bar",
      "type\tattachment" },
// we ignore any path given in the header and use only the filename
    { "attachment; filename=\"/etc/shadow\"",
      "type\tattachment\n"
      "filename\tshadow" },
// we ignore any path given in the header and use only the filename even if there is an error later
      { "attachment; filename=\"/etc/shadow\"; foo=\"baz\"; foo=\"bar\"",
      "type\tattachment\n"
      "filename\tshadow" },
// control characters are forbidden in the quoted string
      { "attachment; filename=\"foo\003\"",
      "type\tattachment" },
// duplicate keys must be ignored
      { "attachment; filename=\"bar\"; filename*0=\"foo.\"; filename*1=\"html\"",
      "type\tattachment" }
};

#if 0
// deactivated for now: failing due to missing implementation
{"attachment; filename=\"foo-&#xc3;&#xa4;.html\"",
"type\tattachment\n"
"filename\tfoo-Ã¤.html" },
// deactivated for now: failing due to missing implementation
{ "attachment; filename*=iso-8859-1''foo-%E4.html",
"type\tattachment\n"
"filename\tfoo-ä.html",
// deactivated for now: not the same utf, no idea if the expected value is actually correct
{ "attachment; filename*=UTF-8''foo-a%cc%88.html",
      "type\tattachment\n"
      "filename\tfoo-ä.html" }

// deactivated for now: only working to missing implementation
// string is not valid iso-8859-1 so filename should be ignored
//"attachment; filename*=iso-8859-1''foo-%c3%a4-%e2%82%ac.html",
//"type\tattachment",

// deactivated for now: only working to missing implementation
// should not be decoded
//"attachment; filename=\"=?ISO-8859-1?Q?foo-=E4.html?=\"",
//"type\tattachment\n"
//"filename\t=?ISO-8859-1?Q?foo-=E4.html?=",

};

static const char * const isoheaders[] =
{
"attachment; filename=\"foo-ä.html\""
};

static const char * const isoresults[] =
{
"type\tattachment\n"
"filename\tfoo-ä.html",
};
#endif

void HeaderDispositionTest::runAllTests()
{
    for (unsigned int i = 0; i < sizeof(testpatterns) / sizeof(testpatterns[0]); i++)
        runTest(testpatterns[i].header, testpatterns[i].result);
// deactivated for now: failing due to missing implementation
//    for (unsigned int i = 0; i < sizeof(isoheaders) / sizeof(isoheaders[0]); i++)
//        runTest(QString(isoheaders[i]).toLatin1().constData(), isoresults[i]);
}
