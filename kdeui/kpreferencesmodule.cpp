/*  This file is part of the KDE project
    Copyright (C) 2002 Matthias Kretz <kretz@kde.org>

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
// $Id$

#include "kpreferencesmodule.h"
#include "kpreferencesdialog.h"
#include "kdialogbase.h"

#include <qframe.h>

#include <kiconloader.h>

static inline QPixmap loadIcon( const QString & icon )
{
	return KGlobal::iconLoader()->loadIcon( icon, KIcon::NoGroup, KIcon::SizeMedium );
}

struct KPreferencesModule::KPreferencesModulePrivate
{
	KPreferencesDialog * dialog;
};

KPreferencesModule::KPreferencesModule( const QString & name, const QString & description, const QString & icon, QObject * parent, const char * _name )
	: QObject( parent, _name )
	, d( new KPreferencesModulePrivate )
{
	d->dialog = KPreferencesDialog::self();
	d->dialog->addModule( this );
	m_page = d->dialog->m_dialog->addPage( name, description, loadIcon( icon ) );
}

KPreferencesModule::~KPreferencesModule()
{
	d->dialog->removeModule( this );
	delete m_page;
	delete d;
}

// vim:sw=4:ts=4

#include "kpreferencesmodule.moc"

