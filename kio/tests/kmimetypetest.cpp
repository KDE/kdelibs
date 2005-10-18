/*
 *  Copyright (C) 2005 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kmimetypetest.h"
#include "kmimetypetest.moc"
#include <kmimetype.h>
#include <QtTest/qttest_kde.h>
#include <assert.h>
#include <stdlib.h>

static void checkIcon( const KURL& url, const QString& expectedIcon )
{
    QString icon = KMimeType::iconNameForURL( url );
    COMPARE( icon, expectedIcon );
}

QTTEST_KDEMAIN( KMimeTypeTest, NoGUI )

void KMimeTypeTest::testIcons()
{
    // Obviously those tests will need to be fixed if we ever change the name of the icons
    // but at least they unit-test KMimeType::iconNameForURL.

    // TODO test data-driven testing :)

    checkIcon( "file:/tmp/", "folder" );
    checkIcon( "file:/root/", "folder_locked" );
    checkIcon( "trash:/", "trashcan_full" ); // #100321
    checkIcon( "trash:/foo/", "folder" );

}
