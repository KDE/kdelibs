/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QLabel>
#include <QLayout>
#include <QTreeWidget>
#include <QString>

#include <klocale.h>
#include <kmessagebox.h>

#include "engine.h"
#include "provider.h"

#include "providerdialog.h"
#include "providerdialog.moc"

using namespace KNS;

class ProviderItem : public QTreeWidgetItem
{
  public:
    ProviderItem( QTreeWidget *parent, Provider *provider ) :
      QTreeWidgetItem( parent ), mProvider( provider )
    {
      setText( 0, provider->name() );
    }

    Provider *provider() { return mProvider; }

  private:
    Provider *mProvider;
};

ProviderDialog::ProviderDialog( Engine *engine, QWidget *parent ) :
  KDialogBase( Plain, i18n("Hot New Stuff Providers"), Ok | Cancel, Cancel,
               parent, 0, false, true ),
  mEngine( engine )
{
  QFrame *topPage = plainPage();

  QBoxLayout *topLayout = new QVBoxLayout( topPage );

  QLabel *description = new QLabel( i18n("Please select one of the providers listed below:"), topPage );
  topLayout->addWidget( description );

  mListWidget = new QTreeWidget( topPage );
  mListWidget->setHeaderLabels( QStringList( i18n("Name") ) );
  topLayout->addWidget( mListWidget );
}

void ProviderDialog::clear()
{
  mListWidget->clear();
}

void ProviderDialog::addProvider( Provider *provider )
{
  new ProviderItem( mListWidget, provider );
  if ( mListWidget->model()->rowCount() == 1 ) {
    QModelIndex index = mListWidget->model()->index( 0, 0 );
    mListWidget->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Select );
  } else if (mListWidget->model()->rowCount() > 1) {
    QModelIndex index = mListWidget->model()->index( 0, 0 );
    mListWidget->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Deselect );
  }
}

void ProviderDialog::slotOk()
{
  QList<QTreeWidgetItem*> items = mListWidget->selectedItems();
  ProviderItem *item = static_cast<ProviderItem *>( items.first() );
  if ( !item ) {
    KMessageBox::error( this, i18n("No provider selected.") );
    return;
  }

  mEngine->requestMetaInformation( item->provider() );

  accept();
}
