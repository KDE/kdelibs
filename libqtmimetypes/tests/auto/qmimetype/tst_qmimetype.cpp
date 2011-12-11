/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtMimeTypes addon of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "tst_qmimetype.h"

#include <qmimemagicrulematcher_p.h>
#include <qmimetype_p.h>

#include <qmimetype.h>
#include <qmimedatabase.h>

#include <QtTest/QtTest>

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::initTestCase()
{
    qputenv("XDG_DATA_DIRS", "doesnotexist");
}

// ------------------------------------------------------------------------------------------------

static QString pngMimeTypeName()
{
    return QString::fromLatin1("image/png");
}

// ------------------------------------------------------------------------------------------------

#if 0
static QString pngMimeTypeDisplayName()
{
    return QString::fromLatin1("PNG File");
}
#endif

// ------------------------------------------------------------------------------------------------

static QString pngMimeTypeIconUrl()
{
    return QString::fromLatin1("/usr/share/icons/oxygen/64x64/mimetypes/image-x-generic.png");
}

// ------------------------------------------------------------------------------------------------

static QStringList buildPngMimeTypeFilenameExtensions()
{
    QStringList result;
    result << QString::fromLatin1("*.png");
    return result;
}

// ------------------------------------------------------------------------------------------------

static const QStringList &pngMimeTypeGlobPatterns()
{
    static const QStringList result (buildPngMimeTypeFilenameExtensions());
    return result;
}

// ------------------------------------------------------------------------------------------------

static QMimeTypePrivate buildMimeTypePrivate (
                            const QString &name,
                            const QString &genericIconName,
                            const QString &iconName,
                            const QStringList &globPatterns
                       )
{
    QMimeTypePrivate instantiatedPngMimeTypePrivate;
    instantiatedPngMimeTypePrivate.name = name;
    instantiatedPngMimeTypePrivate.genericIconName = genericIconName;
    instantiatedPngMimeTypePrivate.iconName = iconName;
    instantiatedPngMimeTypePrivate.globPatterns = globPatterns;
    return instantiatedPngMimeTypePrivate;
}

// ------------------------------------------------------------------------------------------------
 void tst_qmimetype::test_isValid()
{
    QMimeType instantiatedPngMimeType (
                  buildMimeTypePrivate(pngMimeTypeName(), pngMimeTypeIconUrl(), pngMimeTypeIconUrl(), pngMimeTypeGlobPatterns())
             );

    QVERIFY(instantiatedPngMimeType.isValid());

    QMimeType otherPngMimeType (instantiatedPngMimeType);

    QVERIFY(otherPngMimeType.isValid());
    QCOMPARE(instantiatedPngMimeType, otherPngMimeType);

    QMimeType defaultMimeType;

    QVERIFY(!defaultMimeType.isValid());
}

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::test_name()
{
    QMimeType instantiatedPngMimeType (
                  buildMimeTypePrivate(pngMimeTypeName(), pngMimeTypeIconUrl(), pngMimeTypeIconUrl(), pngMimeTypeGlobPatterns())
             );

    QMimeType otherPngMimeType (
                  buildMimeTypePrivate(QString(), pngMimeTypeIconUrl(), pngMimeTypeIconUrl(), pngMimeTypeGlobPatterns())
             );

    // Verify that the Id is part of the equality test:
    QCOMPARE(instantiatedPngMimeType.name(), pngMimeTypeName());

    QVERIFY(instantiatedPngMimeType != otherPngMimeType);
    QVERIFY(!(instantiatedPngMimeType == otherPngMimeType));
}

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::test_genericIconName()
{
    QMimeType instantiatedPngMimeType (
                  buildMimeTypePrivate(pngMimeTypeName(), pngMimeTypeIconUrl(), pngMimeTypeIconUrl(), pngMimeTypeGlobPatterns())
             );

    QMimeType otherPngMimeType (
                  buildMimeTypePrivate(pngMimeTypeName(), QString(), pngMimeTypeIconUrl(), pngMimeTypeGlobPatterns())
             );

    // Verify that the IconUrl is part of the equality test:
    QCOMPARE(instantiatedPngMimeType.genericIconName(), pngMimeTypeIconUrl());

    QVERIFY(instantiatedPngMimeType != otherPngMimeType);
    QVERIFY(!(instantiatedPngMimeType == otherPngMimeType));
}

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::test_iconName()
{
    QMimeType instantiatedPngMimeType (
                  buildMimeTypePrivate(pngMimeTypeName(), pngMimeTypeIconUrl(), pngMimeTypeIconUrl(), pngMimeTypeGlobPatterns())
             );

    QMimeType otherPngMimeType (
                  buildMimeTypePrivate(pngMimeTypeName(), pngMimeTypeIconUrl(), QString(), pngMimeTypeGlobPatterns())
             );

    // Verify that the IconUrl is part of the equality test:
    QCOMPARE(instantiatedPngMimeType.iconName(), pngMimeTypeIconUrl());

    QVERIFY(instantiatedPngMimeType != otherPngMimeType);
    QVERIFY(!(instantiatedPngMimeType == otherPngMimeType));
}

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::test_suffixes()
{
    QMimeType instantiatedPngMimeType (
                  buildMimeTypePrivate(pngMimeTypeName(), pngMimeTypeIconUrl(), pngMimeTypeIconUrl(), pngMimeTypeGlobPatterns())
             );

    QMimeType otherPngMimeType (
                  buildMimeTypePrivate(pngMimeTypeName(), pngMimeTypeIconUrl(), pngMimeTypeIconUrl(), QStringList())
             );

    // Verify that the FilenameExtensions are part of the equality test:
    QCOMPARE(instantiatedPngMimeType.globPatterns(), pngMimeTypeGlobPatterns());
    QCOMPARE(instantiatedPngMimeType.suffixes(), QStringList() << QString::fromLatin1("png"));

    QVERIFY(instantiatedPngMimeType != otherPngMimeType);
    QVERIFY(!(instantiatedPngMimeType == otherPngMimeType));
}

// ------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    tst_qmimetype tc;
    return QTest::qExec(&tc, argc, argv);
}
