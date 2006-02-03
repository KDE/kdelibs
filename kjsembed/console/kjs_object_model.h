#ifndef KJS_OBJECT_MODEL
#define KJS_OBJECT_MODEL

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

namespace KJS
{
    class Interpreter;
    class JSObject;
}

struct Node;
class KJSObjectModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    KJSObjectModel(KJS::Interpreter *js, QObject *parent = 0);
    ~KJSObjectModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void updateModel( KJS::JSObject *m_root);

private:
    KJS::Interpreter *m_js;
    KJS::JSObject *m_root;
};

#endif
