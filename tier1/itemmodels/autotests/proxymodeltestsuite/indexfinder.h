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

#ifndef INDEXFINDER_H
#define INDEXFINDER_H

#include <QModelIndex>

class IndexFinder
{
  public:
    IndexFinder(QList<int> rows = QList<int>()) : m_rows(rows), m_model(0) {}

    IndexFinder(const QAbstractItemModel *model, QList<int> rows = QList<int>() )
      :  m_rows(rows), m_model(model)
    {
      Q_ASSERT(model);
    }

    QModelIndex getIndex() const
    {
      if(!m_model)
        return QModelIndex();
      static const int col = 0;
      QModelIndex parent = QModelIndex();
      QListIterator<int> i(m_rows);
      while (i.hasNext())
      {
        parent = m_model->index(i.next(), col, parent);
        Q_ASSERT(parent.isValid());
      }
      return parent;
    }

    static IndexFinder indexToIndexFinder(const QModelIndex &_idx)
    {
      if (!_idx.isValid())
        return IndexFinder();

      QList<int> list;
      QModelIndex idx = _idx;
      while (idx.isValid())
      {
        list.prepend(idx.row());
        idx = idx.parent();
      }
      return IndexFinder(_idx.model(), list);
    }

    bool operator==( const IndexFinder &other ) const
    {
      return (m_rows == other.m_rows && m_model == other.m_model );
    }

    QList<int> rows() const { return m_rows; }
    void appendRow(int row) { m_rows.append(row); }
    void setRows( const QList<int> &rows ) { m_rows = rows; }
    void setModel(QAbstractItemModel *model) { m_model = model; }

  private:
    QList<int> m_rows;
    const QAbstractItemModel * m_model;
};


Q_DECLARE_METATYPE( IndexFinder )

#endif
