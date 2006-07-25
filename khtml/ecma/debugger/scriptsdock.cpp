#include <QVBoxLayout>
#include <QListWidget>
#include <QTreeWidget>

#include <kdebug.h>

#include "debugwindow.h"
#include "scriptsdock.h"
#include "scriptsdock.moc"

using namespace KJS;

ScriptsDock::ScriptsDock(QWidget *parent)
    : QDockWidget("Loaded Scripts", parent)
{
    m_widget = new QTreeWidget(this);
    m_widget->setColumnCount(2);
    connect(m_widget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
            this, SLOT(scriptSelected(QTreeWidgetItem*, int)));

    setWidget(m_widget);
}

ScriptsDock::~ScriptsDock()
{
/*
    if (m_widget)
        delete m_widget;
    m_widget = 0;
*/
}

void ScriptsDock::documentDestroyed(KJS::DebugDocument *document)
{
    QTreeWidgetItem *item = m_documents[document];
    if (item)
    {
        int idx = m_widget->indexOfTopLevelItem(item);
        if (idx != -1)
            m_widget->takeTopLevelItem(idx);
    }
}

void ScriptsDock::addDocument(KJS::DebugDocument *document)
{
    if (document && m_widget)
    {
        QTreeWidgetItem *item = m_documents[document];
        if (item)
        {
            int idx = m_widget->indexOfTopLevelItem(item);
            item = m_widget->topLevelItem(idx);
            item->setText(1, "multiple");
        }
        else
        {
            item = new QTreeWidgetItem;
            item->setText(0, document->url());
            m_widget->addTopLevelItem(item);
            m_documents[document] = item;
        }
    }
    else
    {
        kDebug() << "ERRORORORORORORR" << endl;
    }
}

void ScriptsDock::scriptSelected(QTreeWidgetItem *item, int column)
{
    KJS::DebugDocument *doc = m_documents.key(item);
    if (doc)
        emit displayScript(doc);
}



