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

#include "ksettings/componentsdialog.h"
#include <klocale.h>
#include <qlayout.h>
#include <klistview.h>
#include <qlabel.h>
#include <q3header.h>
#include <kplugininfo.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kseparator.h>
#include <QList>

namespace KSettings
{

class ComponentsDialog::ComponentsDialogPrivate
{
    public:
        KListView * listview;
        QFrame * infowidget;
        QLabel * iconwidget;
        QLabel * commentwidget;
        QLabel * descriptionwidget;
        QMap<Q3CheckListItem*, KPluginInfo*> plugininfomap;
        QList<KPluginInfo*> plugininfolist;
};

ComponentsDialog::ComponentsDialog( QWidget * parent, const char * name )
    : KDialogBase( parent, name, false, i18n( "Select Components" ) )
, d( new ComponentsDialogPrivate )
{
    QWidget * page = new QWidget( this );
    setMainWidget( page );
    ( new QHBoxLayout( page, 0, KDialog::spacingHint() ) )->setAutoAdd( true );
    d->listview = new KListView( page );
    d->listview->setMinimumSize( 200, 200 );
    d->infowidget = new QFrame( page );
    d->infowidget->setMinimumSize( 200, 200 );
    ( new QVBoxLayout( d->infowidget, 0, KDialog::spacingHint() ) )->setAutoAdd( true );
    d->iconwidget = new QLabel( d->infowidget );
    ( void )new KSeparator( d->infowidget );
    d->commentwidget = new QLabel( d->infowidget );
    d->commentwidget->setAlignment( Qt::TextWordWrap );
    d->descriptionwidget = new QLabel( d->infowidget );
    d->descriptionwidget->setAlignment( Qt::TextWordWrap );

    d->listview->addColumn( QString::null );
    d->listview->header()->hide();
    d->listview->setRootIsDecorated( true );
    d->listview->setSorting( -1 );
    d->listview->setAcceptDrops( false );
    d->listview->setSelectionModeExt( KListView::Single );
    d->listview->setAllColumnsShowFocus( true );

    connect( d->listview, SIGNAL( pressed( Q3ListViewItem * ) ), this,
            SLOT( executed( Q3ListViewItem * ) ) );
    connect( d->listview, SIGNAL( spacePressed( Q3ListViewItem * ) ), this,
            SLOT( executed( Q3ListViewItem * ) ) );
    connect( d->listview, SIGNAL( returnPressed( Q3ListViewItem * ) ), this,
            SLOT( executed( Q3ListViewItem * ) ) );
    connect( d->listview, SIGNAL( selectionChanged( Q3ListViewItem * ) ), this,
            SLOT( executed( Q3ListViewItem * ) ) );
}

ComponentsDialog::~ComponentsDialog()
{
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
        d->plugininfolist.append( it.data() );
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
    for( QList<KPluginInfo*>::ConstIterator it = d->plugininfolist.begin();
            it != d->plugininfolist.end(); ++it )
    {
        ( *it )->load();
        Q3CheckListItem * item = new Q3CheckListItem( d->listview, ( *it )->name(),
                Q3CheckListItem::CheckBox );
        if( ! ( *it )->icon().isEmpty() )
            item->setPixmap( 0, SmallIcon( ( *it )->icon(), IconSize( KIcon::Small ) ) );
        item->setOn( ( *it )->isPluginEnabled() );
        d->plugininfomap[ item ] = ( *it );
    }
    KDialogBase::show();
}

void ComponentsDialog::executed( Q3ListViewItem * item )
{
    kdDebug( 704 ) << k_funcinfo << endl;
    if( item == 0 )
        return;
    if( item->rtti() != 1 ) // check for QCheckListItem
        return;

    Q3CheckListItem * citem = static_cast<Q3CheckListItem *>( item );
    bool checked = citem->isOn();

    kdDebug( 704 ) << "it's a " << ( checked ? "checked" : "unchecked" )
        << " QCheckListItem" << endl;

    KPluginInfo * info = d->plugininfomap[ citem ];
    info->setPluginEnabled( checked );
    //checkDependencies( info );
    // show info about the component on the right
    d->iconwidget->setPixmap( SmallIcon( info->icon(), KIcon::SizeLarge ) );
    d->commentwidget->setText( info->comment() );
    //d->descriptionwidget->setText( info->description() );
}

void ComponentsDialog::savePluginInfos()
{
    for( QList<KPluginInfo*>::ConstIterator it = d->plugininfolist.begin();
            it != d->plugininfolist.end(); ++it )
    {
        if( ( *it )->config() )
        {
            ( *it )->save();
            ( *it )->config()->sync();
        }
    }
}

void ComponentsDialog::slotOk()
{
    savePluginInfos();
    KDialogBase::slotOk();
}

void ComponentsDialog::slotApply()
{
    savePluginInfos();
    KDialogBase::slotApply();
}

} //namespace

#include "componentsdialog.moc"
// vim: sw=4 sts=4 et
