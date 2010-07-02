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

#ifndef MODEL_COMMANDER_H
#define MODEL_COMMANDER_H

#include "dynamictreemodel.h"

#include "proxymodeltestsuite_export.h"

class QAbstractItemModel;

#include <QStringList>

class PROXYMODELTESTSUITE_EXPORT ModelCommander : public QObject
{
  Q_OBJECT
public:
  explicit ModelCommander(DynamicTreeModel *model, QObject *parent);

  ModelChangeCommand* currentCommand();

public slots:
  void init_testInsertWhenEmpty(const QString &dataTag);
  void init_testInsertInRoot(const QString &dataTag);
  void init_testInsertInTopLevel(const QString &dataTag);
  void init_testInsertInSecondLevel(const QString &dataTag);

  void init_testRemoveFromRoot(const QString &dataTag);
  void init_testRemoveFromTopLevel(const QString &dataTag);
  void init_testRemoveFromSecondLevel(const QString &dataTag);

  void init_testMoveFromRoot(const QString &dataTag);
  void init_testMoveFromTopLevel(const QString &dataTag);
  void init_testMoveFromSecondLevel(const QString &dataTag);

  void init_testModifyInRoot(const QString &dataTag);
  void init_testModifyInTopLevel(const QString &dataTag);
  void init_testModifyInSecondLevel(const QString &dataTag);

  QStringList execute_testInsertWhenEmpty(const QString &dataTag);
  QStringList execute_testInsertInRoot(const QString &dataTag);
  QStringList execute_testInsertInTopLevel(const QString &dataTag);
  QStringList execute_testInsertInSecondLevel(const QString &dataTag);

  QStringList execute_testRemoveFromRoot(const QString &dataTag);
  QStringList execute_testRemoveFromTopLevel(const QString &dataTag);
  QStringList execute_testRemoveFromSecondLevel(const QString &dataTag);

  QStringList execute_testMoveFromRoot(const QString &dataTag);
  QStringList execute_testMoveFromTopLevel(const QString &dataTag);
  QStringList execute_testMoveFromSecondLevel(const QString &dataTag);

  QStringList execute_testModifyInRoot(const QString &dataTag);
  QStringList execute_testModifyInTopLevel(const QString &dataTag);
  QStringList execute_testModifyInSecondLevel(const QString &dataTag);

private:
  QStringList executeTestInsert(QList<int> rowAncestors, const QString &dataTag);
  QStringList executeTestRemove(QList<int> rowAncestors, const QString &dataTag);
  QStringList executeTestMove(QList<int> rowAncestors, const QString &dataTag);
  QStringList executeTestModify(QList<int> rowAncestors, const QString &dataTag);

  void initTestModel(const QString &dataTag);

  void execute(ModelChangeCommand *command);

private:
  int m_counter;
  DynamicTreeModel *m_model;
  ModelChangeCommand *m_currentCommand;
  QSet<QString> m_testsSeen;
};

#endif
