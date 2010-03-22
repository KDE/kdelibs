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

#include "modelcommanderwidget.h"

#include <QTreeWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include "dynamictreemodel.h"
#include "modelcommander.h"
#include <QMetaMethod>

ModelCommanderWidget::ModelCommanderWidget(DynamicTreeModel *dynamicTreeModel, QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f),
    m_dynamicTreeModel(dynamicTreeModel),
    m_modelCommander(new ModelCommander(m_dynamicTreeModel, this)),
    m_treeWidget(new QTreeWidget),
    m_executeButton(new QPushButton("Execute"))
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addWidget(m_treeWidget);
  layout->addWidget(m_executeButton);

  init();

  connect(m_treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
                        SLOT(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));

  connect(m_executeButton, SIGNAL(clicked(bool)), SLOT(executeCurrentTest()));
}

void ModelCommanderWidget::init()
{
  const QMetaObject *mo = m_modelCommander->metaObject();
  QMetaMethod mm;
  for(int i = 0; i < mo->methodCount(); ++i)
  {
    mm = mo->method(i);
    QString signature = mm.signature();
    if (signature.startsWith("init_") && signature.endsWith("(QString)"))
    {
      QTreeWidgetItem *testFunctionItem = new QTreeWidgetItem(m_treeWidget, QStringList() << signature.mid(5, signature.length() - 14));
      m_treeWidget->addTopLevelItem(testFunctionItem);

      QStringList testData;
      QMetaObject::invokeMethod(m_modelCommander, "execute_" + testFunctionItem->text(0).toAscii(),
                                Q_RETURN_ARG(QStringList, testData),
                                Q_ARG(QString, QString()));

      foreach(const QString &testRun, testData)
        QTreeWidgetItem *testDataItem = new QTreeWidgetItem(testFunctionItem, QStringList() << testRun);
    }
  }
}

void ModelCommanderWidget::currentItemChanged( QTreeWidgetItem * current, QTreeWidgetItem * previous )
{
  Q_UNUSED(previous);
  initTest(current);
}

void ModelCommanderWidget::executeCurrentTest()
{
  executeTest(m_treeWidget->currentItem());

  disconnect(m_executeButton, SIGNAL(clicked(bool)), this, SLOT(executeCurrentTest()));
  m_executeButton->setText("Reset");
  connect(m_executeButton, SIGNAL(clicked(bool)), SLOT(resetCurrentTest()));
}

void ModelCommanderWidget::resetCurrentTest()
{
  initTest(m_treeWidget->currentItem());

  disconnect(m_executeButton, SIGNAL(clicked(bool)), this, SLOT(resetCurrentTest()));
  m_executeButton->setText("Execute");
  connect(m_executeButton, SIGNAL(clicked(bool)), SLOT(executeCurrentTest()));
}

void ModelCommanderWidget::initTest(QTreeWidgetItem *item)
{
  if (!item->parent())
    return; // m_dynamicTreeModel->clear();
  m_dynamicTreeModel->clear();
  bool success = QMetaObject::invokeMethod(m_modelCommander, "init_" + item->parent()->text(0).toAscii(),
                            Q_ARG(QString, item->text(0)));
  Q_ASSERT(success);
}

void ModelCommanderWidget::executeTest(QTreeWidgetItem *item)
{
  if (!item->parent())
    return;

  bool success = QMetaObject::invokeMethod(m_modelCommander, "execute_" + item->parent()->text(0).toAscii(),
                            Q_ARG(QString, item->text(0)));
  Q_ASSERT(success);
}

