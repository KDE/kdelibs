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

#ifndef PROXY_MODEL_TEST_H
#define PROXY_MODEL_TEST_H

#include <QtTest>
#include <QtCore>
#include <qtestevent.h>
#include <QItemSelectionRange>
#include <QAbstractProxyModel>

#include "dynamictreemodel.h"
#include "indexfinder.h"
#include "modelcommander.h"
#include "modelspy.h"
#include "persistentchangelist.h"

typedef QList<QVariantList> SignalList;

Q_DECLARE_METATYPE( SignalList )

enum Persistence
{
  LazyPersistence,
  ImmediatePersistence
};

enum SourceModel
{
  DynamicTree,
  IntermediateProxy
};

class BuiltinTestDataInterface
{
public:
  virtual ~BuiltinTestDataInterface() { }
private:
  virtual void testInsertWhenEmptyData() = 0;
  virtual void testInsertInRootData() = 0;
  virtual void testInsertInTopLevelData() = 0;
  virtual void testInsertInSecondLevelData() = 0;
};

class BuiltinTestInterface : BuiltinTestDataInterface
{
public:
  virtual ~BuiltinTestInterface() { }
private:
  virtual void testInsertWhenEmpty_data() = 0;
  virtual void testInsertWhenEmpty() = 0;

  virtual void testInsertInRoot_data() = 0;
  virtual void testInsertInRoot() = 0;

  virtual void testInsertInTopLevel_data() = 0;
  virtual void testInsertInTopLevel() = 0;

  virtual void testInsertInSecondLevel_data() = 0;
  virtual void testInsertInSecondLevel() = 0;
};

class ProxyModelTest : public QObject, protected BuiltinTestInterface
{
  Q_OBJECT
public:
  ProxyModelTest(QObject *parent = 0);
  virtual ~ProxyModelTest() {}

  void setLazyPersistence(Persistence persistence);
  void setUseIntermediateProxy(SourceModel sourceModel);

  DynamicTreeModel* rootModel() const { return m_rootModel; }
  QAbstractItemModel* sourceModel() const { return m_sourceModel; }
  QAbstractProxyModel* proxyModel() const { return m_proxyModel; }
  ModelSpy* modelSpy() const { return m_modelSpy; }

  PersistentIndexChange getChange(IndexFinder sourceFinder, int start, int end, int difference, bool toInvalid = false);
  QVariantList noSignal() const { return QVariantList() << NoSignal; }
  QVariantList getSignal(SignalType type, IndexFinder parentFinder, int start, int end) const
  { return QVariantList() << type << QVariant::fromValue(parentFinder) << start << end; }

protected:
  virtual QAbstractProxyModel* getProxy() = 0;

  void doCleanupTestCase() { cleanupTestCase(); }
  void doCleanup() { cleanup(); }

signals:
  void testInsertWhenEmptyData();
  void testInsertInRootData();
  void testInsertInTopLevelData();
  void testInsertInSecondLevelData();

protected slots:
  void testMappings();
  void verifyModel(const QModelIndex &parent, int start, int end);
  void verifyModel(const QModelIndex &parent, int start, int end, const QModelIndex &destParent, int dest);
  void verifyModel(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private slots:
  void init();
  void cleanup();
  void cleanupTestCase();

  void testEmptyModel();
  void testSourceReset();
  void testDestroyModel();

  void testInsertWhenEmpty_data() { clearDataTags(); testInsertWhenEmptyData(); }
  void testInsertWhenEmpty() { doTest(); }

  void testInsertInRoot_data() { clearDataTags(); testInsertInRootData(); }
  void testInsertInRoot() { doTest(); }

  void testInsertInTopLevel_data() { clearDataTags(); testInsertInTopLevelData(); }
  void testInsertInTopLevel() { doTest(); }

  void testInsertInSecondLevel_data() { clearDataTags(); testInsertInSecondLevelData(); }
  void testInsertInSecondLevel() { doTest(); }

protected:
  void connectTestSignals(QObject *reciever);
  void disconnectTestSignals(QObject *reciever);

  void connectProxy(QAbstractProxyModel *proxyModel);
  void doTestMappings(const QModelIndex &parent);
  void initRootModel(DynamicTreeModel *rootModel, const QString &currentTest, const QString &currentTag);

  void doTest();
  void handleSignal(QVariantList expected);
  QVariantList getResultSignal();
  int getChange(bool sameParent, int start, int end, int currentPosition, int destinationStart);
  void clearDataTags() { m_dataTags.clear(); }
  QStringList dataTags() const { return m_dataTags; }

private:
  DynamicTreeModel *m_rootModel;
  QAbstractItemModel *m_sourceModel;
  QAbstractProxyModel *m_proxyModel;
  QAbstractProxyModel *m_intermediateProxyModel;
  ModelSpy *m_modelSpy;
  ModelCommander *m_modelCommander;
  QStringList m_dataTags;
};

class ProxyModelTestData : public QObject, BuiltinTestDataInterface
{
  Q_OBJECT
public:
  ProxyModelTestData(ProxyModelTest *parent = 0)
    : QObject(parent), m_proxyModelTest(parent)
  {
  }

protected:
  void dummyTestData()
  {
    QTest::addColumn<SignalList>("signalList");
    QTest::addColumn<PersistentChangeList>("changeList");

    QTest::newRow("fail01") << SignalList() << PersistentChangeList();
    QSKIP("Test not implemented", SkipSingle);
  }

  void processTestName(const QString &name)
  {
    if (m_currentTestFunction != QTest::currentTestFunction())
    {
      m_testNames.clear();
      m_currentTestFunction = QTest::currentTestFunction();
    }
    m_testNames.insert(name);
  }

  QStringList namedTests()
  {
    return m_testNames.toList();
  }

  void noopTest(const QString &name)
  {
    processTestName(name);

    QTest::newRow(name.toAscii()) << SignalList() << PersistentChangeList();
  }

  void testForwardingInsertData(const IndexFinder &indexFinder)
  {
    QTest::addColumn<SignalList>("signalList");
    QTest::addColumn<PersistentChangeList>("changeList");

    newInsertTest("insert01", indexFinder, 0, 0, 10);
    newInsertTest("insert02", indexFinder, 0, 9, 10);
    newInsertTest("insert03", indexFinder, 10, 10, 10);
    newInsertTest("insert04", indexFinder, 10, 19, 10);
    newInsertTest("insert05", indexFinder, 4, 4, 10);
    newInsertTest("insert06", indexFinder, 4, 13, 10);
    newInsertTest("insert07", indexFinder, 0, 0, 10);
    newInsertTest("insert08", indexFinder, 10, 10, 10);
    newInsertTest("insert09", indexFinder, 4, 4, 10);
    newInsertTest("insert10", indexFinder, 0, 4, 10);
    newInsertTest("insert11", indexFinder, 10, 14, 10);
    newInsertTest("insert12", indexFinder, 4, 8, 10);
  }

  void newInsertTest(const QString &name, const IndexFinder &indexFinder, int start, int end, int rowCount)
  {
    processTestName(name);

    SignalList signalList;
    PersistentChangeList persistentList;

    signalList << m_proxyModelTest->getSignal(RowsAboutToBeInserted, indexFinder, start, end);
    signalList << m_proxyModelTest->getSignal(RowsInserted, indexFinder, start, end);

    if (rowCount - 1 + ( end - start + 1 ) > end)
      persistentList << m_proxyModelTest->getChange( indexFinder, start, rowCount - 1, end - start + 1 );

    QTest::newRow(name.toAscii()) << signalList << persistentList;
  }

  void noop_testInsertWhenEmptyData()
  {
    QTest::addColumn<SignalList>("signalList");
    QTest::addColumn<PersistentChangeList>("changeList");

    noopTest("insert01");
    noopTest("insert02");
    noopTest("insert03");
  }

  void noop_testInsertInRootData()
  {
    QTest::addColumn<SignalList>("signalList");
    QTest::addColumn<PersistentChangeList>("changeList");

    // These commands have no effect because this model shows children of selection.

    noopTest("insert01");
    noopTest("insert02");
    noopTest("insert03");
    noopTest("insert04");
    noopTest("insert05");
    noopTest("insert06");
    noopTest("insert07");
    noopTest("insert08");
    noopTest("insert09");
    noopTest("insert10");
    noopTest("insert11");
    noopTest("insert12");
  }

  void noop_testInsertInTopLevelData()
  {
    // Same test names etc.
    noop_testInsertInRootData();
  }

  void noop_testInsertInSecondLevelData()
  {
    noop_testInsertInRootData();
  }

  ProxyModelTest *m_proxyModelTest;
  QString m_currentTestFunction;
  QSet<QString> m_testNames;
};

#define PROXYMODELTEST(TestData, TemplateArg, IntermediateProxy, LazyPersistence, Config) \
  if (testObjects.isEmpty() || testObjects.contains(testNum)) { \
    proxyModelTestClass->setTestData(new TestData<TemplateArg>(proxyModelTestClass)); \
    proxyModelTestClass->setUseIntermediateProxy(IntermediateProxy); \
    proxyModelTestClass->setLazyPersistence(LazyPersistence); \
    qDebug()  << "\n   Running" << proxyModelTestClass->objectName().toAscii() << testNum << ":\n" \
              << "  Source Model:      " << #IntermediateProxy << "\n" \
              << "  Persistence:       " << #LazyPersistence << "\n" \
              Config; \
    result = QTest::qExec(proxyModelTestClass, arguments); \
    if (result != 0) \
      return result; \
  } \
  ++testNum; \

#define PROXYMODELTEST_CUSTOM(TestData, IntermediateProxy, LazyPersistence, Config) \
  if (testObjects.isEmpty() || testObjects.contains(testNum)) { \
    proxyModelTestClass->setTestData(TestData); \
    proxyModelTestClass->setUseIntermediateProxy(IntermediateProxy); \
    proxyModelTestClass->setLazyPersistence(LazyPersistence); \
    qDebug()  << "\n   Running" << proxyModelTestClass->objectName().toAscii() << testNum << ":\n" \
              << "  Source Model:      " << #IntermediateProxy << "\n" \
              << "  Persistence:       " << #LazyPersistence << "\n" \
              Config; \
    result = QTest::qExec(proxyModelTestClass, arguments); \
    if (result != 0) \
      return result; \
  } \
  ++testNum; \

// The DynamicTreeModel uses a unique internalId for the first column of each row.
// In the QSortFilterProxyModel the internalId is shared between all rows of the same parent.
// We test the proxy on top of both so that we know it is not using the internalId of its source model
// which will be different each time the test is run.
#define COMPLETETEST(TestData, TemplateArg, Config) \
  PROXYMODELTEST(TestData, TemplateArg, DynamicTree, ImmediatePersistence, Config) \
  PROXYMODELTEST(TestData, TemplateArg, IntermediateProxy, ImmediatePersistence, Config) \
  PROXYMODELTEST(TestData, TemplateArg, DynamicTree, LazyPersistence, Config) \
  PROXYMODELTEST(TestData, TemplateArg, IntermediateProxy, LazyPersistence, Config) \

#define PROXYMODELTEST_MAIN(TestClass, Body) \
  int main(int argc, char *argv[]) \
  { \
    QApplication app(argc, argv); \
    QList<int> testObjects; \
    QStringList arguments; \
    bool ok; \
    foreach(const QString &arg, app.arguments()) \
    { \
      int testObject = arg.toInt(&ok); \
      if (!ok) \
      { \
        arguments.append(arg); \
        continue; \
      } \
      testObjects.append(testObject); \
    } \
    TestClass *proxyModelTestClass = new TestClass(); \
    proxyModelTestClass->setObjectName( #TestClass ); \
    int result = 0; \
    int testNum = 1; \
     \
    Body \
     \
    delete proxyModelTestClass; \
    return result; \
    } \

#endif
