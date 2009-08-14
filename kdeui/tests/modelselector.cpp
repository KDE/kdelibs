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


#include "modelselector.h"

#include "proxymodeltest.h"


ModelSelector::ModelSelector(ModelSpy* parent)
    : QObject(parent), m_modelSpy(parent)
{

}

void ModelSelector::setWatchedModel(QAbstractItemModel* model)
{
  m_model = model;
}

void ModelSelector::setSelectionModel(QItemSelectionModel* selectionModel)
{
  m_selectionModel = selectionModel;
}

QItemSelectionModel* ModelSelector::selectionModel() const
{
  return m_selectionModel;
}

void ModelSelector::setWatch(bool watch)
{
  Q_ASSERT(m_model);
  if (watch)
  {
    disconnect(m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
               this, SLOT(rowsInserted(const QModelIndex &, int, int)));
    connect(m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            SLOT(rowsInserted(const QModelIndex &, int, int)));
  }
  else
  {
    disconnect(m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
               this, SLOT(rowsInserted(const QModelIndex &, int, int)));
  }
}

QModelIndex ModelSelector::findNumber(const QModelIndex& start, int num)
{
  QModelIndex idx;
  QModelIndexList list = m_model->match(start, Qt::DisplayRole, num, 1, Qt::MatchExactly);
  if (list.size() > 0)
  {
    return list.at(0);
  }
  for ( int row = 0; row < m_model->rowCount( start ); ++row )
  {
    idx = findNumber(m_model->index(row, 0, start), num );
    if (idx.isValid())
      return idx;
  }

  return QModelIndex();
}

void ModelSelector::deselectNumbers(QList< int > numbers)
{
  QModelIndex idx;
  foreach (int num, numbers)
  {
    idx = findNumber(QModelIndex(), num);
    if (!idx.isValid())
      continue;

    // TODO: Group these into a QItemSelection where possible.
    m_selectionModel->select(idx, QItemSelectionModel::Deselect);
  }
}

void ModelSelector::selectSiliently(QSet< int > numbers)
{
  m_silentNumbers.unite( numbers );
  m_selectedNumbers.subtract( numbers );
  processNumbers( numbers );
}

void ModelSelector::processNumbers(QSet< int > numbers)
{
  QModelIndex idx;
  foreach (int num, numbers)
  {
    idx = findNumber(QModelIndex(), num);
    if (!idx.isValid())
      continue;

    bool spyingState = m_modelSpy->isSpying();
    // TODO: Group these into a QItemSelection where possible.
    if (m_silentNumbers.contains(num))
      m_modelSpy->stopSpying();
    m_selectionModel->select(idx, QItemSelectionModel::SelectCurrent);
    if (m_silentNumbers.contains(num) && spyingState)
      m_modelSpy->startSpying();
  }
}

void ModelSelector::selectNumbers(QSet< int > numbers)
{
  m_selectedNumbers.unite(numbers);
  m_silentNumbers.subtract(numbers);
  processNumbers( numbers );
}

void ModelSelector::rowsInserted(const QModelIndex& parent, int start, int end)
{
  int row = start;
  const int column = 0;
  QModelIndex idx = m_model->index(row, column, parent);

  while (idx.isValid() && row <= end)
  {
    int item = idx.data().toInt();
    if (m_silentNumbers.contains( item ) )
    {
      bool spyingState = m_modelSpy->isSpying();
      m_modelSpy->stopSpying();
      m_selectionModel->select(idx, QItemSelectionModel::SelectCurrent);
      if (spyingState)
        m_modelSpy->startSpying();
    } else if (m_selectedNumbers.contains( item ) )
    {
      m_selectionModel->select(idx, QItemSelectionModel::SelectCurrent);
    }
    idx = idx.sibling(++row, column);
  }
}

void ModelSelector::makeSelections(const QString &testName)
{
  if ( testName == "insert01" )
  {
    selectNumbers( QSet<int>() << 1 );
  } else if ( testName == "insert02" )
  {
    selectSiliently( QSet<int>() << 1 );
  } else if ( testName == "insert03" )
  {
    selectSiliently( QSet<int>() << 1 );
  } else if ( testName == "insert04" )
  {
    selectSiliently( QSet<int>() << 1 );
  } else if ( testName == "insert05" )
  {
    selectSiliently( QSet<int>() << 1 );
  } else if ( testName == "insert06" )
  {
    selectSiliently( QSet<int>() << 1 );
  } else if ( testName == "insert07" )
  {
    selectSiliently( QSet<int>() << 1 );
  } else if ( testName == "insert08" )
  {
    selectSiliently( QSet<int>() << 1 );
  } else if ( testName == "change01" )
  {
    selectSiliently( QSet<int>() << 1 );
  }

}




