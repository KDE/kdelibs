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

class KSelectEntriesDialog::KSelectEntriesDialogPrivate
{
    public:
        KListView * listview;
        QFrame * infowidget;
        QLabel * iconwidget;
        QLabel * commentwidget;
        QLabel * descriptionwidget;
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
}

KSelectEntriesDialog::~KSelectEntriesDialog()
{
}

#include "kselectentriesdialog.moc"
// vim: sw=4 sts=4 et
