/*
    Copyright (c) 2013 Aurélien Gateau <agateau@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include <klinkitemselectionmodeltest.h>

#include <klinkitemselectionmodel.h>
#include <qtest_kde.h>

#include <QItemSelectionModel>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QStandardItemModel>

QTEST_KDEMAIN(KLinkItemSelectionModelTest, GUI)

void KLinkItemSelectionModelTest::init()
{
    // Init m_mainModel
    m_mainModel = new QStandardItemModel;
    for (int x=0; x < 10; ++x) {
        m_mainModel->appendRow(new QStandardItem(QString::number(x)));
    }
    m_mainSelectionModel = new QItemSelectionModel(m_mainModel);

    // Init subModel
    m_subModel = new QSortFilterProxyModel;
    m_subModel->setFilterRegExp(QRegExp("^[5-9]"));
    m_subModel->setSourceModel(m_mainModel);
    m_subSelectionModel = new KLinkItemSelectionModel(m_subModel, m_mainSelectionModel);
}

void KLinkItemSelectionModelTest::cleanup()
{
    delete m_mainSelectionModel;
    m_mainSelectionModel = 0;
    delete m_mainModel;
    m_mainModel = 0;
    delete m_subSelectionModel;
    m_subSelectionModel = 0;
    delete m_subModel;
    m_subModel = 0;
}

void KLinkItemSelectionModelTest::testToggle()
{
    // Select last index in subModel
    QModelIndex subIndex = m_subModel->index(m_subModel->rowCount() - 1, 0);
    m_subSelectionModel->select(subIndex, QItemSelectionModel::Toggle);

    // Check selections
    QModelIndexList subList = m_subSelectionModel->selectedIndexes();
    QCOMPARE(subList.count(), 1);
    QCOMPARE(subList.first(), subIndex);

    QModelIndexList mainList = m_mainSelectionModel->selectedIndexes();
    QModelIndex mainIndex = m_mainModel->index(m_mainModel->rowCount() - 1, 0);
    QCOMPARE(mainList.count(), 1);
    QCOMPARE(mainList.first(), mainIndex);
}

void KLinkItemSelectionModelTest::testMainSetCurrent()
{
    // Set last index of mainModel as current
    QModelIndex mainIndex = m_mainModel->index(m_mainModel->rowCount() - 1, 0);
    m_mainSelectionModel->setCurrentIndex(mainIndex, QItemSelectionModel::Current);

    // Last index of subModel should be current as well
    QModelIndex subIndex = m_subSelectionModel->currentIndex();
    QVERIFY(subIndex.isValid());
    QCOMPARE(subIndex, m_subModel->index(m_subModel->rowCount() - 1, 0));

    // Set first index of mainModel as current. First index of mainModel does
    // not exist in subModel, so its current index should remain the same.
    subIndex = m_subSelectionModel->currentIndex();
    QVERIFY(subIndex.isValid());
    mainIndex = m_mainModel->index(0, 0);
    m_mainSelectionModel->setCurrentIndex(mainIndex, QItemSelectionModel::Current);
    QCOMPARE(subIndex, m_subSelectionModel->currentIndex());
}

void KLinkItemSelectionModelTest::testSubSetCurrent()
{
    // Set last index of subModel as current
    QModelIndex subIndex = m_subModel->index(m_subModel->rowCount() - 1, 0);
    m_subSelectionModel->setCurrentIndex(subIndex, QItemSelectionModel::Current);

    // Last index of mainModel should be current as well
    QModelIndex mainIndex = m_mainSelectionModel->currentIndex();
    QVERIFY(mainIndex.isValid());
    QCOMPARE(mainIndex, m_mainModel->index(m_mainModel->rowCount() - 1, 0));
}

#include <klinkitemselectionmodeltest.moc>
