/* This file is part of the KDE libraries
    Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "QtTest/qttest_kde.h"

#include "ktempfiletest.h"
#include "ktempfile.h"
#include "kcmdlineargs.h"
#include "kaboutdata.h"
#include "kapplication.h"
#include "kstandarddirs.h"
#include <qstring.h>
#include <qfile.h>
#include <qdir.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

QTTEST_KDEMAIN( KTempFileTest, NoGUI )

void KTempFileTest::testBasic()
{
    printf("Making tempfile after KApplication constructor.\n");
    KTempFile f4;
    f4.setAutoDelete( true );
    qDebug("Filename = %s", qPrintable(f4.name()));
    bool exists = QFile::exists( f4.name() );
    VERIFY( exists );
}

void KTempFileTest::testFixedExtension()
{
    printf("Making tempfile with \".ps\" extension.\n");
    KTempFile f2(QString::null, ".ps");
    f2.setAutoDelete( true );
    qDebug("Filename = %s", qPrintable(f2.name()));
    COMPARE( f2.name().right(3), QLatin1String(".ps") );
}

void KTempFileTest::testHomeDir()
{
    printf("Making tempfile in home directory.\n");
    const QString home = QDir::homeDirPath();
    KTempFile f3(home+QLatin1String("/testXXX"), ".myEXT", 0666);
    f3.setAutoDelete( true );
    qDebug("Filename = %s", qPrintable(f3.name()));
    COMPARE( f3.name().left( home.length() ), home );
    COMPARE( f3.name().right(6), QLatin1String( ".myEXT" ) );
}

//QString name = locateLocal("socket", "test");
//printf("Socket Filename = %s\n", name.ascii());

#include "ktempfiletest.moc"
