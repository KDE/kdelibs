/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart at kde.org>

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

#include "knotifyconfigwidget.h"
#include "knotifyconfigactionswidget.h"
#include "knotifyeventlist.h"
#include "knotifyconfigelement.h"

#include <kapplication.h>
#include <kdialog.h>
#include <QVBoxLayout>
#include <QDBusInterface>
#include <QDBusConnectionInterface>

struct KNotifyConfigWidget::Private
{
	KNotifyEventList *eventList;
	KNotifyConfigActionsWidget *actionsconfig;
	KNotifyConfigElement *currentElement;
};


KNotifyConfigWidget::KNotifyConfigWidget( QWidget * parent )
	: QWidget(parent) , d(new Private)
{
	d->currentElement=0l;
	d->eventList=new KNotifyEventList( this );
	d->eventList->setFocus();
	d->actionsconfig=new KNotifyConfigActionsWidget(this);
	d->actionsconfig->setEnabled(false);
	connect(d->eventList , SIGNAL(eventSelected( KNotifyConfigElement* )) , 
			this , SLOT(slotEventSelected( KNotifyConfigElement* )));
	connect(d->actionsconfig,SIGNAL(changed()),this,SLOT(slotActionChanged()));
    
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->addWidget(d->eventList,1);
	layout->addWidget(d->actionsconfig);
}


KNotifyConfigWidget::~KNotifyConfigWidget()
{
	delete d;
}


void KNotifyConfigWidget::setApplication (const QString & app, const QString & context_name, const QString & context_value )
{
	d->currentElement=0l;
	d->eventList->fill( app.isEmpty() ? KGlobal::mainComponent().componentName()  : app , context_name , context_value );
}


void KNotifyConfigWidget::slotEventSelected( KNotifyConfigElement * e )
{
	if(d->currentElement)
	{
		d->actionsconfig->save( d->currentElement );
	}
	d->currentElement=e;
	if(e)
	{
		d->actionsconfig->setConfigElement( e);
		d->actionsconfig->setEnabled(true);
	}
	else
		d->actionsconfig->setEnabled(false);

}

void KNotifyConfigWidget::save( )
{
	if(d->currentElement)
		d->actionsconfig->save( d->currentElement );
	

	d->eventList->save();
	emit changed(false);
	
	//ask the notify daemon to reload the config
	if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.knotify")) 
	{
		QDBusInterface( QLatin1String("org.kde.knotify"), QLatin1String("/Notify"),
						QLatin1String("org.kde.KNotify")).call( "reconfigure" );
	}
}

KNotifyConfigWidget * KNotifyConfigWidget::configure( QWidget * parent, const QString & appname )
{
	KDialog *dialog=new KDialog(parent);
	dialog->setCaption(i18n("Configure Notifications"));
	KNotifyConfigWidget *w=new KNotifyConfigWidget(dialog);
	dialog->setMainWidget(w);
	
	connect(dialog,SIGNAL(applyClicked()),w,SLOT(save()));
	connect(dialog,SIGNAL(okClicked()),w,SLOT(save()));
	connect(w,SIGNAL(changed(bool)) , dialog , SLOT(enableButtonApply(bool)));

	w->setApplication(appname);
	dialog->show();
	return w;
}

void KNotifyConfigWidget::slotActionChanged()
{
	emit changed( true ); //TODO
	if(d->currentElement)
	{
		d->actionsconfig->save( d->currentElement );
		d->eventList->updateCurrentItem();
	}
}


#include "knotifyconfigwidget.moc"
