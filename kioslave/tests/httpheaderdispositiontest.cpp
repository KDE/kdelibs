/* This file is part of the KDE libraries
    Copyright (c) 2010 Rolf Eike Beer <kde@opensource.sf-tec.de>

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

#include <qtest_kde.h>

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QObject>
#include <kdebug.h>

#include <parsinghelpers.h>

#include "httpheaderdispositiontest.h"
#include "httpheaderdispositiontest.moc"

#include <parsinghelpers.cpp>

QTEST_KDEMAIN(HeaderDispositionTest, NoGUI)

static const char *attachmentHeader =
"attachment; FILENAME=\"foo.html\"";

static const char *attachmentResult =
"type\tattachment\n"
"filename\tfoo.html";

void HeaderDispositionTest::testUppercaseAttachmentHeader()
{
    QMap<QLatin1String, QString> parameters = contentDispositionParser(attachmentHeader);

    foreach (const QByteArray &ba, QByteArray(attachmentResult).split('\n')) {
        QList<QByteArray> values = ba.split('\t');
        QLatin1String key(values.takeFirst().constData());

        QVERIFY(parameters.contains(key));

        QCOMPARE(QString(values.takeFirst()), parameters[key]);
        QVERIFY(values.isEmpty());
    }

    QCOMPARE(parameters.count(), QByteArray(attachmentResult).split('\n').count());
}
