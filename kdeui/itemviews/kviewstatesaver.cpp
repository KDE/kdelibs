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

#include "kviewstatesaver.h"

#include <QtGui/QAbstractScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QTreeView>
#include <QListView>

#include "kdebug.h"

#include "kconfiggroup.h"

static const char * selectionKey = "Selection";
static const char * expansionKey = "Expansion";
static const char * currentKey = "Current";
static const char * scrollStateHorizontalKey = "HorizontalScroll";
static const char * scrollStateVerticalKey = "VerticalScroll";

class KViewStateSaverPrivate
{
public:
  KViewStateSaverPrivate(KViewStateSaver *qq)
    : q_ptr(qq), m_treeView(0), m_selectionModel(0), m_scrollArea(0), m_horizontalScrollBarValue(-1), m_verticalScrollBarValue(-1)
  {

  }

  Q_DECLARE_PUBLIC(KViewStateSaver)
  KViewStateSaver * const q_ptr;

  QStringList getExpandedItems(const QModelIndex &index);

  void restoreState();

  inline void restoreScrollBarState()
  {
    if ( m_horizontalScrollBarValue >= 0 && m_horizontalScrollBarValue <= m_scrollArea->horizontalScrollBar()->maximum() ) {
      m_scrollArea->horizontalScrollBar()->setValue( m_horizontalScrollBarValue );
      m_horizontalScrollBarValue = -1;
    }
    if ( m_verticalScrollBarValue >= 0 && m_verticalScrollBarValue <= m_scrollArea->verticalScrollBar()->maximum() ) {
      m_scrollArea->verticalScrollBar()->setValue( m_verticalScrollBarValue );
      m_verticalScrollBarValue = -1;
    }
  }

  inline bool hasPendingChanges() const
  {
    return !m_pendingCurrent.isEmpty() || !m_pendingExpansions.isEmpty() || !m_pendingSelections.isEmpty();
  }

  const QAbstractItemModel* getModel()
  {
    if ( m_selectionModel && m_selectionModel->model() )
      return m_selectionModel->model();
    else if ( m_treeView && m_treeView->model() )
      return m_treeView->model();
    return 0;
  }

  void rowsInserted( const QModelIndex &index, int start, int end )
  {
    if ( !hasPendingChanges() ) {
      Q_Q(KViewStateSaver);
      QObject::disconnect( getModel(), SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
                            q, SLOT( rowsInserted( const QModelIndex&, int, int ) ) );
      q->deleteLater();
      return;
    }
    restoreState();
  }

  QTreeView *m_treeView;
  QListView *m_listView;
  QItemSelectionModel *m_selectionModel;
  QAbstractScrollArea *m_scrollArea;

  int m_horizontalScrollBarValue;
  int m_verticalScrollBarValue;
  QStringList m_pendingSelections;
  QStringList m_pendingExpansions;
  QString m_pendingCurrent;
};

KViewStateSaver::KViewStateSaver(QObject* parent)
  : QObject(parent), d_ptr( new KViewStateSaverPrivate(this) )
{
  // Delete myself if not finished after two seconds.
  QTimer::singleShot(2000, this, SLOT(deleteLater()));
}

KViewStateSaver::~KViewStateSaver()
{
  delete d_ptr;
}

void KViewStateSaver::setTreeView(QTreeView* treeView)
{
  Q_D(KViewStateSaver);
  d->m_treeView = treeView;
  d->m_scrollArea = treeView;
  d->m_selectionModel = treeView->selectionModel();
}

QTreeView* KViewStateSaver::treeView() const
{
  Q_D(const KViewStateSaver);
  return d->m_treeView;
}

void KViewStateSaver::setListView(QListView* listView)
{
  Q_D(KViewStateSaver);
  d->m_listView = listView;
  d->m_scrollArea = listView;
  d->m_selectionModel = listView->selectionModel();
}

QListView* KViewStateSaver::listView() const
{
  Q_D(const KViewStateSaver);
  return d->m_listView;
}

QItemSelectionModel* KViewStateSaver::selectionModel() const
{
  Q_D(const KViewStateSaver);
  return d->m_selectionModel;
}

QAbstractScrollArea* KViewStateSaver::scrollArea() const
{
  Q_D(const KViewStateSaver);
  return d->m_scrollArea;
}

void KViewStateSaver::setSelectionModel(QItemSelectionModel* selectionModel)
{
  Q_D(KViewStateSaver);
  d->m_selectionModel = selectionModel;
}

void KViewStateSaver::setScrollArea(QAbstractScrollArea* scrollArea)
{
  Q_D(KViewStateSaver);
  d->m_scrollArea = scrollArea;
}

void KViewStateSaverPrivate::restoreState()
{
  Q_Q(KViewStateSaver);
  if ( !hasPendingChanges() )
    return;

  if ( m_selectionModel && m_selectionModel->model() )
  {
    QModelIndex currentIndex = q->indexFromConfigString(m_selectionModel->model(), m_pendingCurrent);
    if ( currentIndex.isValid() )
    {
      if ( m_treeView )
        m_treeView->setCurrentIndex(currentIndex);
      else
        m_selectionModel->setCurrentIndex(currentIndex, QItemSelectionModel::NoUpdate);
      m_pendingCurrent.clear();
    }
    QStringList::iterator it = m_pendingSelections.begin();
    for ( ; it != m_pendingSelections.end(); )
    {
      QModelIndex idx = q->indexFromConfigString( m_selectionModel->model(), *it);
      if ( idx.isValid() )
      {
        m_selectionModel->select( idx, QItemSelectionModel::Select );
        it = m_pendingSelections.erase( it );
      } else {
        ++it;
      }
    }
  }

  if ( m_treeView && m_treeView->model() )
  {
    QStringList::iterator it = m_pendingExpansions.begin();
    for ( ; it != m_pendingExpansions.end(); )
    {
      QModelIndex idx = q->indexFromConfigString( m_treeView->model(), *it);
      if ( idx.isValid() )
      {
        m_treeView->expand( idx );
        it = m_pendingExpansions.erase( it );
      } else {
        ++it;
      }
    }
  }

  if ( m_scrollArea )
  {
    Q_Q(KViewStateSaver);
    QTimer::singleShot( 0, q, SLOT( restoreScrollBarState() ) );
  }

  // watch the model for stuff coming in delayed
  if ( hasPendingChanges() )
  {
    const QAbstractItemModel *model =getModel();
    if ( model )
    {
      Q_Q(KViewStateSaver);
      q->connect( model, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
          SLOT( rowsInserted( const QModelIndex&, int, int ) ), Qt::QueuedConnection );
    }
  }
}

void KViewStateSaver::restoreState(const KConfigGroup& configGroup)
{
  Q_D(KViewStateSaver);
  d->m_pendingSelections = configGroup.readEntry( selectionKey, QStringList() );
  d->m_pendingExpansions = configGroup.readEntry( expansionKey, QStringList() );
  d->m_pendingCurrent = configGroup.readEntry( currentKey, QString() );
  d->m_horizontalScrollBarValue = configGroup.readEntry( scrollStateHorizontalKey, -1 );
  d->m_verticalScrollBarValue = configGroup.readEntry( scrollStateVerticalKey, -1 );

  d->restoreState();
}

QStringList KViewStateSaverPrivate::getExpandedItems(const QModelIndex &index)
{
  Q_Q(KViewStateSaver);

  QStringList expansion;
  for ( int i = 0; i < m_treeView->model()->rowCount( index ); ++i ) {
    const QModelIndex child = m_treeView->model()->index( i, 0, index );
    if ( m_treeView->isExpanded( child ) )
      expansion << q->indexToConfigString( child );
    if ( m_treeView->model()->hasChildren( child ) )
      expansion << getExpandedItems( child );
  }
  return expansion;
}

void KViewStateSaver::saveState(KConfigGroup& configGroup)
{
  Q_D(KViewStateSaver);

  if ( d->m_selectionModel )
  {
    QModelIndexList selectedIndexes = d->m_selectionModel->selectedRows();
    QModelIndex currentIndex = d->m_selectionModel->currentIndex();
    QStringList selection;
    foreach ( const QModelIndex &index, selectedIndexes )
      selection << indexToConfigString( index );
    configGroup.writeEntry( selectionKey, selection );
    configGroup.writeEntry( currentKey, indexToConfigString(currentIndex) );
  }

  if ( d->m_treeView )
  {
    QStringList expansion = d->getExpandedItems(QModelIndex());

    configGroup.writeEntry( expansionKey, expansion );
  }

  if ( d->m_scrollArea )
  {
    configGroup.writeEntry( scrollStateVerticalKey, d->m_scrollArea->verticalScrollBar()->value() );
    configGroup.writeEntry( scrollStateHorizontalKey, d->m_scrollArea->horizontalScrollBar()->value() );
  }
}

#include "kviewstatesaver.moc"

