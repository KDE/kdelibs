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

#ifndef EXECSTATEMODEL_H
#define EXECSTATEMODEL_H

#include <QAbstractItemModel>
#include <QHash>
#include <kjs/JSType.h>

namespace KJS {
    class JSValue;
    class ExecState;
}

namespace KJSDebugger {

class Node
{
public:
    Node(const QString &name, KJS::JSValue *value, KJS::ExecState *exec, int row, Node *parent = 0);
    ~Node();
    Node *child(int i);
    Node *parent();
    KJS::JSValue *value() const;
    int row();
    QString name() const;

private:
    QString m_name;
    KJS::JSValue *m_value;
    KJS::ExecState *m_exec;
    QHash<int, Node*> m_children;
    Node *m_parent;
    int m_rowNumber;
};

class ExecStateModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ExecStateModel(KJS::ExecState *exec, QObject *parent = 0);
    ~ExecStateModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                        const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
    QVariant valueToVariant(KJS::JSValue*) const;

    KJS::ExecState *m_exec;
    Node *m_rootNode;
};

}

#endif


