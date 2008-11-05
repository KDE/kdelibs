/* This file is part of the KDE project
   Copyright (C) 2006 David Faure <faure@kde.org>

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
#ifndef KDIRMODELTEST_H
#define KDIRMODELTEST_H

#include <QtCore/QObject>
#include <ktempdir.h>
#include <QtCore/QDate>
#include <kdirmodel.h>
#include <QtCore/QEventLoop>

class KDirModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testRowCount();
    void testIndex();
    void testNames();
    void testItemForIndex();
    void testIndexForItem();
    void testData();
    void testReload();
    void testModifyFile();
    void testRenameFile();
    void testRenameDirectory();
    void testChmodDirectory();
    void testExpandToUrl_data();
    void testExpandToUrl();
    void testFilter();
    void testMultipleSlashes();
    void testUrlWithRef();
    void testUrlWithHost();
    void testZipFile();
    // These two must be done last
    void testDeleteFile();
    void testDeleteFiles();

protected Q_SLOTS: // 'more private than private slots' - i.e. not seen by qtestlib
    void slotListingCompleted();
    void slotExpand(const QModelIndex& index);
    void slotRowsInserted(const QModelIndex& index, int, int);

private:
    void recreateTestData();
    void enterLoop();
    void fillModel(bool reload, bool expectAllIndexes = true);
    void collectKnownIndexes();

private:
    QEventLoop m_eventLoop;
    KTempDir* m_tempDir;
    KDirModel m_dirModel;
    QModelIndex m_fileIndex;
    QModelIndex m_specialFileIndex;
    QModelIndex m_secondFileIndex;
    QModelIndex m_dirIndex;
    QModelIndex m_fileInDirIndex;
    QModelIndex m_fileInSubdirIndex;
    QStringList m_topLevelFileNames; // files only

    // for slotExpand
    QStringList m_expectedExpandSignals;
    int m_nextExpectedExpandSignals; // index into m_expectedExpandSignals
    KDirModel* m_dirModelForExpand;
    bool m_rowsInsertedEmitted;
};


#endif
