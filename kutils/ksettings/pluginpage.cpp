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

#include "kcdpluginpage.h"
#include "kpluginselector.h"
#include <kglobal.h>
#include <qlayout.h>
#include <kdialog.h>

class KCDPluginPage::KCDPluginPagePrivate
{
	public:
		KCDPluginPagePrivate()
			: selwid( 0 )
		{
		}

		KPluginSelector * selwid;
};

KCDPluginPage::KCDPluginPage( QWidget * parent, const char * name, const QStringList & args )
	: KCModule( parent, name, args )
	, d( new KCDPluginPagePrivate )
{
	( new QVBoxLayout( this, 0, KDialog::spacingHint() ) )->setAutoAdd( true );
	d->selwid = new KPluginSelector( this );
	connect( d->selwid, SIGNAL( changed( bool ) ), this, SLOT( setChanged( bool ) ) );
}

KCDPluginPage::KCDPluginPage( KInstance * instance, QWidget * parent, const QStringList & args )
	: KCModule( instance, parent, args )
	, d( new KCDPluginPagePrivate )
{
	( new QVBoxLayout( this, 0, KDialog::spacingHint() ) )->setAutoAdd( true );
	d->selwid = new KPluginSelector( this );
	connect( d->selwid, SIGNAL( changed( bool ) ), this, SLOT( setChanged( bool ) ) );
}

KCDPluginPage::~KCDPluginPage()
{
	delete d;
}

KPluginSelector * KCDPluginPage::pluginSelector()
{
	return d->selwid;
}

void KCDPluginPage::load()
{
	d->selwid->load();
}

void KCDPluginPage::save()
{
	d->selwid->save();
}

void KCDPluginPage::defaults()
{
	d->selwid->defaults();
}

#include "kcdpluginpage.moc"
// vim: sw=4 ts=4 noet
