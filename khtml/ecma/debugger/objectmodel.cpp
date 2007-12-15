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

#include "objectmodel.h"
#include "objectmodel.moc"

#include <kjs/context.h>
#include <kjs/object.h>
#include <kjs/interpreter.h>
#include <kjs/PropertyNameArray.h>

#include <kdebug.h>

namespace KJSDebugger {

//////////// Item
ObjectNode::ObjectNode(ObjectNode *parent)
{
    m_parent = parent;
}

ObjectNode::ObjectNode(const QList<QVariant> &data, ObjectNode *parent)
{
    m_parent = parent;
    m_data = data;
}

ObjectNode::~ObjectNode()
{
    qDeleteAll(m_children);
}

void ObjectNode::setData(const QList<QVariant> &data)
{
    m_data = data;
}

void ObjectNode::appendChild(ObjectNode *item)
{
    m_children.append(item);
}

ObjectNode *ObjectNode::child(int row)
{
    return m_children.value(row);
}

int ObjectNode::childCount() const
{
    return m_children.count();
}

int ObjectNode::columnCount() const
{
    return m_data.count();
}

QVariant ObjectNode::data(int column) const
{
    return m_data.value(column);
}

QList<QVariant> ObjectNode::data() const
{
    return m_data;
}

ObjectNode *ObjectNode::parent()
{
    return m_parent;
}

int ObjectNode::row() const
{
    if (m_parent)
        return m_parent->m_children.indexOf(const_cast<ObjectNode*>(this));

    return 0;
}



/////////// Model

ObjectModel::ObjectModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << "Reference" << "Type" << "Value";
    m_root = new ObjectNode(rootData);
}

ObjectModel::~ObjectModel()
{
    delete m_root;
}

int ObjectModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<ObjectNode*>(parent.internalPointer())->columnCount();
    else
        return m_root->columnCount();
}

QVariant ObjectModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    ObjectNode *item = static_cast<ObjectNode*>(index.internalPointer());
    return item->data(index.column());
}

Qt::ItemFlags ObjectModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ObjectModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_root->data(section);

    return QVariant();
}

QModelIndex ObjectModel::index(int row, int column, const QModelIndex &parent)
            const
{
    ObjectNode *parentItem;

    if (!parent.isValid())
        parentItem = m_root;
    else
        parentItem = static_cast<ObjectNode*>(parent.internalPointer());

    ObjectNode *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ObjectModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    ObjectNode *childItem = static_cast<ObjectNode*>(index.internalPointer());
    ObjectNode *parentItem = childItem->parent();

    if (parentItem == m_root)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int ObjectModel::rowCount(const QModelIndex &parent) const
{
    ObjectNode *parentItem;

    if (!parent.isValid())
        parentItem = m_root;
    else
        parentItem = static_cast<ObjectNode*>(parent.internalPointer());

    return parentItem->childCount();
}

void ObjectModel::update(KJS::ExecState *exec)
{
    KJS::Context* context = exec->context();
    if (!context)
    {
        kDebug() << "nothing running!";
        return;
    }

    KJS::ScopeChain chain = context->scopeChain();
    for( KJS::ScopeChainIterator obj = chain.begin();
         obj != chain.end();
         ++obj)
    {
        KJS::JSObject *object = (*obj);
        if (!object)
            break;

        if (object->isActivation())         // hack check to see if we're in local scope
        {
            if (m_root)
                delete m_root;

            QList<QVariant> rootData;
            rootData << "Reference" << "Type" << "Value";
            m_root = new ObjectNode(rootData);

            setupModelData(exec, object, m_root);
            return;
        }
    }
}

void ObjectModel::setupModelData(KJS::ExecState *exec, KJS::JSObject *scope, ObjectNode *parent)
{
//    QList<ObjectNode*> parents;
//    parents << parent;

    KJS::PropertyNameArray props;
    scope->getPropertyNames(exec, props);
    for(KJS::PropertyNameArrayIterator ref = props.begin();
        ref != props.end();
        ref++)
    {
        ObjectNode *node = new ObjectNode(parent);
        QList<QVariant> data;

        KJS::Identifier id = *ref;
        QString refName = id.qstring();
        KJS::JSValue *value = scope->get(exec, id);

        data << refName;

        // Should we check for these?
        // bool isUndefined () const
        // bool isNull () const
        // bool isUndefinedOrNull () const

        // First lets check if its a primitive type
        if (value->isBoolean())
        {
            data << "bool";
            data << value->toBoolean(exec);
        }
        else if (value->isNumber())
        {
            data << "number";
            data << value->toNumber(exec);
        }
        else if (value->isString())
        {
            data << "string";
            data << value->toString(exec).qstring();
        }
        else if (value->isObject())
        {
            data << "object";
            data << "[object data]";
            // then scroll through the object and add children
/*
            kDebug() << "Object!" << endl << endl << endl << endl << endl;
            KJS::JSObject *tmpObject = value->toObject(exec);
            if(tmpObject->implementsConstruct())
            {
                item->setIcon(QIcon(":/images/class.png"));
                item->setData(Qt::ForegroundRole, "blue");
            }
            else if(tmpObject->implementsCall())
            {
                item->setIcon(QIcon(":/images/method.png"));
                item->setData(Qt::ForegroundRole, "green");
            }
            else
            {
                item->setIcon(QIcon(":/images/property.png"));
                item->setData(Qt::ForegroundRole, "black");
            }
*/
        }

        node->setData(data);
        parent->appendChild(node);
    }
}

}
