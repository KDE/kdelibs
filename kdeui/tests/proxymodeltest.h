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

Q_DECLARE_METATYPE( QModelIndex )


class ModelSpy : public QObject, public QList<QVariantList>
{
  Q_OBJECT
public:
  ModelSpy(QObject *parent);

  void setModel(QAbstractItemModel *model);

  void startSpying();
  void stopSpying();

protected slots:
  void rowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
  void rowsInserted(const QModelIndex &parent, int start, int end);
  void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
  void rowsRemoved(const QModelIndex &parent, int start, int end);
  void rowsAboutToBeMoved(const QModelIndex &srcParent, int start, int end, const QModelIndex &destParent, int destStart);
  void rowsMoved(const QModelIndex &srcParent, int start, int end, const QModelIndex &destParent, int destStart);

  void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
  QAbstractItemModel *m_model;
};

struct PersistentIndexChange
{
  IndexFinder parentFinder;
  int startRow;
  int endRow;
  int difference;
  bool toInvalid;
  QModelIndexList indexes;
  QList<QPersistentModelIndex> persistentIndexes;

  QModelIndexList descendantIndexes;
  QList<QPersistentModelIndex> persistentDescendantIndexes;
};

enum SignalType
{
  NoSignal,
  RowsAboutToBeInserted,
  RowsInserted,
  RowsAboutToBeRemoved,
  RowsRemoved,
  RowsAboutToBeMoved,
  RowsMoved,
  DataChanged
};


class ProxyModelTest : public QObject
{
  Q_OBJECT
public:
  ProxyModelTest(QObject *parent = 0);

  void setProxyModel(QAbstractProxyModel *proxyModel);
  DynamicTreeModel* sourceModel();

private slots:
  void init();

  void testInsertAndRemove_data();
  void testInsertAndRemove() { doTest(); }



//   void testMove_data();
//   void testMove() { doTest(); }

protected:
  QModelIndexList getUnchangedIndexes(const QModelIndex &parent, QList< QItemSelectionRange > ignoredRanges);

  QModelIndexList getDescendantIndexes(const QModelIndex &index);
  QList< QPersistentModelIndex > toPersistent(QModelIndexList list);

  PersistentIndexChange getChange(IndexFinder sourceFinder, int start, int end, int difference, bool toInvalid = false);
  QVariantList getSignal(SignalType type, IndexFinder parentFinder, int start, int end);
  void signalInsertion(const QString &name, IndexFinder parentFinder, int startRow, int rowsAffected, int rowCount = -1);
  void signalMove(const QString &name, IndexFinder srcFinder, int start, int end, IndexFinder destFinder, int destRow );
  void signalRemoval(const QString &name, IndexFinder parentFinder, int startRow, int rowsAffected, int rowCount = -1);
  void noSignal(const QString &name);

  void signalDataChange(const QString &name, IndexFinder topLeft, IndexFinder bottomRight);

  void doTest();
  void setExpected(const QString &name, const QList<QVariantList> &list, const QList<PersistentIndexChange> &persistentChanges = QList<PersistentIndexChange>() );
  void handleSignal(QVariantList expected);
  QVariantList getResultSignal();
  int getChange(bool sameParent, int start, int end, int currentPosition, int destinationStart);

private:
  QHash<QString, QList<QVariantList> > m_expectedSignals;
  QHash<QString, QList<PersistentIndexChange> > m_persistentChanges;
  DynamicTreeModel *m_model;
  QAbstractProxyModel *m_proxyModel;
  ModelSpy *m_modelSpy;
  ModelCommander *m_modelCommander;

};

#endif
