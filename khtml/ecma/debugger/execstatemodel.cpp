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

#include "execstatemodel.h"
#include "execstatemodel.moc"

#include <kdebug.h>
#include <klocale.h>

#include <kjs/context.h>
#include <kjs/value.h>
#include <kjs/object.h>
#include <kjs/ExecState.h>
#include <kjs/PropertyNameArray.h>

namespace KJSDebugger {

// NODE
Node::Node(const QString &name, KJS::JSValue *value, KJS::ExecState *exec, int row, Node *parent)
{
    m_name = name;
    m_value = value;
    m_exec = exec;
    // Record the item's location within its parent.
    m_rowNumber = row;
    m_parent = parent;
}

Node::~Node()
{
    QHash<int, Node*>::iterator it;
    for (it = m_children.begin(); it != m_children.end(); ++it)
        delete it.value();
}

KJS::JSValue *Node::value() const
{
    return m_value;
}

QString Node::name() const
{
    return m_name;
}

Node *Node::parent()
{
    return m_parent;
}

Node *Node::child(int i)
{
    if (m_children.contains(i))
        return m_children[i];

    if (!m_value->isObject())
        return 0;

    if (!m_exec)
        return 0;

    KJS::JSObject *object = m_value->toObject(m_exec);
    KJS::PropertyNameArray props;
    object->getPropertyNames(m_exec, props);

    if (i >= 0 && i < props.size())
    {
        KJS::Identifier id = props[i];
        QString name = id.qstring();
        KJS::JSValue *childValue = object->get(m_exec, id);

        Node *childItem = new Node(name, childValue, m_exec, i, this);
        m_children[i] = childItem;
        return childItem;
    }

    return 0;
}

int Node::row()
{
    return m_rowNumber;
}


// MODEL
ExecStateModel::ExecStateModel(KJS::ExecState *exec, QObject *parent)
    : QAbstractItemModel(parent),
      m_exec(exec),
      m_rootNode(0)
{
    if (!m_exec)
    {
        kDebug() << "exec is null!";
        return;
    }

    KJS::Context* context = m_exec->context();
    if (!context)
    {
        kDebug() << "nothing running!";
        return;
    }

    KJS::JSObject *activationObject = 0;
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
            activationObject = object;
            break;
        }
    }

    if (activationObject)
    {
//        QString name = activationObject->toString(m_exec).qstring();
        m_rootNode = new Node("Activation", activationObject, m_exec, 0);
    }
}

ExecStateModel::~ExecStateModel()
{
    delete m_rootNode;
}

int ExecStateModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 3;
}

QString ExecStateModel::typeToString(KJS::JSType type) const
{
    switch(type)
    {
        case 0:
            return "UnspecifiedType";
        case 1:
            return "NumberType";
        case 2:
            return "BooleanType";
        case 3:
            return "UndefinedType";
        case 4:
            return "NullType";
        case 5:
            return "StringType";
        case 6:
            return "ObjectType";
        case 7:
            return "GetterSetterType";
        default:
            return QString();
    }
}

QVariant ExecStateModel::valueToVariant(KJS::JSValue *value) const
{
    switch(value->type())
    {
        case KJS::NumberType:
            return value->toNumber(m_exec);
        case KJS::BooleanType:
            return value->toBoolean(m_exec);
        case KJS::StringType:
            return value->toString(m_exec).qstring();
        case KJS::UnspecifiedType:
        case KJS::UndefinedType:
        case KJS::NullType:
        case KJS::ObjectType:
        case KJS::GetterSetterType:
        default:
            return QVariant();
    }
}

QVariant ExecStateModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    Node *item = static_cast<Node*>(index.internalPointer());

    KJS::JSValue *value = item->value();
    switch (index.column())
    {
        case 0:
            return item->name();
        case 1:
            return typeToString(value->type());
        case 2:
            return valueToVariant(value);
        default:
            return QVariant();
    }
}

Qt::ItemFlags ExecStateModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ExecStateModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
            case 0:
                return i18n("Reference");
            case 1:
                return i18n("Type");
            case 2:
                return i18n("Value");
            default:
                return QVariant();
        }
    }

    return QVariant();
}

QModelIndex ExecStateModel::index(int row, int column, const QModelIndex &parent)
            const
{
    Node *parentNode;

    if (!parent.isValid()) 
    {
        parentNode = m_rootNode;
        if (!m_rootNode)
            return QModelIndex();
    }
    else
    {
        parentNode = static_cast<Node*>(parent.internalPointer());
    }


    Node *childNode = parentNode->child(row);
    if (childNode)
        return createIndex(row, column, childNode);
    else
        return QModelIndex();
}

QModelIndex ExecStateModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    Node *childNode = static_cast<Node*>(child.internalPointer());
    Node *parentNode = childNode->parent();

    if (!parentNode || parentNode == m_rootNode)
        return QModelIndex();

    return createIndex(parentNode->row(), 0, parentNode);
}

int ExecStateModel::rowCount(const QModelIndex &parent) const
{
    Node *parentNode;

    if (!parent.isValid())
        parentNode = m_rootNode;
    else
        parentNode = static_cast<Node*>(parent.internalPointer());

    if (!parentNode)
        return 0;

    KJS::JSValue *value = parentNode->value();
    if (!value->isObject())
        return 0;

    KJS::JSObject *object = value->toObject(m_exec);
    KJS::PropertyNameArray props;
    object->getPropertyNames(m_exec, props);
    return props.size();
}


}
