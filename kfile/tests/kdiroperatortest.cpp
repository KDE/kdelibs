/* This file is part of the KDE libraries
    Copyright (c) 2009 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kdebug.h>
#include <qtest_kde.h>
#include <kdiroperator.h>
#include <kconfiggroup.h>
#include <qtreeview.h>

/**
 * Unit test for KDirOperator
 */
class KDirOperatorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
    }

    void cleanupTestCase()
    {
    }

    void testNoViewConfig()
    {
        KDirOperator dirOp;
        // setIconsZoom tries to write config.
        // Make sure it won't crash if setViewConfig() isn't called
        dirOp.setIconsZoom(5);
        QCOMPARE(dirOp.iconsZoom(), 5);
    }

    void testReadConfig()
    {
        // Test: Make sure readConfig() and then setView() restores
        // the correct kind of view.
        KDirOperator *dirOp = new KDirOperator;
        dirOp->setView(KFile::DetailTree);
        dirOp->setShowHiddenFiles(true);
        KConfigGroup cg(KGlobal::config(), "diroperator");
        dirOp->writeConfig(cg);
        delete dirOp;

        dirOp = new KDirOperator;
        dirOp->readConfig(cg);
        dirOp->setView(KFile::Default);
        QVERIFY(dirOp->showHiddenFiles());
        // KDirOperatorDetail inherits QTreeView, so this test should work
        QVERIFY(qobject_cast<QTreeView*>(dirOp->view()));
        delete dirOp;
    }
};

QTEST_KDEMAIN( KDirOperatorTest, GUI )

#include "kdiroperatortest.moc"
