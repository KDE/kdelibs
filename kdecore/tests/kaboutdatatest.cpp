/* This file is part of the KDE libraries
    Copyright (C) 2011 David Faure <faure@kde.org>

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

#include <QObject>

#include <QtTest/QtTest>
#include <kaboutdata.h>

class KAboutDataTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testKAboutDataOrganizationDomain();
};


void KAboutDataTest::testKAboutDataOrganizationDomain()
{
    KAboutData data( "app", 0, qi18n("program"), "version",
                     qi18n("description"), KAboutData::License_LGPL,
                     qi18n("copyright"), qi18n("hello world"),
                     "http://www.koffice.org" );
    QCOMPARE( data.organizationDomain(), QString::fromLatin1( "koffice.org" ) );

    KAboutData data2( "app", 0, qi18n("program"), "version",
                      qi18n("description"), KAboutData::License_LGPL,
                      qi18n("copyright"), qi18n("hello world"),
                      "http://edu.kde.org/kig" );
    QCOMPARE( data2.organizationDomain(), QString::fromLatin1( "kde.org" ) );
}

QTEST_MAIN(KAboutDataTest)

#include "kaboutdatatest.moc"



