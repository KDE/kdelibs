#include <kjs/object.h>
#include <kjs/interpreter.h>
#include <kjs/reference_list.h>
#include <kdebug.h>

#include "objectmodel.h"

ObjectNode::ObjectNode(const QString &name, KJS::JSObject *instance, ObjectNode *parent)
{
    itemName = name;
    itemInstance = instance;
    parentItem = parent;
}

ObjectNode::~ObjectNode()
{
    while (!childItems.isEmpty())
        delete childItems.takeFirst();
    childItems.clear();
}

void ObjectNode::appendChild(ObjectNode *item)
{
    childItems.append(item);
}

ObjectNode *ObjectNode::child(int row)
{
    return childItems.value(row);
}

int ObjectNode::childCount() const
{
    return childItems.count();
}

int ObjectNode::columnCount() const
{
    return 1;
//    return itemData.count();
}

QString ObjectNode::name() const
{
    return itemName;
}

KJS::JSObject *ObjectNode::instance() const
{
    return itemInstance;
}

ObjectNode *ObjectNode::parent()
{
    return parentItem;
}

int ObjectNode::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<ObjectNode*>(this));

    return 0;
}






void ObjectModel::update(KJS::Interpreter *interpreter)
{
    if (!interpreter)
        return;

    KJS::JSObject *instance = interpreter->globalObject();
    KJS::ExecState *exec = interpreter->globalExec();
    QString name = instance->toString(exec).qstring();
    rootItem = new ObjectNode(name, instance);

    setupModelData(interpreter, rootItem);
}


ObjectModel::ObjectModel(KJS::Interpreter *interpreter, QObject *parent)
    : QAbstractItemModel(parent)
{
    update(interpreter);
}

ObjectModel::~ObjectModel()
{
    delete rootItem;
}

int ObjectModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<ObjectNode*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant ObjectModel::data(const QModelIndex &index, int role) const
{
//     if (!index.isValid())
//         return QVariant();
// 
//     ObjectNode *item = static_cast<ObjectNode*>(index.internalPointer());
//     KJS::JSObject *instance = item->instance();
// 
//     if (role == Qt::DecorationRole )
//     {
//         if( instance->implementsConstruct() )
//             return QPixmap(":/images/class.png");
//         else if( instance->implementsCall() )
//             return QPixmap(":/images/method.png");
//         else
//             return QPixmap(":/images/property.png");
//     }
//     if( role == Qt::TextColorRole )
//     {
//         if( instance->implementsConstruct() )
//             return QColor("blue");
//         else if( instance->implementsCall() )
//             return QColor("green");
//         else
//             return QColor("black");
//     }
//     if (role == Qt::DisplayRole)
//         return "item";
// //        return item->name();
// 


    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    ObjectNode *item = static_cast<ObjectNode*>(index.internalPointer());
    return item->name();
}

Qt::ItemFlags ObjectModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ObjectModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QModelIndex ObjectModel::index(int row, int column, const QModelIndex &parent) const
{
    ObjectNode *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
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

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int ObjectModel::rowCount(const QModelIndex &parent) const
{
    ObjectNode *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<ObjectNode*>(parent.internalPointer());

    return parentItem->childCount();
}

void ObjectModel::setupModelData(KJS::Interpreter *interpreter, ObjectNode *parentNode)
{
    KJS::JSObject *parent = parentNode->instance();
    KJS::ExecState *exec = interpreter->globalExec();
    KJS::ReferenceList props = parent->propList(exec);
    for( KJS::ReferenceListIterator ref = props.begin(); ref != props.end(); ref++)
    {
        KJS::Identifier id = ref->getPropertyName(exec);
        QString name = id.qstring();
        KJS::JSObject* instance = parent->get(exec, id)->toObject(exec);

        kDebug() << "refrence list object: " << name << endl;
        ObjectNode *child = new ObjectNode(name, instance);
        parentNode->appendChild(child);

    }
}


















