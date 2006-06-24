#include <QVBoxLayout>
#include <QListWidget>
#include <QTreeWidget>

#include <kdebug.h>

#include "debugwindow.h"
#include "scriptsdock.h"
#include "scriptsdock.moc"

ScriptsDock::ScriptsDock(QWidget *parent)
    : QDockWidget("Loaded Scripts", parent)
{
    QTreeWidget *m_widget = new QTreeWidget(this);
    m_widget->setColumnCount(1);

    for (int i = 0; i < 10; i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, QString("item :%1").arg(i));
        m_widget->addTopLevelItem(item);
    }

    setWidget(m_widget);
}

ScriptsDock::~ScriptsDock()
{
/*
    if (m_widget)
    {
        delete m_widget;
        m_widget = 0;
    }
*/
}

void ScriptsDock::addDocument(KJS::DebugDocument *document)
{
/*
    if (m_widget)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, "Blah");
        m_widget->addTopLevelItem(item);
    }
*/

    /*
    if (document && m_widget)
    {
        QList<QTreeWidgetItem *> items = m_widget->findItems("blah", Qt::MatchExactly);
        if (items.count() > 0)
        {
            QTreeWidgetItem *script = items[0];
            int idx = m_widget->indexOfTopLevelItem(script);
            m_widget->takeTopLevelItem(idx);

            m_widget->insertTopLevelItem(idx, new QTreeWidgetItem(m_widget, QStringList("blah")));
            kDebug() << "inserted document for url: " << document->url() << endl;
        }
        else
        {
            m_widget->addTopLevelItem(new QTreeWidgetItem(m_widget, QStringList("blah")));
            kDebug() << "added document for url: " << document->url() << endl;
        }
    }
    else
    {
        kDebug() << "ERRORORORORORORR";
    }
    */
}



