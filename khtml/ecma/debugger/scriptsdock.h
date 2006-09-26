#ifndef SCRIPTSDOCK_H
#define SCRIPTSDOCK_H

#include <QDockWidget>

class QTreeWidget;
class QTreeWidgetItem;
class QStandardItemModel;
class QModelIndex;

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
    void scriptSelected(QTreeWidgetItem *item, int column);

private:
    void updateModel();
    QTreeWidget *m_widget;
    QHash<KJS::DebugDocument*, QTreeWidgetItem*> m_documents;
    QHash<QString, QTreeWidgetItem*> m_headers;
};

#endif
