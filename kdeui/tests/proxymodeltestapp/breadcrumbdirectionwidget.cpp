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

#include "breadcrumbdirectionwidget.h"

#include <dynamictreemodel.h>
#include <kbreadcrumbselectionmodel.h>
#include <QBoxLayout>
#include <QSplitter>
#include <QTreeView>
#include <QEvent>

BreadcrumbDirectionWidget::BreadcrumbDirectionWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{

  DynamicTreeModel *rootModel = new DynamicTreeModel(this);

  ModelInsertCommand ins(rootModel);
  ins.setStartRow(0);
  ins.interpret(
    "- 1"
    "- 2"
    "- - 3"
    "- - 3"
    "- - - 4"
    "- - - 4"
    "- - - - 4"
    "- - 4"
    "- - 5"
    "- - - 4"
    "- - - - 4"
    "- - 5"
    "- 6"
    "- 7"
    "- - 8"
    "- - - 9"
    "- - - 10"
    "- - - - 9"
    "- - - - - 10"
    "- - - - - - 9"
    "- - - - - - 10"
    "- - - - - - - 9"
    "- - - - - - - - 10"
    "- - - - - - - - 9"
    "- - - - - - - 10"
    "- 20"
    "- 21");
  ins.doCommand();

  QHBoxLayout *layout = new QHBoxLayout(this);
  QSplitter *splitter1 = new QSplitter(Qt::Vertical, this);
  layout->addWidget(splitter1);
  QSplitter *splitter2 = new QSplitter(splitter1);
  QSplitter *splitter3 = new QSplitter(splitter1);

  QTreeView *view1 = new QTreeView(splitter2);
  view1->setModel(rootModel);
  view1->expandAll();
  view1->viewport()->setBackgroundRole(QPalette::Button);
  QTreeView *view2 = new QTreeView(splitter2);
  view2->setModel(rootModel);
  view2->expandAll();
  view2->viewport()->installEventFilter(this);
  QTreeView *view3 = new QTreeView(splitter3);
  view3->setModel(rootModel);
  view3->expandAll();
  QTreeView *view4 = new QTreeView(splitter3);
  view4->setModel(rootModel);
  view4->expandAll();
  view4->viewport()->installEventFilter(this);
  view4->viewport()->setBackgroundRole(QPalette::Button);

  KBreadcrumbSelectionModel *breadcrumbSelection1 = new KBreadcrumbSelectionModel(view2->selectionModel(), this);
  view1->setSelectionModel(breadcrumbSelection1);

  KBreadcrumbSelectionModel *breadcrumbSelection2 = new KBreadcrumbSelectionModel(view3->selectionModel(),
                                                                                  KBreadcrumbSelectionModel::MakeBreadcrumbSelectionInOther, this);
  view4->setSelectionModel(breadcrumbSelection2);
}

bool BreadcrumbDirectionWidget::eventFilter(QObject *o, QEvent *e )
{
  if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonDblClick || e->type() == QEvent::MouseButtonRelease)
    return true;
  return QObject::eventFilter( o, e);
}

