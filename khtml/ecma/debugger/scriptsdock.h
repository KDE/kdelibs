#ifndef SCRIPTSDOCK_H
#define SCRIPTSDOCK_H

#include <QDockWidget>

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

private:
    QTreeWidget *m_widget;
};

#endif
