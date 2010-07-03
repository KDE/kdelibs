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

#include "statesaverwidget.h"

#include <QSplitter>
#include <QBoxLayout>
#include <QTreeView>
#include <QApplication>

#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "dynamictreewidget.h"
#include "dynamictreemodel.h"

QModelIndex DynamicTreeStateSaver::indexFromConfigString(const QAbstractItemModel* model, const QString& key) const
{
  QModelIndexList list = model->match(model->index(0, 0), DynamicTreeModel::DynamicTreeModelId, key.toInt(), 1, Qt::MatchRecursive);
  if (list.isEmpty())
    return QModelIndex();
  return list.first();
}

QString DynamicTreeStateSaver::indexToConfigString(const QModelIndex& index) const
{
  return index.data(DynamicTreeModel::DynamicTreeModelId).toString();
}

DynamicTreeStateSaver::DynamicTreeStateSaver(QObject* parent)
  : KViewStateSaver(parent)
{
}

StateSaverWidget::StateSaverWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  QSplitter *splitter = new QSplitter(this);
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->addWidget(splitter);

  DynamicTreeModel *model = new DynamicTreeModel(this);

  DynamicTreeWidget *dynamicTreeWidget = new DynamicTreeWidget(model, splitter);

  m_view = new QTreeView(splitter);
  m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_view->setModel(model);

  connect( model, SIGNAL(modelAboutToBeReset()), SLOT(saveState()) );
  connect( model, SIGNAL(modelReset()), SLOT(restoreState()) );
  connect( qApp, SIGNAL(aboutToQuit()), SLOT(saveState()) );

  restoreState();
}

StateSaverWidget::~StateSaverWidget()
{
  saveState();
}

void StateSaverWidget::saveState()
{
  DynamicTreeStateSaver saver;
  saver.setView(m_view);

  KConfigGroup cfg( KGlobal::config(), "ExampleViewState" );
  saver.saveState( cfg );
  cfg.sync();
}

void StateSaverWidget::restoreState()
{
  DynamicTreeStateSaver *saver = new DynamicTreeStateSaver;
  saver->setView(m_view);
  KConfigGroup cfg( KGlobal::config(), "ExampleViewState" );
  saver->restoreState( cfg );
}




