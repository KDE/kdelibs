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

#include "kpreferencesdialog.h"
#include "kpreferencesmodule.h"
#include "kdialogbase.h"

#include <qptrlist.h>

#include <klocale.h>
#include <kstaticdeleter.h>

KPreferencesDialog * KPreferencesDialog::m_instance = 0L;
static KStaticDeleter<KPreferencesDialog> instanceDeleter;

class KPreferencesDialog::KPreferencesDialogPrivate
{
	public:
		KPreferencesDialogPrivate()
		{
			modules.setAutoDelete( false );
		}

		~KPreferencesDialogPrivate()
		{
			KPreferencesModule * module;
			while( ( module = modules.first() ) )
				delete module;
		}

		QPtrList<KPreferencesModule> modules;
};

KPreferencesDialog * KPreferencesDialog::self()
{
	if( ! m_instance )
		instanceDeleter.setObject( m_instance, new KPreferencesDialog() );
	return m_instance;
}

KPreferencesDialog::KPreferencesDialog()
    : QObject( 0, "KPreferencesDialog Wrapper" )
	, d( new KPreferencesDialogPrivate )
	, m_dialog( new KDialogBase( KDialogBase::IconList, i18n( "Preferences" ), KDialogBase::Help|KDialogBase::Ok|KDialogBase::Apply|KDialogBase::Cancel, KDialogBase::Ok, 0, "KPreferencesDialog Dialog", false ) )
{
	m_dialog->showButton( KDialogBase::Help, false );
	connect( m_dialog, SIGNAL( applyClicked() ), SLOT( slotApply() ) );
	connect( m_dialog, SIGNAL( okClicked() ), SLOT( slotOk() ) );
}

KPreferencesDialog::~KPreferencesDialog()
{
	m_instance = 0;
	delete d;
	delete m_dialog;
}

void KPreferencesDialog::addModule( KPreferencesModule * module )
{
	d->modules.append( module );
}

void KPreferencesDialog::removeModule( KPreferencesModule * module )
{
	d->modules.removeRef( module );
}

void KPreferencesDialog::show()
{
	self()->showInternal();
}

void KPreferencesDialog::setHelp( const QString & anchor, const QString & appname )
{
	self()->m_dialog->setHelp( anchor, appname );
	self()->m_dialog->showButton( KDialogBase::Help, true );
}

void KPreferencesDialog::showInternal()
{
	KPreferencesModule * module;
	for( module = d->modules.first(); module; module = d->modules.next() )
		module->load();
	m_dialog->show();
}

void KPreferencesDialog::slotApply()
{
	KPreferencesModule * module;
	for( module = d->modules.first(); module; module = d->modules.next() )
		module->save();
	emit reloadConfig();
}

void KPreferencesDialog::slotOk()
{
	slotApply();
	m_dialog->hide();
}

// vim:sw=4:ts=4

#include "kpreferencesdialog.moc"
