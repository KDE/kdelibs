/*  This file is part of the KDE project
    Copyright (C) 2002-2003 Matthias Kretz <kretz@kde.org>

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

#include <qmap.h>
#include <qptrdict.h>
#include <qptrlist.h>
#include <qstring.h>

#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kxmlguiclient.h>
#include <kaction.h>

class KPreferencesActionHandler : public KXMLGUIClient
{
	public:
		KPreferencesActionHandler( QCString actionName, KXMLGUIClient * parent )
			: KXMLGUIClient( parent )
			, guiDescription( ""
					"<!DOCTYPE kpartgui><kpartgui name=\"KPreferencesActionHandler\">"
					"<MenuBar>"
					"    <Menu name=\"settings\">"
					"        <Action name=\"%1\" />"
					"    </Menu>"
					"</MenuBar>"
					"</kpartgui>" )
		{
			QString completeDescription = QString::fromLatin1( guiDescription ).arg( actionName );
			setXML( completeDescription, false /*merge*/ );
		}

		~KPreferencesActionHandler()
		{
			kdDebug( 297 ) << k_funcinfo << endl;
		}
	private:
		const char * guiDescription;
};

static inline QPixmap loadIcon( const QString & icon )
{
	return KGlobal::iconLoader()->loadIcon( icon, KIcon::NoGroup, KIcon::SizeMedium );
}

static inline QPixmap loadSmallIcon( const QString & icon )
{
	return KGlobal::iconLoader()->loadIcon( icon, KIcon::NoGroup, KIcon::SizeSmall );
}

class KPreferencesDialog::KPreferencesDialogPrivate
{
	public:
		KPreferencesDialogPrivate()
			: dialog( 0 )
			, actionHandler( 0 )
			, aPref( 0 )
		{}

		~KPreferencesDialogPrivate()
		{
			delete actionHandler;
		}

		QMap<QObject*, QString> groupNames;
		QPtrDict<QFrame> pages;
		QPtrList<KPreferencesModule> modules;

		KDialogBase * dialog;

		KPreferencesActionHandler * actionHandler;
		KAction * aPref;

		bool tree;
};

KPreferencesDialog::KPreferencesDialog( QObject * parent, const char * name )
	: QObject( parent, name )
	, d( new KPreferencesDialogPrivate )
{
}

KPreferencesDialog::~KPreferencesDialog()
{
	delete d;
}

void KPreferencesDialog::show()
{
	if( ! d->dialog )
	{
		d->tree = d->groupNames.count() > 1;
		d->dialog = new KDialogBase( d->tree ? KDialogBase::TreeList : KDialogBase::IconList,
				i18n( "Preferences" ),
				KDialogBase::Help | KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel,
				KDialogBase::Ok, 0, "KPreferencesDialog Dialog", false );
		d->dialog->setShowIconsInTreeList( true );
		d->dialog->setRootIsDecorated( false );
		d->dialog->showButton( KDialogBase::Help, false );
		connect( d->dialog, SIGNAL( applyClicked() ), SLOT( slotApply() ) );
		connect( d->dialog, SIGNAL( okClicked() ), SLOT( slotOk() ) );
		connect( d->dialog, SIGNAL( finished() ), SLOT( slotFinished() ) );

		// the modules have already registered with the dialog
		// -> create the pages
		for( KPreferencesModule * m = d->modules.first(); m; m = d->modules.next() )
			createPage( m );

		d->dialog->unfoldTreeList( true );
	}
	d->dialog->show();
}

void KPreferencesDialog::registerModule( KPreferencesModule * m )
{
	kdDebug( 297 ) << k_funcinfo << endl;
	d->modules.append( m );
	connect( m, SIGNAL( destroyed( QObject * ) ), this,
			SLOT( unregisterModule( QObject * ) ) );
	if( d->dialog )
		createPage( m );
}

void KPreferencesDialog::unregisterModule( QObject * o )
{
	// remove the pointer to the module before it becomes dangling
	d->modules.removeRef( static_cast<KPreferencesModule*>( o ) );
	if( ! d->pages.isEmpty() )
		delete d->pages[ o ];
}

void KPreferencesDialog::slotApply()
{
	for( KPreferencesModule * m = d->modules.first(); m; m = d->modules.next() )
		m->applyChanges();
}

void KPreferencesDialog::slotOk()
{
	slotApply();
}

void KPreferencesDialog::slotFinished()
{
	d->dialog->delayedDestruct();
	d->dialog = 0;
	d->pages.clear();
}

void KPreferencesDialog::setGroupName( KPreferencesModule * mod, const QString & name )
{
	// set the name of the group that m belongs to to name
	d->groupNames[ mod->parent() ] = name;
	if( d->aPref )
	{
		d->aPref->setText( i18n( "Configure %1..." ).arg( name ) );
		d->aPref->setIcon( mod->pixmap() );
	}
}

void KPreferencesDialog::createMenuEntry()
{
	KXMLGUIClient * xmlc = dynamic_cast<KXMLGUIClient*>( parent() );
	if( xmlc )
	{
		QCString name = "options_configure_";
		name += parent()->name();
		d->actionHandler = new KPreferencesActionHandler( name, xmlc );
		d->aPref = new KAction( 0, 0, this, SLOT( show() ), d->actionHandler->actionCollection(), name );
	}
	else
		kdWarning( 297 ) << "no KXMLGUIClient parent! I won't create a menuentry for the dialog!" << endl;
}

void KPreferencesDialog::createPage( KPreferencesModule * m )
{
	if( d->tree )
	{
		QStringList path;
		if( ! d->groupNames[ m->parent() ].isNull() )
			path += d->groupNames[ m->parent() ];
		if( ! m->itemName().isNull() )
			path += m->itemName();
		d->pages.insert( m, d->dialog->addPage( path, m->header(), loadSmallIcon( m->pixmap() ) ) );
		m->createPage( d->pages[ m ] );
	}
	else
	{
		d->pages.insert( m, d->dialog->addPage(
					m->itemName().isNull() ? d->groupNames[ m->parent() ] : m->itemName(),
					m->header(), loadIcon( m->pixmap() ) ) );
		m->createPage( d->pages[ m ] );
	}
}

#include "kpreferencesdialog.moc"

// vim: sw=4 ts=4
