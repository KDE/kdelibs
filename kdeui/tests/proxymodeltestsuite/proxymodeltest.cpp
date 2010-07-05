/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

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

#include "proxymodeltest.h"

#include "dynamictreemodel.h"

#include <QItemSelectionModel>
#include <QSortFilterProxyModel>

#include "modelspy.h"

ProxyModelTest::ProxyModelTest(QObject *parent)
  : QObject(parent),
    m_rootModel(new DynamicTreeModel(this)),
    m_sourceModel(m_rootModel),
    m_proxyModel(0),
    m_intermediateProxyModel(0),
    m_modelSpy(new ModelSpy(this)),
    m_modelCommander(new ModelCommander(m_rootModel, this))
{
}

void ProxyModelTest::setLazyPersistence(Persistence persistence)
{
  m_modelSpy->setLazyPersistence(persistence == LazyPersistence);
}

void ProxyModelTest::setUseIntermediateProxy(SourceModel sourceModel)
{
  if (sourceModel == DynamicTree)
    return;

  m_intermediateProxyModel = new QSortFilterProxyModel(this);
  m_intermediateProxyModel->setSourceModel(m_rootModel);
  m_sourceModel = m_intermediateProxyModel;
}

void ProxyModelTest::initRootModel(DynamicTreeModel *rootModel, const QString &currentTest, const QString &currentTag)
{
  Q_UNUSED(rootModel)
  // Get the model into the state it is expected to be in.
  QMetaObject::invokeMethod(m_modelCommander, QString("init_" + currentTest).toAscii(), Q_ARG(QString, currentTag));
}

void ProxyModelTest::verifyExecutedTests()
{
  if (m_dataTags.contains(ProxyModelTestData::failTag()))
    return;
  QSet<QString> unimplemented = m_modelCommanderTags.toSet().subtract(m_dataTags.toSet());
  QString unimplementedTestsString("(");
  foreach(const QString &test, unimplemented)
    unimplementedTestsString.append(test + ",");
  unimplementedTestsString.append(")");

  if (!unimplemented.isEmpty())
  {
    QString failString = QString("Some tests in %1 were not implemented: %2").arg(m_currentTest, unimplementedTestsString);
    m_dataTags.clear();
    m_currentTest = QTest::currentTestFunction();
    QFAIL(failString.toAscii());
  }
}

void ProxyModelTest::init()
{
  QVERIFY(m_modelSpy->isEmpty());
  m_rootModel->clear();

  const char *currentTest = QTest::currentTestFunction();
  const char *currentTag = QTest::currentDataTag();
  QVERIFY(currentTest != 0);
  initRootModel(m_rootModel, currentTest, currentTag);

  Q_ASSERT(sourceModel());
  QAbstractProxyModel *proxyModel = getProxy();

  Q_ASSERT(proxyModel);
  // Don't set the sourceModel in getProxy.
  Q_ASSERT(!proxyModel->sourceModel());
  connectProxy(proxyModel);

  // Get the model into the state it is expected to be in.
  m_modelSpy->startSpying();
  QVERIFY(m_modelSpy->isEmpty());

  if (m_currentTest != currentTest)
  {
    verifyExecutedTests();
    m_dataTags.clear();
    QString metaMethod = QString("execute_" + QLatin1String(currentTest));
    QMetaObject::invokeMethod(m_modelCommander, metaMethod.toAscii(), Q_RETURN_ARG(QStringList, m_modelCommanderTags), Q_ARG(QString, QString()));
    m_currentTest = currentTest;
  }
  m_dataTags.append(currentTag);
}

void ProxyModelTest::cleanup()
{
  QVERIFY(m_modelSpy->isEmpty());
  m_modelSpy->stopSpying();
  m_modelSpy->setModel(0);
  m_proxyModel->setSourceModel(0);
  delete m_proxyModel;
  m_proxyModel = 0;
  QVERIFY(m_modelSpy->isEmpty());
}

void ProxyModelTest::cleanupTestCase()
{
  verifyExecutedTests();
  m_modelCommanderTags.clear();
  if (!m_intermediateProxyModel)
    return;

  m_sourceModel = m_rootModel;
  delete m_intermediateProxyModel;
  m_intermediateProxyModel = 0;

  m_modelSpy->clear();
}

PersistentIndexChange ProxyModelTest::getChange(IndexFinder parentFinder, int start, int end, int difference, bool toInvalid)
{
  Q_ASSERT(start <= end);
  PersistentIndexChange change;
  change.parentFinder = parentFinder;
  change.startRow = start;
  change.endRow = end;
  change.difference = difference;
  change.toInvalid = toInvalid;
  return change;
}

void ProxyModelTest::handleSignal(QVariantList expected)
{
  QVERIFY(!expected.isEmpty());
  int signalType = expected.takeAt(0).toInt();
  if (NoSignal == signalType)
    return;

  Q_ASSERT(!m_modelSpy->isEmpty());
  QVariantList result = getResultSignal();

  QCOMPARE(result.takeAt(0).toInt(), signalType);
  // Check that the signal we expected to recieve was emitted exactly.
  switch (signalType)
  {
  case RowsAboutToBeInserted:
  case RowsInserted:
  case RowsAboutToBeRemoved:
  case RowsRemoved:
  {
    QVERIFY( expected.size() == 3 );
    IndexFinder parentFinder = qvariant_cast<IndexFinder>(expected.at(0));
    parentFinder.setModel(m_proxyModel);
    QModelIndex parent = parentFinder.getIndex();

// This is where is usually goes wrong...
#if 0
    kDebug() << qvariant_cast<QModelIndex>(result.at(0)) << parent;
    kDebug() << result.at(1) << expected.at(1);
    kDebug() << result.at(2) << expected.at(2);
#endif

    QCOMPARE(qvariant_cast<QModelIndex>(result.at(0)), parent );
    QCOMPARE(result.at(1), expected.at(1) );
    QCOMPARE(result.at(2), expected.at(2) );
    break;
  }
  case LayoutAboutToBeChanged:
  case LayoutChanged:
  {
    QVERIFY( expected.size() == 0 );
    QVERIFY( result.size() == 0 );
    break;
  }
  case RowsAboutToBeMoved:
  case RowsMoved:
  {
    QVERIFY( expected.size() == 5 );
    IndexFinder scrParentFinder = qvariant_cast<IndexFinder>(expected.at(0));
    scrParentFinder.setModel(m_proxyModel);
    QModelIndex srcParent = scrParentFinder.getIndex();
    QCOMPARE(qvariant_cast<QModelIndex>(result.at(0)), srcParent );
    QCOMPARE(result.at(1), expected.at(1) );
    QCOMPARE(result.at(2), expected.at(2) );
    IndexFinder destParentFinder = qvariant_cast<IndexFinder>(expected.at(3));
    destParentFinder.setModel(m_proxyModel);
    QModelIndex destParent = destParentFinder.getIndex();
    QCOMPARE(qvariant_cast<QModelIndex>(result.at(3)), destParent );
    QCOMPARE(result.at(4), expected.at(4) );
    break;
  }
  case DataChanged:
  {
    QVERIFY( expected.size() == 2 );
    IndexFinder topLeftFinder = qvariant_cast<IndexFinder>(expected.at(0));
    topLeftFinder.setModel(m_proxyModel);
    QModelIndex topLeft = topLeftFinder.getIndex();
    IndexFinder bottomRightFinder = qvariant_cast<IndexFinder>(expected.at(1));
    bottomRightFinder.setModel(m_proxyModel);

    QModelIndex bottomRight = bottomRightFinder.getIndex();

    QVERIFY(topLeft.isValid() && bottomRight.isValid());

#if 0
    kDebug() << qvariant_cast<QModelIndex>(result.at(0)) << topLeft;
    kDebug() << qvariant_cast<QModelIndex>(result.at(1)) << bottomRight;
#endif

    QCOMPARE(qvariant_cast<QModelIndex>(result.at(0)), topLeft );
    QCOMPARE(qvariant_cast<QModelIndex>(result.at(1)), bottomRight );
  }

  }
}

QVariantList ProxyModelTest::getResultSignal()
{
  return m_modelSpy->takeFirst();
}

void ProxyModelTest::testEmptyModel()
{
  Q_ASSERT(sourceModel());
  QAbstractProxyModel *proxyModel = getProxy();
  // Many of these just check that the proxy does not crash when it does not have a source model.
  QCOMPARE(proxyModel->rowCount(), 0);
  QCOMPARE(proxyModel->columnCount(), 0);
  QVERIFY(!proxyModel->index(0,0).isValid());
  QVERIFY(!proxyModel->data(QModelIndex()).isValid());
  QVERIFY(!proxyModel->parent(QModelIndex()).isValid());
  QVERIFY(!proxyModel->mapToSource(QModelIndex()).isValid());
  QVERIFY(!proxyModel->mapFromSource(QModelIndex()).isValid());
  QVERIFY(!proxyModel->headerData(0, Qt::Horizontal, Qt::DisplayRole).isValid());
  QVERIFY(!proxyModel->headerData(0, Qt::Vertical, Qt::DisplayRole).isValid());
  Qt::ItemFlags flags = proxyModel->flags ( QModelIndex() );
  QVERIFY( flags == Qt::ItemIsDropEnabled || flags == 0 );
  QVERIFY(proxyModel->itemData(QModelIndex()).isEmpty());
  QVERIFY(proxyModel->mapSelectionToSource(QItemSelection()).isEmpty());
  QVERIFY(proxyModel->mapSelectionFromSource(QItemSelection()).isEmpty());
  proxyModel->revert();
  QVERIFY(proxyModel->submit());
  QVERIFY(!proxyModel->sourceModel());
  QVERIFY(!proxyModel->canFetchMore(QModelIndex()));
  proxyModel->fetchMore(QModelIndex());
  QMimeData *data = new QMimeData();
  QVERIFY(!proxyModel->dropMimeData( data, Qt::CopyAction, 0, 0, QModelIndex()));
  delete data;
  QVERIFY(!proxyModel->hasChildren());
  QVERIFY(!proxyModel->hasIndex(0, 0, QModelIndex()));
  proxyModel->supportedDragActions();
  proxyModel->supportedDropActions();
  delete proxyModel;
}

void ProxyModelTest::testSourceReset()
{
  m_modelSpy->stopSpying();
  ModelInsertCommand *ins = new ModelInsertCommand(m_rootModel, this);
  ins->setStartRow(0);
  ins->interpret(
    "- 1"
    "- 2"
    "- - 3"
    "- - 4"
    "- 5"
    "- 6"
    "- 7"
    "- 8"
    "- 9"
    "- - 10"
  );
  ins->doCommand();
  // The proxymodel should reset any internal state it holds when the source model is reset.
  QPersistentModelIndex pmi = m_proxyModel->index(0, 0);
  testMappings();
  m_rootModel->clear(); // Resets the model.
  testMappings(); // Calls some rowCount() etc which should test internal structures in the proxy.
  m_proxyModel->setSourceModel(0);

  m_modelSpy->startSpying();
}

void ProxyModelTest::testDestroyModel()
{
  QAbstractItemModel *currentSourceModel = m_sourceModel;
  DynamicTreeModel *rootModel = new DynamicTreeModel(this);
  m_sourceModel = rootModel;

  ModelInsertCommand *ins = new ModelInsertCommand(rootModel, this);
  ins->setStartRow(0);
  ins->interpret(
    " - 1"
    " - 1"
    " - - 1"
    " - 1"
    " - 1"
    " - 1"
    " - 1"
    " - 1"
    " - - 1"
  );
  ins->doCommand();

  QAbstractProxyModel *proxyModel = getProxy();
  connectProxy(proxyModel);

  if(proxyModel->hasChildren())
  {
    m_modelSpy->startSpying();
    delete m_sourceModel;
    m_sourceModel = 0;
    m_modelSpy->stopSpying();
    testMappings();
    QCOMPARE(m_modelSpy->size(), 1);
    QVERIFY(m_modelSpy->takeFirst().first() == ModelReset);
  }
  m_sourceModel = currentSourceModel;
}

void ProxyModelTest::doTestMappings(const QModelIndex &parent)
{
  if (!m_proxyModel)
    return;
  QModelIndex idx;
  QModelIndex srcIdx;
  for (int column = 0; column < m_proxyModel->columnCount(parent); ++column)
  {
    for (int row = 0; row < m_proxyModel->rowCount(parent); ++row)
    {
      idx = m_proxyModel->index(row, column, parent);
      QVERIFY(idx.isValid());
      QVERIFY(idx.row() == row);
      QVERIFY(idx.column() == column);
      QVERIFY(idx.parent() == parent);
      QVERIFY(idx.model() == m_proxyModel);
      srcIdx = m_proxyModel->mapToSource(idx);
      QVERIFY(srcIdx.isValid());
      QVERIFY(srcIdx.model() == m_proxyModel->sourceModel());
      QVERIFY(m_sourceModel == m_proxyModel->sourceModel());
      QVERIFY(idx.data() == srcIdx.data());
      QVERIFY(m_proxyModel->mapFromSource(srcIdx) == idx);
      if (m_proxyModel->hasChildren(idx))
        doTestMappings(idx);
    }
  }
}

void ProxyModelTest::testMappings()
{
  doTestMappings(QModelIndex());
}

void ProxyModelTest::verifyModel(const QModelIndex& parent, int start, int end)
{
  Q_UNUSED(start);
  Q_UNUSED(end);

  QVERIFY(parent.model() == m_proxyModel || !parent.isValid());
}

void ProxyModelTest::verifyModel(const QModelIndex& parent, int start, int end, const QModelIndex& destParent, int dest)
{
  Q_UNUSED(start);
  Q_UNUSED(end);
  Q_UNUSED(dest);

  QVERIFY(parent.model() == m_proxyModel || !parent.isValid());
  QVERIFY(destParent.model() == m_proxyModel || !destParent.isValid());
}

void ProxyModelTest::verifyModel(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
  QVERIFY(topLeft.model() == m_proxyModel || !topLeft.isValid());
  QVERIFY(bottomRight.model() == m_proxyModel || !bottomRight.isValid());
}

void ProxyModelTest::connectProxy(QAbstractProxyModel *proxyModel)
{
  if (m_proxyModel)
  {
    disconnect(m_proxyModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
            this, SLOT(testMappings()));
    disconnect(m_proxyModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this, SLOT(testMappings()));
    disconnect(m_proxyModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
            this, SLOT(testMappings()));
    disconnect(m_proxyModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this, SLOT(testMappings()));
    disconnect(m_proxyModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(testMappings()));
    disconnect(m_proxyModel, SIGNAL(layoutChanged()),
            this, SLOT(testMappings()));
    disconnect(m_proxyModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int,const QModelIndex &, int)),
            this, SLOT(testMappings()));
    disconnect(m_proxyModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
            this, SLOT(testMappings()));
    disconnect(m_proxyModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(testMappings()));

    disconnect(m_proxyModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int)));
    disconnect(m_proxyModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int)));
    disconnect(m_proxyModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int)));
    disconnect(m_proxyModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int)));
    disconnect(m_proxyModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int,const QModelIndex &, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int, const QModelIndex &, int)));
    disconnect(m_proxyModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int, const QModelIndex &, int)));
    disconnect(m_proxyModel, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int)));
    disconnect(m_proxyModel, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int)));
    disconnect(m_proxyModel, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int)));
    disconnect(m_proxyModel, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int)));
    disconnect(m_proxyModel, SIGNAL(columnsAboutToBeMoved(const QModelIndex &, int, int,const QModelIndex &, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int, const QModelIndex &, int)));
    disconnect(m_proxyModel, SIGNAL(columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int, const QModelIndex &, int)));
    disconnect(m_proxyModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(verifyModel(const QModelIndex &, const QModelIndex &)));
  }

  m_proxyModel = proxyModel;
  QVERIFY(m_modelSpy->isEmpty());
  m_modelSpy->setModel(m_proxyModel);

  QVERIFY(m_modelSpy->isEmpty());

  m_modelSpy->startSpying();
  m_proxyModel->setSourceModel(m_sourceModel);
  m_modelSpy->stopSpying();

  QVERIFY(m_modelSpy->size() == 2);
  QVERIFY(m_modelSpy->takeFirst().first() == ModelAboutToBeReset);
  QVERIFY(m_modelSpy->takeFirst().first() == ModelReset);
  QVERIFY(m_modelSpy->isEmpty());
  testMappings();

  connect(m_proxyModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
          SLOT(testMappings()));
  connect(m_proxyModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
          SLOT(testMappings()));
  connect(m_proxyModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
          SLOT(testMappings()));
  connect(m_proxyModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
          SLOT(testMappings()));
  connect(m_proxyModel, SIGNAL(layoutAboutToBeChanged()),
          SLOT(testMappings()));
  connect(m_proxyModel, SIGNAL(layoutChanged()),
          SLOT(testMappings()));
  connect(m_proxyModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int,const QModelIndex &, int)),
          SLOT(testMappings()));
  connect(m_proxyModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
          SLOT(testMappings()));
  connect(m_proxyModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          SLOT(testMappings()));

  connect(m_proxyModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
          SLOT(verifyModel(const QModelIndex &, int, int)));
  connect(m_proxyModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
          SLOT(verifyModel(const QModelIndex &, int, int)));
  connect(m_proxyModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
          SLOT(verifyModel(const QModelIndex &, int, int)));
  connect(m_proxyModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
          SLOT(verifyModel(const QModelIndex &, int, int)));
  connect(m_proxyModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int,const QModelIndex &, int)),
          SLOT(verifyModel(const QModelIndex &, int, int, const QModelIndex &, int)));
  connect(m_proxyModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
          SLOT(verifyModel(const QModelIndex &, int, int, const QModelIndex &, int)));
  connect(m_proxyModel, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
          SLOT(verifyModel(const QModelIndex &, int, int)));
  connect(m_proxyModel, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
          SLOT(verifyModel(const QModelIndex &, int, int)));
  connect(m_proxyModel, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
          SLOT(verifyModel(const QModelIndex &, int, int)));
  connect(m_proxyModel, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
          SLOT(verifyModel(const QModelIndex &, int, int)));
  connect(m_proxyModel, SIGNAL(columnsAboutToBeMoved(const QModelIndex &, int, int,const QModelIndex &, int)),
          SLOT(verifyModel(const QModelIndex &, int, int, const QModelIndex &, int)));
  connect(m_proxyModel, SIGNAL(columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
          SLOT(verifyModel(const QModelIndex &, int, int, const QModelIndex &, int)));
  connect(m_proxyModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          SLOT(verifyModel(const QModelIndex &, const QModelIndex &)));

}

void ProxyModelTest::doTest()
{
  QFETCH( SignalList, signalList );
  QFETCH( PersistentChangeList, changeList );

  QVERIFY(m_modelSpy->isEmpty());

  QString testName = QTest::currentTestFunction();
  QString testDataTag = QTest::currentDataTag();

  if (testDataTag == ProxyModelTestData::failTag())
    return;

  if ((signalList.size() == 1 && signalList.first().size() == 1)
    && signalList.first().first().toString() == "skip")
  {
    return;
  }

  static int numTests = 0;
  if (qApp->arguments().contains("-count"))
    kDebug() << "numTests" << ++numTests;

  m_modelSpy->preTestPersistIndexes(changeList);

  // Run the test.

  Q_ASSERT(m_modelSpy->isEmpty());
  m_modelSpy->startSpying();
  QMetaObject::invokeMethod(m_modelCommander, QString("execute_" + testName).toAscii(), Q_ARG(QString, testDataTag));
  m_modelSpy->stopSpying();

  if (modelSpy()->isEmpty())
    QVERIFY(signalList.isEmpty());

  // Make sure we didn't get any signals we didn't expect.
  if (signalList.isEmpty())
    QVERIFY(modelSpy()->isEmpty());

  const bool isLayoutChange = signalList.contains(QVariantList() << LayoutAboutToBeChanged);

  while (!signalList.isEmpty())
  {
    // Process each signal we recieved as a result of running the test.
    QVariantList expected = signalList.takeAt(0);
    handleSignal(expected);
  }

  // Make sure we didn't get any signals we didn't expect.
  QVERIFY(m_modelSpy->isEmpty());

  // Persistent indexes should change by the amount described in change objects.
  foreach (PersistentIndexChange change, m_modelSpy->getChangeList())
  {
    for (int i = 0; i < change.indexes.size(); i++)
    {
      QModelIndex idx = change.indexes.at(i);
      QPersistentModelIndex persistentIndex = change.persistentIndexes.at(i);

      // Persistent indexes go to an invalid state if they are removed from the model.
      if (change.toInvalid)
      {
        QVERIFY(!persistentIndex.isValid());
        continue;
      }
#if 0
      kDebug() << idx << idx.data() << change.difference << change.toInvalid << persistentIndex.row();
#endif

      QCOMPARE(idx.row() + change.difference, persistentIndex.row());
      QCOMPARE(idx.column(), persistentIndex.column());
      if (!isLayoutChange)
        QCOMPARE(idx.parent(), persistentIndex.parent());
    }

    for (int i = 0; i < change.descendantIndexes.size(); i++)
    {
      QModelIndex idx = change.descendantIndexes.at(i);
      QPersistentModelIndex persistentIndex = change.persistentDescendantIndexes.at(i);

      // The descendant indexes of indexes which were removed should now also be invalid.
      if (change.toInvalid)
      {
        QVERIFY(!persistentIndex.isValid());
        continue;
      }
      // Otherwise they should be unchanged.
      QCOMPARE(idx.row(), persistentIndex.row());
      QCOMPARE(idx.column(), persistentIndex.column());
      if (!isLayoutChange)
        QCOMPARE(idx.parent(), persistentIndex.parent());
    }
  }

  QModelIndexList unchangedIndexes = m_modelSpy->getUnchangedIndexes();
  QList<QPersistentModelIndex> unchangedPersistentIndexes = m_modelSpy->getUnchangedPersistentIndexes();

  // Indexes unaffected by the signals should be unchanged.
  for (int i = 0; i < unchangedIndexes.size(); ++i)
  {
    QModelIndex unchangedIdx = unchangedIndexes.at(i);
    QPersistentModelIndex persistentIndex = unchangedPersistentIndexes.at(i);
    QCOMPARE(unchangedIdx.row(), persistentIndex.row());
    QCOMPARE(unchangedIdx.column(), persistentIndex.column());
    if (!isLayoutChange)
      QCOMPARE(unchangedIdx.parent(), persistentIndex.parent());
  }
  m_modelSpy->clearTestData();
}

void ProxyModelTest::connectTestSignals(QObject *reciever)
{
  if (!reciever)
    return;
  for (int methodIndex = 0; methodIndex < metaObject()->methodCount(); ++methodIndex) {
    QMetaMethod mm = metaObject()->method(methodIndex);
    if (mm.methodType() == QMetaMethod::Signal
      && QString(mm.signature()).startsWith("test")
      && QString(mm.signature()).endsWith("Data()"))
      {
        int slotIndex = reciever->metaObject()->indexOfSlot(mm.signature());
        Q_ASSERT(slotIndex >= 0);
        metaObject()->connect(this, methodIndex, reciever, slotIndex);
      }
  }
}

void ProxyModelTest::disconnectTestSignals(QObject *reciever)
{
  if (!reciever)
    return;
  for (int methodIndex = 0; methodIndex < metaObject()->methodCount(); ++methodIndex) {
    QMetaMethod mm = metaObject()->method(methodIndex);
    if (mm.methodType() == QMetaMethod::Signal
      && QString(mm.signature()).startsWith("test")
      && QString(mm.signature()).endsWith("Data()"))
    {
      int slotIndex = reciever->metaObject()->indexOfSlot(mm.signature());
      Q_ASSERT(slotIndex >= 0);
      metaObject()->disconnect(this, methodIndex, reciever, slotIndex);
    }
  }
}

uint qHash( const QVariant & var )
{
    if ( !var.isValid() || var.isNull() )
        return -1;

    switch ( var.type() )
    {
        case QVariant::Int:
                return qHash( var.toInt() );
            break;
        case QVariant::UInt:
                return qHash( var.toUInt() );
            break;
        case QVariant::Bool:
                return qHash( var.toUInt() );
            break;
        case QVariant::Double:
                return qHash( var.toUInt() );
            break;
        case QVariant::LongLong:
                return qHash( var.toLongLong() );
            break;
        case QVariant::ULongLong:
                return qHash( var.toULongLong() );
            break;
        case QVariant::String:
                return qHash( var.toString() );
            break;
        case QVariant::Char:
                return qHash( var.toChar() );
            break;
        case QVariant::StringList:
                return qHash( var.toString() );
            break;
        case QVariant::ByteArray:
                return qHash( var.toByteArray() );
            break;
        case QVariant::Date:
        case QVariant::Time:
        case QVariant::DateTime:
        case QVariant::Url:
        case QVariant::Locale:
        case QVariant::RegExp:
                return qHash( var.toString() );
            break;
        case QVariant::Map:
        case QVariant::List:
        case QVariant::BitArray:
        case QVariant::Size:
        case QVariant::SizeF:
        case QVariant::Rect:
        case QVariant::LineF:
        case QVariant::Line:
        case QVariant::RectF:
        case QVariant::Point:
        case QVariant::PointF:
            // not supported yet
            break;
       case QVariant::UserType:
        case QVariant::Invalid:
        default:
            return -1;
    }

    // could not generate a hash for the given variant
    Q_ASSERT(0);
	return -1;
} 