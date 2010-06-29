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

#include "modeldumper.h"

#include <QAbstractItemModel>

ModelDumper::ModelDumper()
{

}

static int num;

void ModelDumper::dumpModel(QAbstractItemModel *model, QIODevice* device) const
{
  num = 1;

  device->write(dumpLevel(model, QModelIndex(), 1).toLatin1());
}

QString ModelDumper::dumpModel(QAbstractItemModel *model) const
{
  num = 1;
  return dumpLevel(model, QModelIndex(), 1);
}

QString ModelDumper::dumpLevel(QAbstractItemModel *model, const QModelIndex& parent, int level) const
{
  const int rowCount = model->rowCount(parent);
  QString lines;
  for (int row = 0; row < rowCount; ++row)
  {
    QString line;
    line.append("\"");
    for (int l = 0; l < level; ++l)
      line.append("- ");
    line.append(QString::number(num++));
    line.append("\"");
    line.append("\n");
    lines.append(line);
    static const int column = 0;
    const QModelIndex idx = model->index(row, column, parent);
    if (model->hasChildren(idx))
      lines.append(dumpLevel(model, idx, level + 1));
  }
  return lines;
}


