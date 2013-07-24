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

#include "klinkitemselectionmodeltest.h"

#include <klinkitemselectionmodel.h>
#include <qtest_kde.h>

#include <QItemSelectionModel>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QStandardItemModel>

QTEST_KDEMAIN(KLinkItemSelectionModelTest, GUI)

void KLinkItemSelectionModelTest::testToggle()
{
    // Init mainModel
    QStandardItemModel mainModel;
    for (int x=0; x < 10; ++x) {
        mainModel.appendRow(new QStandardItem(QString::number(x)));
    }
    QItemSelectionModel mainSelectionModel(&mainModel);

    // Init subModel
    QSortFilterProxyModel subModel;
    subModel.setFilterRegExp(QRegExp("^[5-9]"));
    subModel.setSourceModel(&mainModel);
    KLinkItemSelectionModel subSelectionModel(&subModel, &mainSelectionModel);

    // Select last index in subModel
    QModelIndex subIndex = subModel.index(subModel.rowCount() - 1, 0);
    subSelectionModel.select(subIndex, QItemSelectionModel::Toggle);

    // Check selections
    QModelIndexList subList = subSelectionModel.selectedIndexes();
    QCOMPARE(subList.count(), 1);
    QCOMPARE(subList.first(), subIndex);

    QModelIndexList mainList = mainSelectionModel.selectedIndexes();
    QModelIndex mainIndex = mainModel.index(mainModel.rowCount() - 1, 0);
    QCOMPARE(mainList.count(), 1);
    QCOMPARE(mainList.first(), mainIndex);
}

#include <klinkitemselectionmodeltest.moc>
