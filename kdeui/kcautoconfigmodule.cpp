/*  This file is part of the KDE project
    Copyright (C) 2003 Olivier Goffart <ogoffart@tiscalinet.be>

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

#include <kautoconfig.h>
#include <qlayout.h>
#include "kcautoconfigmodule.h"

class KCAutoConfigModule::KCAutoConfigModulePrivate
{
	public:
	KAutoConfig *kautoconfig;
};


KCAutoConfigModule::KCAutoConfigModule( QWidget * parent, const char * name, const QStringList & args )
		: KCModule( parent, name, args )
		, d( new KCAutoConfigModulePrivate )
{
	d->kautoconfig = new KAutoConfig( this );
	connect(d->kautoconfig, SIGNAL(widgetModified()), SLOT(slotWidgetModified()));
	connect(d->kautoconfig, SIGNAL(settingsChanged()), SLOT(widgetModified()));
}

KCAutoConfigModule::KCAutoConfigModule( KInstance * instance, QWidget * parent, const QStringList & args )
    : KCModule( instance, parent, args )
    , d( new KCAutoConfigModulePrivate )
{
	d->kautoconfig = new KAutoConfig( this );
	connect(d->kautoconfig, SIGNAL(widgetModified()), SLOT(slotWidgetModified()));
	connect(d->kautoconfig, SIGNAL(settingsChanged()), SLOT(slotWidgetModified()));
}



KCAutoConfigModule::KCAutoConfigModule( KConfig *config,QWidget * parent, const char * name, const QStringList & args )
		: KCModule( parent, name, args )    , d( new KCAutoConfigModulePrivate )
{
	d->kautoconfig = new KAutoConfig( config, this );
	connect(d->kautoconfig, SIGNAL(widgetModified()), SLOT(slotWidgetModified()));
	connect(d->kautoconfig, SIGNAL(settingsChanged()), SLOT(slotWidgetModified()));
}

KCAutoConfigModule::KCAutoConfigModule( KConfig *config , KInstance * instance, QWidget * parent, const QStringList & args )
    : KCModule( instance, parent, args )
    , d( new KCAutoConfigModulePrivate )
{
	d->kautoconfig = new KAutoConfig( config, this );
	connect(d->kautoconfig, SIGNAL(widgetModified()), SLOT(slotWidgetModified()));
	connect(d->kautoconfig, SIGNAL(settingsChanged()), SLOT(slotWidgetModified()));
}


KCAutoConfigModule::~KCAutoConfigModule()
{
	delete d;
}


void KCAutoConfigModule::load()
{
	d->kautoconfig->reloadSettings();
}

void KCAutoConfigModule::save()
{
    d->kautoconfig->saveSettings();
}

void KCAutoConfigModule::defaults()
{
    d->kautoconfig->resetSettings();
}

void KCAutoConfigModule::slotWidgetModified()
{
    emit changed(d->kautoconfig->hasChanged());
}

KAutoConfig *KCAutoConfigModule::autoConfig()
{
	return d->kautoconfig;
}

void KCAutoConfigModule::setMainWidget(QWidget *widget, const QString& group )
{
	QBoxLayout * l = new QVBoxLayout( this );
	l->addWidget( widget );

	d->kautoconfig->addWidget(widget,group);
	d->kautoconfig->retrieveSettings(true);
}




#include "kcautoconfigmodule.moc"
// vim: sw=4 sts=4 et
