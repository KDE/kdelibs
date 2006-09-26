#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStandardItemModel>
#include <QHeaderView>

#include <kdebug.h>

#include "debugwindow.h"
#include "debugdocument.h"
#include "scriptsdock.h"
#include "scriptsdock.moc"

using namespace KJS;

ScriptsDock::ScriptsDock(QWidget *parent)
    : QDockWidget("Loaded Scripts", parent)
{
    m_widget = new QTreeWidget;

    connect(m_widget, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
            this, SLOT(scriptSelected(QTreeWidgetItem *, int)));

    setWidget(m_widget);
}

ScriptsDock::~ScriptsDock()
{
}

void ScriptsDock::documentDestroyed(KJS::DebugDocument *document)
{
    if (!m_documents.contains(document))
        return;

    QTreeWidgetItem *child = m_documents[document];
    QTreeWidgetItem *parent = child->parent();

    int childIdx = parent->indexOfChild(child);
    parent->takeChild(childIdx);

    if (parent->childCount() == 0)
    {
        int parentIdx = m_widget->indexOfTopLevelItem(parent);
        parent = m_widget->takeTopLevelItem(parentIdx);
        m_headers[parent->text(0)];
    }

    m_documents[document] = 0;

}

void ScriptsDock::addDocument(KJS::DebugDocument *document)
{
    if (m_documents.contains(document))
        return;

    QString name = document->name();
    QString domain = QUrl(document->url()).host();
    if (domain.isEmpty())
        domain = "unknown";

    QTreeWidgetItem *parent = 0;
    if (m_headers.contains(domain))
        parent = m_headers[domain];
    else
    {
        parent = new QTreeWidgetItem(QStringList() << domain);
        m_headers[domain] = parent;
    }

    QTreeWidgetItem *child = new QTreeWidgetItem(parent, QStringList() << name);
    m_documents[document] = child;
    m_widget->invisibleRootItem()->addChild(parent);
}

void ScriptsDock::scriptSelected(QTreeWidgetItem *item, int column)
{
    KJS::DebugDocument *doc = m_documents.key(item);
    if (doc)
        emit displayScript(doc);
}

void ScriptsDock::updateModel()
{
/*    m_model->clear();

    QHash<QString, QStandardItem*> parents;

    QStandardItem *top = m_model->invisibleRootItem();
    foreach (KJS::DebugDocument *document, m_documents)
    {
        QString domain = QUrl(document->url()).host();
        if (domain.isEmpty())
            domain = "unknown";

        QString name = document->name();

        kDebug() << "domain: " << domain << ", name: " << name << endl;
        QStandardItem *parent = parents[domain];
        if (!parent)
        {
            kDebug() << "Couldn't find the domain, adding it" << endl; 
            parent = new QStandardItem(domain);
            parent->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            parents[domain] = parent;
            top->appendRow(parent);
        }

        QStandardItem *item = new QStandardItem(name);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        QVariant var;
        var.setValue(document);
        item->setData(var, Qt::UserRole);
        parent->appendRow(item);
    }

    m_view->expandAll();*/
}

