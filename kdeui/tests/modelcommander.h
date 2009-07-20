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

class QAbstractItemModel;

#include <QObject>

class ModelCommander : public QObject
{
  Q_OBJECT
public:
  explicit ModelCommander(DynamicTreeModel *model, QObject *parent);

  void setCommands(QList<QPair<QString, ModelChangeCommandList> > commands);
  void setDefaultCommands();

  void executeUntil(const QString &stopBefore = QString());
  bool hasNextCommand();
  void executeNextCommand();

protected:
  QPair<QString, ModelChangeCommandList> nextCommand();

  void setCommand(const QString &name, ModelChangeCommandList list);

private:
  DynamicTreeModel *m_model;
  QList<QPair<QString, ModelChangeCommandList> > m_commands;
  int m_counter;

};

#endif
