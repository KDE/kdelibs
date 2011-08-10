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

#ifndef DYNAMICTREEWIDGET_H
#define DYNAMICTREEWIDGET_H

#include <QWidget>

#include "proxymodeltestsuite_export.h"

class QModelIndex;

class QComboBox;
class QPlainTextEdit;
class QTreeView;
class QRadioButton;

class DynamicTreeModel;

class PROXYMODELTESTSUITE_EXPORT DynamicTreeWidget : public QWidget
{
  Q_OBJECT
public:
  DynamicTreeWidget(DynamicTreeModel *rootModel, QWidget* parent = 0, Qt::WindowFlags f = 0);

  void setInitialTree(const QString &treeString);

  DynamicTreeModel *model() const { return m_dynamicTreeModel; }
  QTreeView *treeView() const { return m_treeView; }
  QPlainTextEdit *textEdit() const { return m_textEdit; }

private slots:
  void currentChanged(int index);
  void setTreePredefine(int index);
  void setInsertSubTreePredefine(int index);

  void removeSelected();
  void insertSelected();
  void resetModel();

private:
  void stringToModel(const QString &treeString);
  QString modelTreeToString(int depth, const QModelIndex &parent);

private:
  enum Tab {
    EditTab,
    ViewTab
  };

  QString m_initialString;
  DynamicTreeModel *m_dynamicTreeModel;
  QTreeView *m_treeView;
  QPlainTextEdit *m_textEdit;

  QPlainTextEdit *m_insertPatternTextEdit;
  QRadioButton *m_insertChildren;
  QRadioButton *m_insertSiblingsAbove;
  QRadioButton *m_insertSiblingsBelow;
  QComboBox *m_insertSubTreePredefines;
  QComboBox *m_treePredefines;
};

#endif
