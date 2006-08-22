#include <QVBoxLayout>
#include <QTreeView>
#include <QStandardItemModel>
#include <QHeaderView>

#include <kdebug.h>

#include "debugwindow.h"
#include "scriptsdock.h"
#include "scriptsdock.moc"

using namespace KJS;

ScriptsDock::ScriptsDock(QWidget *parent)
    : QDockWidget("Loaded Scripts", parent)
{
    m_view = new QTreeView;
    m_view->header()->hide();
    m_model = new QStandardItemModel;
    m_view->setModel(m_model);

    connect(m_view, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(scriptSelected(const QModelIndex &)));

    setWidget(m_view);


}

ScriptsDock::~ScriptsDock()
{
}

void ScriptsDock::documentDestroyed(KJS::DebugDocument *document)
{
    m_documents.removeAll(document);
    updateModel();
}

void ScriptsDock::addDocument(KJS::DebugDocument *document)
{
    m_documents.append(document);
    updateModel();
}

void ScriptsDock::scriptSelected(const QModelIndex &idx)
{
    if (!idx.isValid())
        return;

    QVariant var = idx.data(Qt::UserRole);
    KJS::DebugDocument *doc = var.value<DebugDocument*>();
    if (doc)
        emit displayScript(doc);
}

void ScriptsDock::updateModel()
{
    m_model->clear();

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

    m_view->expandAll();
}

