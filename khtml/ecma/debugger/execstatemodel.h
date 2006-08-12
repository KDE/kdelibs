#ifndef EXECSTATEMODEL_H
#define EXECSTATEMODEL_H

#include <QAbstractItemModel>
#include <QHash>

namespace KJS
{
    class JSValue;
    enum JSType;
    class ExecState;
}

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
    QString typeToString(KJS::JSType) const;
    QVariant valueToVariant(KJS::JSValue*) const;

    KJS::ExecState *m_exec;
    Node *m_rootNode;
};

#endif


