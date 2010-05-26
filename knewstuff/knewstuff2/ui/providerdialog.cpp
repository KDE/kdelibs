/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "providerdialog.h"

#include <QLabel>
#include <QLayout>
#include <QTreeWidget>


#include <klocale.h>
#include <kmessagebox.h>

//#include "engine.h"
#include "knewstuff2/core/provider.h"

using namespace KNS;

class ProviderItem : public QTreeWidgetItem
{
public:
    ProviderItem(QTreeWidget *parent, Provider *provider) :
            QTreeWidgetItem(parent), mProvider(provider) {
        setText(0, provider->name().representation());
    }

    Provider *provider() {
        return mProvider;
    }

private:
    Provider *mProvider;
};

ProviderDialog::ProviderDialog(/*Engine *engine,*/ QWidget *parent) :
        KDialog(parent)
//  mEngine( engine )
{
    setCaption(i18n("Hot New Stuff Providers"));
    setButtons(Ok | Cancel);
    setDefaultButton(Cancel);
    setModal(false);

    QFrame *topPage = new QFrame(this);
    setMainWidget(topPage);

    QBoxLayout *topLayout = new QVBoxLayout(topPage);

    QLabel *description = new QLabel(i18n("Please select one of the providers listed below:"), topPage);
    topLayout->addWidget(description);

    mListWidget = new QTreeWidget(topPage);
    mListWidget->setHeaderLabels(QStringList(i18n("Name")));
    topLayout->addWidget(mListWidget);
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
}

void ProviderDialog::clear()
{
    mListWidget->clear();
}

void ProviderDialog::addProvider(Provider *provider)
{
    new ProviderItem(mListWidget, provider);
    if (mListWidget->model()->rowCount() == 1) {
        QModelIndex index = mListWidget->model()->index(0, 0);
        mListWidget->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
    } else if (mListWidget->model()->rowCount() > 1) {
        QModelIndex index = mListWidget->model()->index(0, 0);
        mListWidget->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Deselect);
    }
}

void ProviderDialog::slotOk()
{
    QList<QTreeWidgetItem*> items = mListWidget->selectedItems();
    ProviderItem *item = static_cast<ProviderItem *>(items.first());
    if (!item) {
        KMessageBox::error(this, i18n("No provider selected."));
        return;
    }

//  mEngine->requestMetaInformation( item->provider() );
//  FIXME: return properly
    m_provider = item->provider();

    accept();
}

KNS::Provider *ProviderDialog::provider() const
{
    return m_provider;
}

#include "providerdialog.moc"
