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

#ifndef SCRIPTABLEREPARENTINGWIDGET_H
#define SCRIPTABLEREPARENTINGWIDGET_H

#include <QWidget>
#include <QScriptValue>

#include "kreparentingproxymodel.h"

class QComboBox;
class QTreeView;
class QPlainTextEdit;
class QScriptEngine;

class ScriptableReparentingProxyModel : public KReparentingProxyModel
{
  Q_OBJECT
public:
  ScriptableReparentingProxyModel(QObject* parent = 0);

  /* reimp */ bool isDescendantOf(const QModelIndex& ancestor, const QModelIndex& descendant) const;

  void setImplementation(const QString &implementation);

private:
  QScriptEngine *m_scriptEngine;
  mutable QScriptValue m_implementationFunction;
};

class ScriptableReparentingWidget : public QWidget
{
  Q_OBJECT
public:
  ScriptableReparentingWidget(QAbstractItemModel *rootModel, QWidget* parent = 0, Qt::WindowFlags f = 0);

private slots:
  void textChanged();
  void setExampleFunction(int index);

private:
  QComboBox *m_comboBox;
  ScriptableReparentingProxyModel *m_reparentingProxyModel;
  QTreeView *m_treeView;
  QPlainTextEdit *m_textEdit;
};


#endif // SCRIPTABLEREPARENTINGWIDGET_H