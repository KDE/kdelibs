/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2006 Matt Broadstone (mbroadst@gmail.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

namespace KJS {
    class JSObject;
}

namespace KJSDebugger {

class ObjectNode
{
public:
    ObjectNode(ObjectNode *parent = 0);
    ObjectNode(const QList<QVariant> &data, ObjectNode *parent = 0);
    ~ObjectNode();

    void appendChild(ObjectNode *child);

    ObjectNode *child(int row);
    int childCount() const;
    int columnCount() const;
    void setData(const QList<QVariant> &data);
    QVariant data(int column) const;
    QList<QVariant> data() const;
    int row() const;
    ObjectNode *parent();

private:
    QList<ObjectNode*> m_children;
    QList<QVariant> m_data;
    ObjectNode *m_parent;
};

class ObjectModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ObjectModel(QObject *parent = 0);
    ~ObjectModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void update(KJS::ExecState *exec);

private:
    void setupModelData(KJS::ExecState*, KJS::JSObject*, ObjectNode*);
    ObjectNode *m_root;
};


}

#endif
