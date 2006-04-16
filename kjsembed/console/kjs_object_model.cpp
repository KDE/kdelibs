/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 KJSEmbed Authors
    See included AUTHORS file.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include <QPixmap>
#include <QDebug>

#include <kjs/interpreter.h>
#include <kjs/reference_list.h>

#include "kjs_object_model.h"

struct Node
{
    QByteArray name;
    KJS::JSObject *instance;
    Node *parent;
};

KJSObjectModel::KJSObjectModel(KJS::Interpreter *js, QObject *parent ):
    QAbstractItemModel(parent), m_js(js)
{
}

void KJSObjectModel::updateModel( KJS::JSObject *root)
{
    m_root = root;
    reset();
}

KJSObjectModel::~KJSObjectModel()
{
}

Qt::ItemFlags KJSObjectModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


int KJSObjectModel::rowCount(const QModelIndex &parent ) const
{
    KJS::ExecState *exec = m_js->globalExec();
    KJS::ReferenceList props;
    if (!parent.isValid())
        props = m_root->propList(exec);
    else
    {
        Node *item = static_cast<Node*>(parent.internalPointer());
        props = item->instance->propList(exec);
    }
    return props.length();
}

int KJSObjectModel::columnCount(const QModelIndex &parent ) const
{
    return 1;
}

QVariant KJSObjectModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if( section == 0)
            return "Object Name";
        else
            return "Value";
    }
    return QVariant();
}

QModelIndex KJSObjectModel::index(int row, int column, const QModelIndex &parent ) const
{
    KJS::JSObject *parentInstance = 0;
    Node *childItem = 0;
    KJS::ExecState *exec = m_js->globalExec();

    if (!parent.isValid())
    {
        if (m_root)
            parentInstance = m_root;
        else
            return QModelIndex();
    }
    else
        parentInstance = static_cast<Node*>(parent.internalPointer())->instance;
    int idx = 0;
    KJS::ReferenceList props = parentInstance->propList(exec);
    for( KJS::ReferenceListIterator ref = props.begin(); ref != props.end(); ref++)
    {
        if( idx == row)
        {
                childItem = new Node;
                childItem->name = ref->getPropertyName(exec).ascii();
                childItem->instance = parentInstance->get( exec,
                        childItem->name.constData() )->toObject(exec);
                childItem->parent = static_cast<Node*>(parent.internalPointer());
        }
        ++idx;
    }
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex KJSObjectModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        Node *node = new Node;
        node->instance = m_root;
        node->name = "Objects";
        node->parent = 0;
        return createIndex(0, index.column(), node);
    }

    Node *parentItem = static_cast<Node*>(index.internalPointer())->parent;
    if ( parentItem )
    {
        Node *node = new Node;
        node->instance = parentItem->instance;
        node->name = parentItem->name;
        node->parent = parentItem->parent;
        return createIndex(0, index.column(), node);
    }
    else
        return QModelIndex();
}

QVariant KJSObjectModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Node *item = static_cast<Node*>(index.internalPointer());
    KJS::JSObject *instance = item->instance;

    if (role == Qt::DecorationRole )
    {
        if( instance->implementsConstruct() )
            return QPixmap(":/images/class.png");
        else if( instance->implementsCall() )
            return QPixmap(":/images/method.png");
        else
            return QPixmap(":/images/property.png");
    }
    if( role == Qt::TextColorRole )
    {
        if( instance->implementsConstruct() )
            return QColor("blue");
        else if( instance->implementsCall() )
            return QColor("green");
        else
            return QColor("black");
    }
    if (role == Qt::DisplayRole)
        return item->name;
    return QVariant();
}

#include "kjs_object_model.moc"
