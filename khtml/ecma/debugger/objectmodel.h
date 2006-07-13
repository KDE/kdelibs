#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

namespace KJS
{
    class Interpreter;
    class JSObject;
}

class ObjectNode
{
public:
    ObjectNode(const QByteArray &name, KJS::JSObject *instance, ObjectNode *parent = 0);
    ~ObjectNode();

    void appendChild(ObjectNode *child);

    ObjectNode *child(int row);
    int childCount() const;
    int columnCount() const;

    QByteArray name() const;
    KJS::JSObject *instance() const;

    int row() const;
    ObjectNode *parent();

private:
    QList<ObjectNode*> childItems;

    QByteArray itemName;
    KJS::JSObject *itemInstance;
    ObjectNode *parentItem;
};

class ObjectModel : public QAbstractItemModel
{
public:
    ObjectModel(KJS::Interpreter *interpreter, QObject *parent = 0);
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

private:
    void setupModelData(KJS::Interpreter *interpreter, ObjectNode *parent);

    ObjectNode *rootItem;
};



#endif
