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

#ifndef KVIEWSTATEMAINTAINER_H
#define KVIEWSTATEMAINTAINER_H

#include "kdeui_export.h"

#include <QtGui/QItemSelectionModel>
#include <QtGui/QAbstractItemView>

#include "kconfiggroup.h"
#include "ksharedconfig.h"

class KViewStateMaintainerBasePrivate;

class KDEUI_EXPORT KViewStateMaintainerBase : public QObject
{
  Q_OBJECT
public:
  KViewStateMaintainerBase(KConfigGroup configGroup, QObject* parent = 0);
  ~KViewStateMaintainerBase();

  void setSelectionModel(QItemSelectionModel *selectionModel);
  QItemSelectionModel *selectionModel() const;

  void setView(QAbstractItemView *view);
  QAbstractItemView *view() const;

  virtual void saveState() = 0;
  virtual void restoreState() = 0;

protected:
  KConfigGroup configGroup() const;

private:
  Q_DECLARE_PRIVATE(KViewStateMaintainerBase)
  KViewStateMaintainerBasePrivate * const d_ptr;

  Q_PRIVATE_SLOT( d_func(), void _k_modelAboutToBeReset() )
  Q_PRIVATE_SLOT( d_func(), void _k_modelReset() )
};

/**
 * @brief Encapsulates the maintenance of state between resets of QAbstractItemModel
 *
 * @code
 *   m_collectionViewStateMaintainer = new KViewStateMaintainer<Akonadi::ETMViewStateSaver>(KGlobal::config()->group("collectionView"));
 *   m_collectionViewStateMaintainer->setView(m_collectionView);
 *
 *   m_collectionCheckStateMaintainer = new KViewStateMaintainer<Akonadi::ETMViewStateSaver>(KGlobal::config()->group("collectionCheckState"));
 *   m_collectionCheckStateMaintainer->setSelectionModel(m_checkableProxy->selectionModel());
 * @endcode
 *
 * @see KViewStateSaver
 */
template<typename StateSaver>
class KViewStateMaintainer : public KViewStateMaintainerBase
{
  typedef StateSaver StateRestorer;
public:
  KViewStateMaintainer(KConfigGroup configGroup, QObject* parent = 0)
    : KViewStateMaintainerBase(configGroup, parent)
  {

  }

  /* reimp */ void saveState()
  {
    StateSaver saver;
    saver.setView(view());
    saver.setSelectionModel(selectionModel());
    KConfigGroup cfg = configGroup();
    saver.saveState(cfg);
    cfg.sync();
  }

  /* reimp */ void restoreState()
  {
    StateRestorer *restorer = new StateRestorer;
    restorer->setView(view());
    restorer->setSelectionModel(selectionModel());
    restorer->restoreState(configGroup());
  }

};

#endif
