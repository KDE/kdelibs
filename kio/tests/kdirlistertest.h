/* This file is part of the KDE project
   Copyright (C) 2007 David Faure <faure@kde.org>

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
#ifndef KDIRLISTERTEST_H
#define KDIRLISTERTEST_H

#include <QtCore/QObject>
#include <ktempdir.h>
#include <QtCore/QDate>
#include <kdirlister.h>
#include <QtCore/QEventLoop>

class KDirListerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testOpenUrl();
    void testOpenUrlFromCache();
    void testNewItems();
    void testRefreshItems();
    void testDeleteItem();
    void testRenameItem();
    void testConcurrentListing();
    void testConcurrentHoldingListing();
    void testOpenUrlTwice();
    void testOpenAndStop();
    void testDeleteCurrentDir(); // must be last!

protected Q_SLOTS: // 'more private than private slots' - i.e. not seen by qtestlib
    void exitLoop();
    void slotNewItems(const KFileItemList&);
    void slotNewItems2(const KFileItemList&);
    void slotRefreshItems(const QList<QPair<KFileItem, KFileItem> >&);

Q_SIGNALS:
    void refreshItemsReceived();

private:
    void enterLoop(int exitCount = 1);

private:
    QString path() const { return m_tempDir.name(); }
    int m_exitCount;
    QEventLoop m_eventLoop;
    KTempDir m_tempDir;
    KDirLister m_dirLister;
    KFileItemList m_items;
    KFileItemList m_items2;
    QList<QPair<KFileItem, KFileItem> > m_refreshedItems;
};


#endif
