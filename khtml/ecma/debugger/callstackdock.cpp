#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>

#include <kdebug.h>

#include "debugdocument.h"
#include "callstackdock.h"
#include "callstackdock.moc"

CallStackDock::CallStackDock(QWidget *parent)
    : QDockWidget("Call Stack", parent)
{
    m_view = new QTableWidget(0, 2);
    m_view->setHorizontalHeaderLabels(QStringList() << "Call" << "Line");
    m_view->verticalHeader()->hide();
    m_view->setShowGrid(false);
    m_view->setAlternatingRowColors(true);

    setWidget(m_view);
}

CallStackDock::~CallStackDock()
{
}

void CallStackDock::displayStack(KJS::DebugDocument *document)
{
    QVector<KJS::CallStackEntry> entries = document->callStack();

    m_view->clearContents();
    m_view->setRowCount(entries.count());
    m_current = document;

    int row = 0;
    foreach (KJS::CallStackEntry entry, entries)
    {
        QTableWidgetItem *function = new QTableWidgetItem(QString("%1").arg(entry.name));
        m_view->setItem(row, 0, function);
        QTableWidgetItem *lineNumber = new QTableWidgetItem(QString("%1").arg(entry.lineNumber));
        m_view->setItem(row, 1, lineNumber);
        row++;
    }
    m_view->resizeColumnsToContents();
    m_view->resizeRowsToContents();
//    m_view->setColumnWidth(1, 20);
}

