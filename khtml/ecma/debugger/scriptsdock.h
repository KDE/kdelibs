#ifndef SCRIPTSDOCK_H
#define SCRIPTSDOCK_H

#include <QDockWidget>
#include <QTreeWidgetItem>

class QTreeWidget;

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

signals:
    void displayScript(KJS::DebugDocument *document);

private slots:
    void scriptSelected(QTreeWidgetItem *item, int column);

private:
    QTreeWidget *m_widget;
    QHash<QTreeWidgetItem*, KJS::DebugDocument*> m_documents;
};

#endif
