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

#ifndef MODELSELECTOR_H
#define MODELSELECTOR_H

#include <QItemSelectionModel>

class ModelSpy;

class ModelSelector : public QObject
{
  Q_OBJECT
public:
  ModelSelector(ModelSpy *parent);

  void setWatchedModel(QAbstractItemModel *model);

  void setSelectionModel(QItemSelectionModel *selectionModel);

  QItemSelectionModel* selectionModel() const;

  void setWatch(bool watch);

  QModelIndex findNumber(const QModelIndex &start, int num);

  void deselectNumbers(QList<int> numbers);

  void selectSiliently( QSet<int> numbers );

  void processNumbers(QSet<int> numbers);

  void selectNumbers(QSet<int> numbers);

  virtual void makeSelections(const QString &testName);

public slots:
  void rowsInserted(const QModelIndex &parent, int start, int end);

private:
  QAbstractItemModel *m_model;
  QItemSelectionModel *m_selectionModel;
  ModelSpy *m_modelSpy;
  QSet<int> m_selectedNumbers;
  QSet<int> m_silentNumbers;
};

#endif

