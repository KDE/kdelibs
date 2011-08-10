/*
    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

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

#ifndef MODELCOMMANDERWIDGET_H
#define MODELCOMMANDERWIDGET_H

#include <QWidget>

class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;

class DynamicTreeModel;
class ModelCommander;

class ModelCommanderWidget : public QWidget
{
  Q_OBJECT
public:
  ModelCommanderWidget(DynamicTreeModel *dynamicTreeModel, QWidget* parent = 0, Qt::WindowFlags f = 0);

private slots:
  void initTest(QTreeWidgetItem *item);
  void executeTest(QTreeWidgetItem *item);
  void currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
  void executeCurrentTest();
  void resetCurrentTest();

private:
  void init();

private:
  DynamicTreeModel *m_dynamicTreeModel;
  ModelCommander *m_modelCommander;
  QTreeWidget *m_treeWidget;
  QPushButton *m_executeButton;
};

#endif
