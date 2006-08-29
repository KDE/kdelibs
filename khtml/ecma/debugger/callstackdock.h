#ifndef CALLSTACKDOCK_H
#define CALLSTACKDOCK_H

#include <QDockWidget>

namespace KJS
{
    class DebugDocument;
}

class QTableWidget;
class CallStackDock : public QDockWidget
{
    Q_OBJECT
public:
    CallStackDock(QWidget *parent = 0);
    ~CallStackDock();

    void displayStack(KJS::DebugDocument *document);

private:
    QTableWidget *m_view;
    KJS::DebugDocument *m_current;
};

#endif
