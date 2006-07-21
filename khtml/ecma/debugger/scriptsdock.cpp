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

void ScriptsDock::addDocument(KJS::DebugDocument *document)
{
    if (document && m_widget)
    {
        QList<QTreeWidgetItem *> items = m_widget->findItems(document->url(), Qt::MatchExactly);
        kDebug() << "Found " << items.count() << " items." << endl;
        QTreeWidgetItem *item;
        if (items.count() > 0)
        {
            QTreeWidgetItem *script = items.takeFirst();
            int idx = m_widget->indexOfTopLevelItem(script);
            item = m_widget->topLevelItem(idx);
            item->setText(1, "multiple");

/*
            item->takeChildren();
            QList<SourceFragment> fragments = document->code();
            foreach (SourceFragment fragment, fragments)
            {
                QTreeWidgetItem *child = new QTreeWidgetItem;
                child->setText(0, QString::number(fragment.baseLine));
                child->setText(1, fragment.source);
                item->addChild(child);
            }
        */
        }
        else
        {
            item = new QTreeWidgetItem;
            item->setText(0, document->url());

/*
            QList<SourceFragment> fragments = document->code();
            foreach (SourceFragment fragment, fragments)
            {
                QTreeWidgetItem *child = new QTreeWidgetItem;
                child->setText(0, QString::number(fragment.baseLine));
                child->setText(1, fragment.source);
                item->addChild(child);
            }
*/
            m_widget->addTopLevelItem(item);
            kDebug() << "added document for url: " << document->url() << endl;
        }
        m_documents[item] = document;
    }
    else
    {
        kDebug() << "ERRORORORORORORR" << endl;
    }
}

void ScriptsDock::scriptSelected(QTreeWidgetItem *item, int column)
{
    KJS::DebugDocument *doc = m_documents[item];
    emit displayScript(doc);
}



