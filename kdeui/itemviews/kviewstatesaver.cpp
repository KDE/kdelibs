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
    : q_ptr(qq),
      m_treeView(0),
      m_view(0),
      m_selectionModel(0),
      m_scrollArea(0),
      m_horizontalScrollBarValue(-1),
      m_verticalScrollBarValue(-1)
  {

  }

  Q_DECLARE_PUBLIC(KViewStateSaver)
  KViewStateSaver * const q_ptr;

  QStringList getExpandedItems(const QModelIndex &index) const;

  void listenToPendingChanges();
  void processPendingChanges();

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

  void restoreSelection();
  void restoreCurrentItem();
  void restoreExpanded();

  inline bool hasPendingChanges() const
  {
    return !m_pendingCurrent.isEmpty() || !m_pendingExpansions.isEmpty() || !m_pendingSelections.isEmpty();
  }

  const QAbstractItemModel* getModel()
  {
    if ( m_selectionModel && m_selectionModel->model() )
      return m_selectionModel->model();
    else if ( m_view && m_view->model() )
      return m_view->model();
    return 0;
  }

  void rowsInserted( const QModelIndex &index, int start, int end )
  {
    Q_Q(KViewStateSaver);
    processPendingChanges();

    if ( !hasPendingChanges() )
    {
      q->disconnect( getModel(), SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
          q, SLOT( rowsInserted( const QModelIndex&, int, int ) ) );
      q->deleteLater();
    }
  }

  QTreeView *m_treeView;
  QAbstractItemView *m_view;
  QItemSelectionModel *m_selectionModel;
  QAbstractScrollArea *m_scrollArea;

  int m_horizontalScrollBarValue;
  int m_verticalScrollBarValue;
  QSet<QString> m_pendingSelections;
  QSet<QString> m_pendingExpansions;
  QString m_pendingCurrent;
};

KViewStateSaver::KViewStateSaver(QObject* parent)
  : QObject(parent), d_ptr( new KViewStateSaverPrivate(this) )
{
  qRegisterMetaType<QModelIndex>( "QModelIndex" );
}

KViewStateSaver::~KViewStateSaver()
{
  delete d_ptr;
}

void KViewStateSaver::setView(QAbstractItemView* view)
{
  Q_D(KViewStateSaver);
  d->m_scrollArea = view;
  d->m_selectionModel = view->selectionModel();
  d->m_view = view;
  d->m_treeView = qobject_cast<QTreeView*>(view);
}

QAbstractItemView* KViewStateSaver::view() const
{
  Q_D(const KViewStateSaver);
  return d->m_view;
}

QItemSelectionModel* KViewStateSaver::selectionModel() const
{
  Q_D(const KViewStateSaver);
  return d->m_selectionModel;
}

void KViewStateSaver::setSelectionModel(QItemSelectionModel* selectionModel)
{
  Q_D(KViewStateSaver);
  d->m_selectionModel = selectionModel;
}

void KViewStateSaverPrivate::listenToPendingChanges()
{
  Q_Q(KViewStateSaver);
  // watch the model for stuff coming in delayed
  if ( hasPendingChanges() )
  {
    const QAbstractItemModel *model = getModel();
    if ( model )
    {
      q->disconnect( model, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
          q, SLOT( rowsInserted( const QModelIndex&, int, int ) ) );
      q->connect( model, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
          SLOT( rowsInserted( const QModelIndex&, int, int ) ) );
      return;
    } else {
      q->deleteLater();
    }
  } else {
    q->deleteLater();
  }
}

void KViewStateSaverPrivate::processPendingChanges()
{
  Q_Q(KViewStateSaver);

  q->restoreCurrentItem(m_pendingCurrent);
  q->restoreSelection(m_pendingSelections.toList());
  q->restoreExpanded(m_pendingExpansions.toList());
  q->restoreScrollState(m_verticalScrollBarValue, m_horizontalScrollBarValue);
}

void KViewStateSaver::restoreState(const KConfigGroup& configGroup)
{
  Q_D(KViewStateSaver);

  // Delete myself if not finished after two seconds.
  QTimer::singleShot(2000, this, SLOT(deleteLater()));


  d->m_pendingCurrent = configGroup.readEntry( currentKey, QString() );
  d->m_pendingSelections = configGroup.readEntry( selectionKey, QStringList() ).toSet();
  d->m_pendingExpansions = configGroup.readEntry( expansionKey, QStringList() ).toSet();
  d->m_horizontalScrollBarValue = configGroup.readEntry( scrollStateHorizontalKey, -1 );
  d->m_verticalScrollBarValue = configGroup.readEntry( scrollStateVerticalKey, -1 );

  d->processPendingChanges();
  if (d->hasPendingChanges())
    d->listenToPendingChanges();
}

QStringList KViewStateSaverPrivate::getExpandedItems(const QModelIndex &index) const
{
  Q_Q(const KViewStateSaver);

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
    configGroup.writeEntry( selectionKey, selectionKeys() );
    configGroup.writeEntry( currentKey, currentIndexKey() );
  }

  if ( d->m_treeView )
  {
    QStringList expansion = expansionKeys();

    configGroup.writeEntry( expansionKey, expansion );
  }

  if ( d->m_scrollArea )
  {
    QPair<int, int> _scrollState = scrollState();
    configGroup.writeEntry( scrollStateVerticalKey, _scrollState.first );
    configGroup.writeEntry( scrollStateHorizontalKey, _scrollState.second );
  }
}

void KViewStateSaverPrivate::restoreCurrentItem()
{
  Q_Q(KViewStateSaver);

  QModelIndex currentIndex = q->indexFromConfigString(m_selectionModel->model(), m_pendingCurrent);
  if ( currentIndex.isValid() )
  {
    if (m_treeView)
      m_treeView->setCurrentIndex(currentIndex);
    else
      m_selectionModel->setCurrentIndex(currentIndex, QItemSelectionModel::NoUpdate);
    m_pendingCurrent.clear();
  }
}

void KViewStateSaver::restoreCurrentItem(const QString& indexString)
{
  Q_D(KViewStateSaver);
  if (!d->m_selectionModel || !d->m_selectionModel->model())
      return;

  if (indexString.isEmpty())
  {
    return;
  }
  d->m_pendingCurrent = indexString;
  d->restoreCurrentItem();

  if (d->hasPendingChanges())
    d->listenToPendingChanges();
}

void KViewStateSaverPrivate::restoreExpanded()
{
  Q_Q(KViewStateSaver);

  QSet<QString>::iterator it = m_pendingExpansions.begin();
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

void KViewStateSaver::restoreExpanded(const QStringList& indexStrings)
{
  Q_D(KViewStateSaver);
  if (!d->m_treeView || !d->m_treeView->model())
      return;

  if (indexStrings.isEmpty())
    return;

  d->m_pendingExpansions.unite(indexStrings.toSet());
  d->restoreExpanded();
  if (d->hasPendingChanges())
    d->listenToPendingChanges();
}

void KViewStateSaver::restoreScrollState(int verticalScoll, int horizontalScroll)
{
  Q_D(KViewStateSaver);

  if ( !d->m_scrollArea )
    return;

  d->m_verticalScrollBarValue = verticalScoll;
  d->m_horizontalScrollBarValue = horizontalScroll;

  QTimer::singleShot( 0, this, SLOT( restoreScrollBarState() ) );
}

void KViewStateSaverPrivate::restoreSelection()
{
  Q_Q(KViewStateSaver);

  QSet<QString>::iterator it = m_pendingSelections.begin();
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

void KViewStateSaver::restoreSelection(const QStringList& indexStrings)
{
  Q_D(KViewStateSaver);

  if (!d->m_selectionModel || !d->m_selectionModel->model())
      return;

  if (indexStrings.isEmpty())
    return;

  d->m_pendingSelections.unite(indexStrings.toSet());
  d->restoreSelection();
  if (d->hasPendingChanges())
    d->listenToPendingChanges();
}

QString KViewStateSaver::currentIndexKey() const
{
  Q_D(const KViewStateSaver);
  if (!d->m_selectionModel)
      return QString();
  return indexToConfigString(d->m_selectionModel->currentIndex());
}

QStringList KViewStateSaver::expansionKeys() const
{
  Q_D(const KViewStateSaver);
  if (!d->m_treeView || !d->m_treeView->model())
      return QStringList();

  return d->getExpandedItems(QModelIndex());
}

QStringList KViewStateSaver::selectionKeys() const
{
  Q_D(const KViewStateSaver);
  if (!d->m_selectionModel)
      return QStringList();

  QModelIndexList selectedIndexes = d->m_selectionModel->selectedRows();
  QStringList selection;
  foreach ( const QModelIndex &index, selectedIndexes )
    selection << indexToConfigString( index );

  return selection;
}

QPair<int, int> KViewStateSaver::scrollState() const
{
  Q_D(const KViewStateSaver);
  return qMakePair(d->m_scrollArea->verticalScrollBar()->value(), d->m_scrollArea->horizontalScrollBar()->value());
}

#include "kviewstatesaver.moc"

