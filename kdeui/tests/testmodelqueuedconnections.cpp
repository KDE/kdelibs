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

#include <QObject>
#include <QtTest>
#include <QtCore>
#include <qtest_kde.h>
#include <qtestevent.h>

#include <QEventLoop>

#include "dynamictreemodel.h"

class ModelQueuedConnectionsTest : public QObject
{
  Q_OBJECT
public slots:
  void rowsInserted(const QModelIndex &parent, int start, int end)
  {
    QModelIndex idx;
    const int column = 0;
    for (int row = start; row <= end; ++row)
    {
      idx = m_rootModel->index(row, column, parent);
      qDebug() << idx << idx.data();
      QEXPECT_FAIL("", "Can't use Queued connections with models", Continue);
      QVERIFY(idx.isValid());
    }

    m_eventLoop->exit();

  }

private slots:

  void initTestCase()
  {
    m_rootModel = new DynamicTreeModel(this);
    m_eventLoop = new QEventLoop(this);
  }

  void testInsertAndRemove()
  {
    ModelInsertAndRemoveQueuedCommand *insAndRem = new ModelInsertAndRemoveQueuedCommand(m_rootModel, this);

    connect (m_rootModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
             SLOT(rowsInserted(const QModelIndex &, int, int)));

    insAndRem->setStartRow(0);
    insAndRem->setEndRow(9);
    insAndRem->doCommand();

    m_eventLoop->exec();

  }

private:
  DynamicTreeModel *m_rootModel;
  QEventLoop *m_eventLoop;

};


QTEST_KDEMAIN(ModelQueuedConnectionsTest, GUI)
#include "testmodelqueuedconnections.moc"

