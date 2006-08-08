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
    ObjectNode(ObjectNode *parent = 0);
    ObjectNode(const QList<QVariant> &data, ObjectNode *parent = 0);
    ~ObjectNode();

    void appendChild(ObjectNode *child);

    ObjectNode *child(int row);
    int childCount() const;
    int columnCount() const;
    void setData(const QList<QVariant> &data);
    QVariant data(int column) const;
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




#endif
