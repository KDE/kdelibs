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


#include "kbreadcrumbselectionmodel.h"

#include "kdebug.h"

class KBreadcrumbSelectionModelPrivate
{
  Q_DECLARE_PUBLIC(KBreadcrumbSelectionModel)
  KBreadcrumbSelectionModel * const q_ptr;
public:
  KBreadcrumbSelectionModelPrivate(KBreadcrumbSelectionModel *breadcrumbSelector, QItemSelectionModel *selectionModel, KBreadcrumbSelectionModel::Direction direction)
    : q_ptr(breadcrumbSelector),
      m_includeActualSelection(true),
      m_selectionDepth(-1),
      m_showHiddenAscendantData(false),
      m_selectionModel(selectionModel),
      m_direction(direction),
      m_ignoreCurrentChanged(false)
  {

  }

  void sourceSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

  bool m_includeActualSelection;
  int m_selectionDepth;
  bool m_showHiddenAscendantData;
  QItemSelectionModel *m_selectionModel;
  KBreadcrumbSelectionModel::Direction m_direction;
  bool m_ignoreCurrentChanged;
};

KBreadcrumbSelectionModel::KBreadcrumbSelectionModel(QItemSelectionModel *selectionModel, QObject* parent)
  : QItemSelectionModel(const_cast<QAbstractItemModel *>(selectionModel->model()), parent),
    d_ptr(new KBreadcrumbSelectionModelPrivate(this, selectionModel, Reverse))
{
}

KBreadcrumbSelectionModel::KBreadcrumbSelectionModel(QItemSelectionModel *selectionModel, Direction direction, QObject* parent)
  : QItemSelectionModel(const_cast<QAbstractItemModel *>(selectionModel->model()), parent),
    d_ptr(new KBreadcrumbSelectionModelPrivate(this, selectionModel, direction))
{
  if ( direction != Reverse)
    connect(selectionModel, SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
            this, SLOT(sourceSelectionChanged(const QItemSelection&,const QItemSelection&)));
}

KBreadcrumbSelectionModel::~KBreadcrumbSelectionModel()
{

}

bool KBreadcrumbSelectionModel::includeActualSelection() const
{
  Q_D(const KBreadcrumbSelectionModel);
  return d->m_includeActualSelection;
}

void KBreadcrumbSelectionModel::setIncludeActualSelection(bool includeActualSelection)
{
  Q_D(KBreadcrumbSelectionModel);
  d->m_includeActualSelection = includeActualSelection;
}

int KBreadcrumbSelectionModel::selectionDepth() const
{
  Q_D(const KBreadcrumbSelectionModel);
  return d->m_selectionDepth;
}

void KBreadcrumbSelectionModel::setSelectionDepth(int selectionDepth)
{
  Q_D(KBreadcrumbSelectionModel);
  d->m_selectionDepth = selectionDepth;
}

QItemSelection KBreadcrumbSelectionModel::getBreadcrumbSelection(const QModelIndex& index)
{
  Q_D(KBreadcrumbSelectionModel);
  QItemSelection breadcrumbSelection;

  if (d->m_includeActualSelection)
    breadcrumbSelection.append(QItemSelectionRange(index));

  QModelIndex parent = index.parent();
  int sumBreadcrumbs = 0;
  bool includeAll = d->m_selectionDepth < 0;
  while (parent.isValid() && (includeAll || sumBreadcrumbs < d->m_selectionDepth)) {
    breadcrumbSelection.append(QItemSelectionRange(parent));
    parent = parent.parent();
  }
  return breadcrumbSelection;
}

QItemSelection KBreadcrumbSelectionModel::getBreadcrumbSelection(const QItemSelection& selection)
{
  Q_D(KBreadcrumbSelectionModel);
  QItemSelection breadcrumbSelection;

  if (d->m_includeActualSelection)
    breadcrumbSelection = selection;

  QItemSelection::const_iterator it = selection.constBegin();
  const QItemSelection::const_iterator end = selection.constEnd();

  for ( ; it != end; ++it)
  {
    QModelIndex parent = it->parent();
    int sumBreadcrumbs = 0;
    bool includeAll = d->m_selectionDepth < 0;
    while (parent.isValid() && (includeAll || sumBreadcrumbs < d->m_selectionDepth))
    {
      breadcrumbSelection.append(QItemSelectionRange(parent));
      parent = parent.parent();
      ++sumBreadcrumbs;
    }
  }
  return breadcrumbSelection;
}

void KBreadcrumbSelectionModel::sourceSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  QItemSelection deselectedCrumbs = getBreadcrumbSelection(deselected);
  QItemSelection selectedCrumbs = getBreadcrumbSelection(selected);

  QItemSelection removed = deselectedCrumbs;
  foreach(const QItemSelectionRange &range, selectedCrumbs)
  {
    removed.removeAll(range);
  }

  QItemSelection added = selectedCrumbs;
  foreach(const QItemSelectionRange &range, deselectedCrumbs)
  {
    added.removeAll(range);
  }

  if (!removed.isEmpty())
  {
    QItemSelectionModel::select(removed, QItemSelectionModel::Deselect);
  }
  if (!added.isEmpty())
  {
    QItemSelectionModel::select(added, QItemSelectionModel::Select);
  }
}

void KBreadcrumbSelectionModel::select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
  Q_D(KBreadcrumbSelectionModel);
  // When an item is removed, the current index is set to the top index in the model.
  // That causes a selectionChanged signal with a selection which we do not want.
  if ( d->m_ignoreCurrentChanged )
  {
    d->m_ignoreCurrentChanged = false;
    return;
  }
  if ( d->m_direction == Forward )
  {
    d->m_selectionModel->select(getBreadcrumbSelection(index), command);
    QItemSelectionModel::select(index, command);
  } else {
    d->m_selectionModel->select(index, command);
    QItemSelectionModel::select(getBreadcrumbSelection(index), command);
  }
}

void KBreadcrumbSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
  Q_D(KBreadcrumbSelectionModel);
  QItemSelection bcc = getBreadcrumbSelection(selection);
  if ( d->m_direction == Forward )
  {
    d->m_selectionModel->select(selection, command);
    QItemSelectionModel::select(bcc, command);
  } else {
    d->m_selectionModel->select(bcc, command);
    QItemSelectionModel::select(selection, command);
  }
}

#include "kbreadcrumbselectionmodel.moc"
