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

#include <QtGui/QItemSelectionModel>


#include "kdebug.h"

class KBreadcrumbsDecoratorBasePrivate
{
  Q_DECLARE_PUBLIC(KBreadcrumbsDecoratorBase);
  KBreadcrumbsDecoratorBase * const q_ptr;
public:
  KBreadcrumbsDecoratorBasePrivate(KBreadcrumbsDecoratorBase *proxy)
    : q_ptr(proxy), m_includeActualSelection(true), m_selectionDepth(-1)
  {

  }

  void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

  bool m_includeActualSelection;
  int m_selectionDepth;
};

KBreadcrumbsDecoratorBase::KBreadcrumbsDecoratorBase()
  : d_ptr(new KBreadcrumbsDecoratorBasePrivate(this))
{

}

bool KBreadcrumbsDecoratorBase::includeActualSelection() const
{
  Q_D(const KBreadcrumbsDecoratorBase);
  return d->m_includeActualSelection;
}

void KBreadcrumbsDecoratorBase::setIncludeActualSelection(bool includeActualSelection)
{
  Q_D(KBreadcrumbsDecoratorBase);
  d->m_includeActualSelection = includeActualSelection;
}

int KBreadcrumbsDecoratorBase::selectionDepth() const
{
  Q_D(const KBreadcrumbsDecoratorBase);
  return d->m_selectionDepth;
}

void KBreadcrumbsDecoratorBase::setSelectionDepth(int selectionDepth)
{
  Q_D(KBreadcrumbsDecoratorBase);
  d->m_selectionDepth = selectionDepth;
}

QItemSelection KBreadcrumbsDecoratorBase::getBreadcrumbSelection(const QModelIndex& index)
{
  kDebug() << index;
  Q_D(KBreadcrumbsDecoratorBase);
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

QItemSelection KBreadcrumbsDecoratorBase::getBreadcrumbSelection(const QItemSelection& selection)
{
  Q_D(KBreadcrumbsDecoratorBase);
  QItemSelection breadcrumbSelection;

  if (d->m_includeActualSelection)
    breadcrumbSelection = selection;

  kDebug() << breadcrumbSelection;

  QItemSelection::const_iterator it = selection.constBegin();
  const QItemSelection::const_iterator end = selection.constEnd();

  for ( ; it != end; ++it)
  {
    QModelIndex parent = it->parent();
    int sumBreadcrumbs = 0;
    bool includeAll = d->m_selectionDepth < 0;
    while (parent.isValid() && (includeAll || sumBreadcrumbs < d->m_selectionDepth))
    {
      kDebug() << "inc bc" << parent << parent.data() << sumBreadcrumbs << d->m_selectionDepth;
      breadcrumbSelection.append(QItemSelectionRange(parent));
      parent = parent.parent();
      ++sumBreadcrumbs;
    }
  }
  return breadcrumbSelection;
}

#if 0
class KBreadcrumbsProxyModelPrivate
{
  Q_DECLARE_PUBLIC(KBreadcrumbsProxyModel);
  KBreadcrumbsProxyModel * const q_ptr;
public:
  KBreadcrumbsProxyModelPrivate(KBreadcrumbsProxyModel *proxy)
    : q_ptr(proxy)
  {

  }

  void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

  bool m_includeActualSelection;

};

KBreadcrumbsProxyModel::KBreadcrumbsProxyModel(QItemSelectionModel* selectionModel, QObject* parent)
  : KSelectionProxyModel(new ProxySelectionModel(selectionModel, selectionModel->model()), parent)
{
  setFilterBehavior(ExactSelection);
  connect(selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(selectionChanged(QItemSelection,QItemSelection)));
}

bool KBreadcrumbsProxyModel::includeActualSelection() const
{
  Q_D(const KBreadcrumbsProxyModel);
  return d->m_includeActualSelection;
}

void KBreadcrumbsProxyModel::setIncludeActualSelection(bool includeActualSelection)
{
  Q_D(KBreadcrumbsProxyModel);
  d->m_includeActualSelection = includeActualSelection;
}

void KBreadcrumbsProxyModelPrivate::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{

}
#endif

