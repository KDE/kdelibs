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

#include "modelcommander.h"
#include "dynamictreemodel.h"


ModelCommander::ModelCommander(DynamicTreeModel* model, QObject *parent)
  : QObject(parent), m_model(model)
{
}

void ModelCommander::execute_testInsertWhenEmpty(const QString &dataTag)
{
  ModelInsertCommand *ins = new ModelInsertCommand(m_model, this);
  ins->setStartRow(0);
  if (dataTag == "insert01")
  {
    // Insert a single item at the top.
    ins->setEndRow(0);
  } else if (dataTag == "insert02")
  {
    // Insert 10 items.
    ins->setEndRow(9);
  } else if (dataTag == "insert03")
  {
    // Insert 5 items, some of which are parents
    ins->interpret(
      " - 1"
      " - - 2"
      " - 3"
      " - 4"
      " - - 5"
      " - - - 6"
      " - 7"
      " - 8"
      " - - 9"
    );
  } else {
    qDebug() << dataTag;
    return;
  }
  execute(ins);
}

void ModelCommander::init_testInsertWhenEmpty(const QString &dataTag)
{
  Q_UNUSED(dataTag);
}

void ModelCommander::init_testInsertInRoot(const QString &dataTag)
{
  initTestInsert(dataTag);
}

void ModelCommander::init_testInsertInTopLevel(const QString &dataTag)
{
  initTestInsert(dataTag);
}

void ModelCommander::init_testInsertInSecondLevel(const QString &dataTag)
{
  initTestInsert(dataTag);
}

void ModelCommander::initTestInsert(const QString &dataTag)
{
  Q_UNUSED(dataTag);

  // A standard initial model for all these tests.
  ModelInsertCommand *ins = new ModelInsertCommand(m_model, this);
  ins->setStartRow(0);
  ins->interpret(
    " - 1"
    " - 2"
    " - - 3"
    " - - 4"
    " - - 5"
    " - 6"
    " - 7"
    " - 8"
    " - 9"
    " - - 10"
    " - - 11"
    " - - - 12"
    " - - - 13"
    " - - 14"
    " - - 15"
    " - - 16"
    " - - 17"
    " - - - 18"
    " - - - 19"
    " - - - - 20"
    " - - - - 21"
    " - - - 22"
    " - - - 23"
    " - - - 24"
    " - - - 25"
    " - - - - 26"
    " - - - - 27"
    " - - - 28"
    " - - - 29"
    " - - - - 30"
    " - - - 31"
    " - - - 32"
    " - - 33"
    " - - 34"
    " - - - 35"
    " - - 36"
    " - - 37"
    " - 38"
    " - 39"
    " - - 40"
    " - - 41"
    " - 42"
    " - 43"
  );
  execute(ins);
}

void ModelCommander::execute_testInsertInRoot(const QString &dataTag)
{
  executeTestInsert(QList<int>(), dataTag);
}

void ModelCommander::execute_testInsertInTopLevel(const QString &dataTag)
{
  executeTestInsert(QList<int>() << 5, dataTag);
}

void ModelCommander::execute_testInsertInSecondLevel(const QString &dataTag)
{
  executeTestInsert(QList<int>() << 5 << 5, dataTag);
}

void ModelCommander::execute(ModelChangeCommand* command)
{
  m_currentCommand = command;
  command->doCommand();
  delete command;
  command = 0;
}

ModelChangeCommand* ModelCommander::currentCommand()
{
  return m_currentCommand;
}

void ModelCommander::executeTestInsert(QList<int> rowAncestors, const QString &dataTag)
{
  ModelInsertCommand *ins = new ModelInsertCommand(m_model, this);
  if (dataTag == "insert01")
  {
    // Insert a single item at the top.
    ins->setAncestorRowNumbers(rowAncestors);
    ins->setStartRow(0);
    ins->setEndRow(0);
  } else if (dataTag == "insert02")
  {
    // Insert 10 items at the top.
    ins->setAncestorRowNumbers(rowAncestors);
    ins->setStartRow(0);
    ins->setEndRow(9);
  } else if (dataTag == "insert03")
  {
    // Insert a single item at the bottom.
    ins->setAncestorRowNumbers(rowAncestors);
    ins->setStartRow(m_model->rowCount());
    ins->setEndRow(m_model->rowCount());
  } else if (dataTag == "insert04")
  {
    // Insert a 10 items at the bottom.
    ins->setAncestorRowNumbers(rowAncestors);
    ins->setStartRow(m_model->rowCount());
    ins->setEndRow(m_model->rowCount() + 9);
  } else if (dataTag == "insert05")
  {
    // Insert a single item in the middle
    ins->setAncestorRowNumbers(rowAncestors);
    ins->setStartRow(4);
    ins->setEndRow(4);
  } else if (dataTag == "insert06")
  {
    // Insert 10 items in the middle
    ins->setAncestorRowNumbers(rowAncestors);
    ins->setStartRow(4);
    ins->setEndRow(13);
  } else if (dataTag == "insert07")
  {
    // Insert a single item at with children at the top
    ins->setAncestorRowNumbers(rowAncestors);
    ins->setStartRow(0);
    ins->interpret(
      " - 1"
      " - - 2"
      " - - - 3"
      " - - - - 4"
      " - - 5"
    );
  } else if (dataTag == "insert08")
  {
    // Insert a single item at with children at the bottom
    ins->setAncestorRowNumbers(rowAncestors);
    ins->setStartRow(m_model->rowCount());
    ins->interpret(
      " - 1"
      " - - 2"
      " - - - 3"
      " - - - - 4"
      " - - 5"
    );
  } else if (dataTag == "insert09")
  {
    // Insert a single item at with children in the middle
    ins->setAncestorRowNumbers(rowAncestors);
    ins->setStartRow(4);
    ins->interpret(
      " - 1"
      " - - 2"
      " - - - 3"
      " - - - - 4"
      " - - 5"
    );
  }
  else if (dataTag == "insert10")
  {
    // Insert 5 items, some of which are parents at the top
    ins->setAncestorRowNumbers(rowAncestors);
    ins->setStartRow(0);
    ins->interpret(
      " - 1"
      " - - 2"
      " - 3"
      " - 4"
      " - - 5"
      " - - 6"
      " - - - 7"
      " - 8"
      " - 9"
      " - - 10"
    );
  }
  else if (dataTag == "insert11")
  {
    // Insert 5 items, some of which are parents at the bottom
    ins->setAncestorRowNumbers(rowAncestors);
    ins->setStartRow(m_model->rowCount());
    ins->interpret(
      " - 1"
      " - - 2"
      " - 3"
      " - 4"
      " - - 5"
      " - - 6"
      " - - - 7"
      " - 8"
      " - 9"
      " - - 10"
    );
  }
  else if (dataTag == "insert12")
  {
    // Insert 5 items, some of which are parents in the middle
    ins->setAncestorRowNumbers(rowAncestors);
    ins->setStartRow(4);
    ins->interpret(
      " - 1"
      " - - 2"
      " - 3"
      " - 4"
      " - - 5"
      " - - 6"
      " - - - 7"
      " - 8"
      " - 9"
      " - - 10"
    );
  }
  execute(ins);
}
