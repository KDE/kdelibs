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

#ifndef SELECTIONPROXYMODELTEST_H
#define SELECTIONPROXYMODELTEST_H

#include "proxymodeltest.h"

#include "kselectionproxymodel.h"
#include "modelselector.h"

class SelectionProxyModelTest : public ProxyModelTest
{
  Q_OBJECT
public:
  SelectionProxyModelTest(QObject *parent = 0)
      : ProxyModelTest( parent ),
      m_selectionModel(0)
  {
  }


protected:
  virtual void testData() = 0;

  virtual void makeSelections()
  {
    m_selMaker->setWatch(true);

    QString currentTag = QTest::currentDataTag();

    m_selMaker->makeSelections(currentTag);
  }

  void doInit()
  {
    ProxyModelTest::doInit();
    makeSelections();
  }

protected:
  QItemSelectionModel *m_selectionModel;
  KSelectionProxyModel *m_proxyModel;
  ModelSelector *m_selMaker;
};

#endif
