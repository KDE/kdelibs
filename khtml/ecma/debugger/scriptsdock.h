#ifndef SCRIPTSDOCK_H
#define SCRIPTSDOCK_H

#include <QDockWidget>


class QTreeView;
class QStandardItemModel;

namespace KJS
{
class DebugDocument;
}

class ScriptsDock : public QDockWidget
{
    Q_OBJECT
public:
    ScriptsDock(QWidget *parent = 0);
    ~ScriptsDock();

    void addDocument(KJS::DebugDocument *document);

public slots:
    void documentDestroyed(KJS::DebugDocument *document);

signals:
     void displayScript(KJS::DebugDocument *document);

private slots:
    void scriptSelected(const QModelIndex &idx);

private:
    void updateModel();
//     QTreeWidget *m_widget;
//     QHash<KJS::DebugDocument*, QTreeWidgetItem*> m_documents;

    QHash<QString, KJS::DebugDocument*> m_urlDocLut;
    QList<KJS::DebugDocument*> m_documents;
    QTreeView *m_view;
    QStandardItemModel *m_model;
};

#endif
