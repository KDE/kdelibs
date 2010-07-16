/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

#include <QtCore>
#include <QtTest>

#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QTreeView>

#include <qtest_kde.h>

#include <kselectionproxymodel.h>

#include <dynamictreemodel.h>
#include <modelselector.h>
#include <kviewstatesaver.h>
#include <sys/stat.h>

class KViewStateSaverTest : public QObject
{
  Q_OBJECT

private slots:
  void testConnectionSeq1();

};


class DynamicTreeModelStateSaver : public KViewStateSaver
{
  Q_OBJECT
public:
  DynamicTreeModelStateSaver(QObject* parent = 0)
    : KViewStateSaver(parent)
  {

  }

  virtual QModelIndex indexFromConfigString(const QAbstractItemModel* model, const QString& key) const
  {
    return DynamicTreeModel::indexForId(model, key.toLongLong());
  }

  virtual QString indexToConfigString(const QModelIndex& index) const
  {
    return index.data(DynamicTreeModel::DynamicTreeModelId).toString();
  }
};


void KViewStateSaverTest::testConnectionSeq1()
{
  DynamicTreeModel rootModel;

  QSortFilterProxyModel qsfpm;
  qsfpm.setSourceModel(&rootModel);

  // We have to use a QTreeView here and not just a QItemSelectionModel
  // because the behaviour of QItemSelectionModel::setCurrentIndex
  // is not consistent with QTreeView::setCurrentIndex
  // The bug appears to be that setCurrentIndex can cause selection,
  // in which case it clears the current selection,
  // and QTreeView randomly sets things to be the currentIndex when rows are
  // inserted and removed.
  // In short, the whole currentIndex thing is unfortunate.
  // Stephen Kelly - 16 Jul 2010.

  QTreeView treeView;
  treeView.setModel(&qsfpm);

  QItemSelectionModel itemSelectionModel(&qsfpm);
  treeView.setSelectionModel(&itemSelectionModel);

  KSelectionProxyModel selProxy(&itemSelectionModel);
  selProxy.setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);
  selProxy.setSourceModel(&rootModel);

  DynamicTreeModelStateSaver *stateRestorer = new DynamicTreeModelStateSaver;
  stateRestorer->setView(&treeView);;
  stateRestorer->restoreSelection(QStringList() << "2");
  stateRestorer->restoreCurrentItem("3");

  {
    ModelInsertCommand ins(&rootModel);
    ins.setStartRow(0);
    ins.interpret(
      "- 1"
      "- - 2"
    );
    ins.doCommand();
  }
  QTest::qWait(30);
  {
    ModelInsertCommand ins(&rootModel);
    ins.setAncestorRowNumbers(QList<int>() << 0 << 0);
    ins.setStartRow(0);
    ins.setEndRow(0);
    ins.doCommand();
  }
  QTest::qWait(30);
}

QTEST_KDEMAIN(KViewStateSaverTest, GUI)

#include "kviewstatesavertest.moc"
