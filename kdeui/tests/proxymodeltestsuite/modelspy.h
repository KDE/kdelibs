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


#ifndef MODELSPY_H
#define MODELSPY_H

#include <QObject>
#include <QVariantList>
#include <QModelIndex>

#include "persistentchangelist.h"
#include <QItemSelectionRange>

#include "proxymodeltestsuite_export.h"

enum SignalType
{
  NoSignal,
  RowsAboutToBeInserted,
  RowsInserted,
  RowsAboutToBeRemoved,
  RowsRemoved,
  RowsAboutToBeMoved,
  RowsMoved,
  DataChanged,
  LayoutAboutToBeChanged,
  LayoutChanged,
  ModelAboutToBeReset,
  ModelReset
};

Q_DECLARE_METATYPE( QModelIndex )

class PROXYMODELTESTSUITE_EXPORT ModelSpy : public QObject, public QList<QVariantList>
{
  Q_OBJECT
public:
  ModelSpy(QObject *parent);

  void setModel(QAbstractItemModel *model);
  void setLazyPersistence(bool lazy) { m_lazyPersist = lazy; }

  void preTestPersistIndexes(const PersistentChangeList &changeList);
  QModelIndexList getUnchangedIndexes() const { return m_unchangedIndexes; }
  QList<QPersistentModelIndex> getUnchangedPersistentIndexes() const { return m_unchangedPersistentIndexes; }
  PersistentChangeList getChangeList() const { return m_changeList; }

  void startSpying();
  void stopSpying();
  bool isSpying() { return m_isSpying; }

  void clearTestData();

protected slots:
  void rowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
  void rowsInserted(const QModelIndex &parent, int start, int end);
  void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
  void rowsRemoved(const QModelIndex &parent, int start, int end);
  void rowsAboutToBeMoved(const QModelIndex &srcParent, int start, int end, const QModelIndex &destParent, int destStart);
  void rowsMoved(const QModelIndex &srcParent, int start, int end, const QModelIndex &destParent, int destStart);

  void layoutAboutToBeChanged();
  void layoutChanged();

  void modelAboutToBeReset();
  void modelReset();

  void modelDestroyed();

  void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
  void doPersist();
  QModelIndexList getUnchangedIndexes(const QModelIndex &parent, QList< QItemSelectionRange > ignoredRanges);
  QModelIndexList getDescendantIndexes(const QModelIndex &index);
  QList< QPersistentModelIndex > toPersistent(QModelIndexList list);

private:
  QAbstractItemModel *m_model;
  bool m_isSpying;
  bool m_lazyPersist;
  PersistentChangeList m_changeList;
  QModelIndexList m_unchangedIndexes;
  QList<QPersistentModelIndex> m_unchangedPersistentIndexes;
};

PROXYMODELTESTSUITE_EXPORT uint qHash( const QVariant & var );
#endif

