/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "componentsdialog_p.h"
#include <klocale.h>
#include <QLayout>
#include <QLabel>
#include <kplugininfo.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kseparator.h>

#include <QtCore/QList>
#include <QTreeWidget>

namespace KSettings
{

class ComponentsDialog::ComponentsDialogPrivate
{
    public:
        QTreeWidget * listview;
        QFrame * infowidget;
        QLabel * iconwidget;
        QLabel * commentwidget;
        QLabel * descriptionwidget;
        QMap<QTreeWidgetItem*, KPluginInfo*> plugininfomap;
        QList<KPluginInfo*> plugininfolist;
};

ComponentsDialog::ComponentsDialog( QWidget * parent, const char * name )
    : KDialog( parent ), d( new ComponentsDialogPrivate )
{
    setObjectName( name );
    setModal( false );
    setCaption( i18n( "Select Components" ) );

    QWidget * page = new QWidget( this );
    setMainWidget( page );
    QHBoxLayout *hbox = new QHBoxLayout( page );
    hbox->setMargin( 0 );

    d->listview = new QTreeWidget( page );
    d->listview->setMinimumSize( 200, 200 );
    d->infowidget = new QFrame( page );
    d->infowidget->setMinimumSize( 200, 200 );

    QVBoxLayout *vbox = new QVBoxLayout( d->infowidget );
    vbox->setMargin( 0 );

    d->iconwidget = new QLabel( d->infowidget );
    vbox->addWidget( d->iconwidget );
    vbox->addWidget( new KSeparator( d->infowidget ) );
    d->commentwidget = new QLabel( d->infowidget );
    d->commentwidget->setWordWrap( true );
    vbox->addWidget( d->commentwidget );
    d->descriptionwidget = new QLabel( d->infowidget );
    d->descriptionwidget->setWordWrap( true );
    vbox->addWidget( d->descriptionwidget );

    d->listview->setAcceptDrops( false );

    connect( d->listview, SIGNAL(itemPressed(QTreeWidgetItem*,int)), this,
            SLOT(executed(QTreeWidgetItem*,int)) );
    connect( d->listview, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this,
            SLOT(executed(QTreeWidgetItem*,int)) );
    connect( d->listview, SIGNAL(itemSelectionChanged(QTreeWidgetItem*,int)), this,
            SLOT(executed(QTreeWidgetItem*,int)) );
}

ComponentsDialog::~ComponentsDialog()
{
    delete d;
}

void ComponentsDialog::addPluginInfo( KPluginInfo * info )
{
    d->plugininfolist.append( info );
}

void ComponentsDialog::setPluginInfos( const QMap<QString, KPluginInfo*> &
        plugininfos )
{
    for( QMap<QString, KPluginInfo*>::ConstIterator it = plugininfos.begin();
            it != plugininfos.end(); ++it )
    {
        d->plugininfolist.append( it.value() );
    }
}

void ComponentsDialog::setPluginInfos( const QList<KPluginInfo *> &plugins )
{
    d->plugininfolist = plugins;
}

void ComponentsDialog::show()
{
    // clear the treelist
    d->listview->clear();
    d->plugininfomap.clear();

    // construct the treelist
    for( QList<KPluginInfo*>::ConstIterator it = d->plugininfolist.constBegin();
            it != d->plugininfolist.constEnd(); ++it )
    {
        ( *it )->load();
        QTreeWidgetItem * item = new QTreeWidgetItem( d->listview, QStringList( ( *it )->name() ) );
        if( ! ( *it )->icon().isEmpty() )
            item->setIcon( 0, SmallIcon( ( *it )->icon(), IconSize( KIconLoader::Small ) ) );
        item->setCheckState( 0, ( *it )->isPluginEnabled() ? Qt::Checked : Qt::Unchecked );
        d->plugininfomap[ item ] = ( *it );
    }
    KDialog::show();
}

void ComponentsDialog::executed( QTreeWidgetItem * item, int )
{
    kDebug( 704 ) ;
    if( item == 0 )
        return;

    bool checked = ( item->checkState(0) == Qt::Checked );

    kDebug( 704 ) << "it's a " << ( checked ? "checked" : "unchecked" )
        << " QCheckListItem" << endl;

    KPluginInfo * info = d->plugininfomap[ item ];
    info->setPluginEnabled( checked );
    //checkDependencies( info );
    // show info about the component on the right
    d->iconwidget->setPixmap( SmallIcon( info->icon(), KIconLoader::SizeLarge ) );
    d->commentwidget->setText( info->comment() );
    //d->descriptionwidget->setText( info->description() );
}

void ComponentsDialog::savePluginInfos()
{
    for( QList<KPluginInfo*>::ConstIterator it = d->plugininfolist.constBegin();
            it != d->plugininfolist.constEnd(); ++it )
    {
        if ((*it)->config().isValid()) {
            ( *it )->save();
            (*it)->config().sync();
        }
    }
}

void ComponentsDialog::slotOk()
{
    savePluginInfos();
    KDialog::slotButtonClicked( Ok );
}

void ComponentsDialog::slotApply()
{
    savePluginInfos();
    KDialog::slotButtonClicked( Apply );
}

} //namespace

#include "moc_componentsdialog_p.cpp"
