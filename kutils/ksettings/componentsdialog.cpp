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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include "kselectentriesdialog.h"
#include <klocale.h>
#include <qlayout.h>
#include <klistview.h>
#include <qframe.h>
#include <qlabel.h>
#include <qstring.h>
#include <qheader.h>
#include "kplugininfo.h"
#include <kiconloader.h>
#include <kdebug.h>
#include <kconfig.h>

class KSelectEntriesDialog::KSelectEntriesDialogPrivate
{
    public:
        KListView * listview;
        QFrame * infowidget;
        QLabel * iconwidget;
        QLabel * commentwidget;
        QLabel * descriptionwidget;
        QMap<QCheckListItem*, KPluginInfo*> plugininfomap;
        QValueList<KPluginInfo*> plugininfolist;
};

KSelectEntriesDialog::KSelectEntriesDialog( QWidget * parent, const char * name )
    : KDialogBase( parent, name, false, i18n( "Select components" ) )
, d( new KSelectEntriesDialogPrivate )
{
    QWidget * page = new QWidget( this );
    setMainWidget( page );
    ( new QHBoxLayout( page, KDialog::marginHint(), KDialog::spacingHint() ) )->setAutoAdd( true );
    d->listview = new KListView( page );
    d->infowidget = new QFrame( page );
    ( new QVBoxLayout( d->infowidget, KDialog::marginHint(), KDialog::spacingHint() ) )->setAutoAdd( true );
    d->iconwidget = new QLabel( d->infowidget );
    d->commentwidget = new QLabel( d->infowidget );
    d->descriptionwidget = new QLabel( d->infowidget );

    d->listview->addColumn( QString::null );
    d->listview->header()->hide();
    d->listview->setRootIsDecorated( true );
    d->listview->setSorting( -1 );
    d->listview->setAcceptDrops( false );
    d->listview->setSelectionModeExt( KListView::Single );
    d->listview->setAllColumnsShowFocus( true );

    connect( d->listview, SIGNAL( pressed( QListViewItem * ) ), this,
            SLOT( executed( QListViewItem * ) ) );
    connect( d->listview, SIGNAL( spacePressed( QListViewItem * ) ), this,
            SLOT( executed( QListViewItem * ) ) );
    connect( d->listview, SIGNAL( returnPressed( QListViewItem * ) ), this,
            SLOT( executed( QListViewItem * ) ) );
    connect( d->listview, SIGNAL( selectionChanged( QListViewItem * ) ), this,
            SLOT( executed( QListViewItem * ) ) );
}

KSelectEntriesDialog::~KSelectEntriesDialog()
{
}

void KSelectEntriesDialog::setPluginInfos( const QMap<QString, KPluginInfo*> &
        plugininfos )
{
    for( QMap<QString, KPluginInfo*>::ConstIterator it = plugininfos.begin();
            it != plugininfos.end(); ++it )
    {
        d->plugininfolist.append( it.data() );
    }
}

void KSelectEntriesDialog::show()
{
    // construct the treelist
    for( QValueList<KPluginInfo*>::ConstIterator it = d->plugininfolist.begin();
            it != d->plugininfolist.end(); ++it )
    {
        ( *it )->load();
        QCheckListItem * item = new QCheckListItem( d->listview, ( *it )->name(),
                QCheckListItem::CheckBox );
        if( ! ( *it )->icon().isEmpty() )
            item->setPixmap( 0, KGlobal::iconLoader()->loadIcon( ( *it )->icon(), KIcon::Small ) );
        item->setOn( ( *it )->pluginEnabled() );
        d->plugininfomap[ item ] = ( *it );
    }
    KDialogBase::show();
}

void KSelectEntriesDialog::executed( QListViewItem * item )
{
    kdDebug( 704 ) << k_funcinfo << endl;
    if( item == 0 )
        return;
    if( item->rtti() != 1 ) // check for QCheckListItem
        return;

    QCheckListItem * citem = static_cast<QCheckListItem *>( item );
    bool checked = citem->isOn();

    kdDebug( 704 ) << "it's a " << ( checked ? "checked" : "unchecked" )
        << " QCheckListItem" << endl;

    KPluginInfo * info = d->plugininfomap[ citem ];
    info->setPluginEnabled( checked );
    //checkDependencies( info );
}

void KSelectEntriesDialog::savePluginInfos()
{
    for( QValueList<KPluginInfo*>::ConstIterator it = d->plugininfolist.begin();
            it != d->plugininfolist.end(); ++it )
    {
        if( ( *it )->config() )
        {
            ( *it )->save();
            ( *it )->config()->sync();
        }
    }
}

void KSelectEntriesDialog::slotOk()
{
    savePluginInfos();
    KDialogBase::slotOk();
}

void KSelectEntriesDialog::slotApply()
{
    savePluginInfos();
    KDialogBase::slotApply();
}

#include "kselectentriesdialog.moc"
// vim: sw=4 sts=4 et
