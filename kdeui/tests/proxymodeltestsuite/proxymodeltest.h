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
#include <qtest_kde.h>
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

class ProxyModelTest : public QObject
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
  QVariantList getSignal(SignalType type, IndexFinder parentFinder, int start, int end) const
  { return QVariantList() << type << QVariant::fromValue(parentFinder) << start << end; }

protected:
  virtual QAbstractProxyModel* getProxy() = 0;

  void doCleanupTestCase() { cleanupTestCase(); }
  void doCleanup() { cleanup(); }

  void testEmptyModel();
  void doTestMappings(const QModelIndex &parent);
  void testSourceReset();
  void testDestroyModel();

protected slots:
  void testMappings();
  void verifyModel(const QModelIndex &parent, int start, int end);
  void verifyModel(const QModelIndex &parent, int start, int end, const QModelIndex &destParent, int dest);
  void verifyModel(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private slots:
  void init();
  void cleanup();
  void cleanupTestCase();

protected:
  void connectTestSignals(QObject *reciever);
  void disconnectTestSignals(QObject *reciever);

  void connectProxy(QAbstractProxyModel *proxyModel);

  void doTest();
  void handleSignal(QVariantList expected);
  QVariantList getResultSignal();
  int getChange(bool sameParent, int start, int end, int currentPosition, int destinationStart);

private:
  DynamicTreeModel *m_rootModel;
  QAbstractItemModel *m_sourceModel;
  QAbstractProxyModel *m_proxyModel;
  QAbstractProxyModel *m_intermediateProxyModel;
  ModelSpy *m_modelSpy;
  ModelCommander *m_modelCommander;

};

#endif
