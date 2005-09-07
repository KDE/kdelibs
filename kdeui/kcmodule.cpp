/*
   This file is part of the KDE libraries

<<<Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
   Copyright (C) 2004 Frans Englich <frans.englich@telia.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/

#include <qlayout.h>

#include <kaboutdata.h>
#include <kconfigskeleton.h>
#include <kconfigdialogmanager.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kinstance.h>
#include <klocale.h>

#include "kcmodule.h"
#include "kcmodule.moc"

class KCModulePrivate
{
public:
    KCModulePrivate():
        _about( 0 ),
        _useRootOnlyMsg( false ),
        _hasOwnInstance( true ),
        _unmanagedWidgetChangeState( false )
        { }

    KInstance *_instance;
    KAboutData *_about;
    QString _rootOnlyMsg;
    bool _useRootOnlyMsg;
    bool _hasOwnInstance;
    Q3PtrList<KConfigDialogManager> managers;
    QString _quickHelp;

    // this member is used to record the state on non-automatically
    // managed widgets, allowing for mixed KConfigXT-drive and manual
    // widgets to coexist peacefully and do the correct thing with
    // the changed(bool) signal
    bool _unmanagedWidgetChangeState;
};

KCModule::KCModule(QWidget *parent, const char *name, const QStringList &)
    : QWidget(parent)
{
    init();
    if (name && strlen(name)) {
        d->_instance = new KInstance(name);
        KGlobal::locale()->insertCatalogue(name);
    } else
        d->_instance = new KInstance("kcmunnamed");
    KGlobal::setActiveInstance(this->instance());

    d->managers.setAutoDelete( true );

}

KCModule::KCModule(KInstance *instance, QWidget *parent, const QStringList & )
    : QWidget( parent )
{
    if ( instance )
        setObjectName( instance->instanceName() );
    init();
    d->_instance = instance;
    KGlobal::locale()->insertCatalogue(instance->instanceName());
    d->_hasOwnInstance = false;
    KGlobal::setActiveInstance(this->instance());
}

void KCModule::init()
{
    d = new KCModulePrivate;
   _btn = Help|Default|Apply;
}

KConfigDialogManager* KCModule::addConfig( KConfigSkeleton *config, QWidget* widget )
{
    KConfigDialogManager* manager = new KConfigDialogManager( widget, config );
    manager->setObjectName( name() );
    connect( manager, SIGNAL( widgetModified() ), SLOT( widgetChanged() ));
    d->managers.append( manager );
    return manager;
}

KCModule::~KCModule()
{
    if (d->_hasOwnInstance)
       delete d->_instance;
    delete d->_about;
    delete d;
}

void KCModule::load()
{
    KConfigDialogManager* manager;
    for( manager = d->managers.first(); manager; manager = d->managers.next() )
        manager->updateWidgets();
}

void KCModule::save()
{
    KConfigDialogManager* manager;
    for( manager = d->managers.first(); manager; manager = d->managers.next() )
        manager->updateSettings();
    emit( changed( false ));
}

void KCModule::defaults()
{
    KConfigDialogManager* manager;
    for( manager = d->managers.first(); manager; manager = d->managers.next() )
        manager->updateWidgetsDefault();
}

void KCModule::widgetChanged()
{
    emit changed(d->_unmanagedWidgetChangeState || managedWidgetChangeState());
}

bool KCModule::managedWidgetChangeState() const
{
    KConfigDialogManager* manager;
    for( manager = d->managers.first(); manager; manager = d->managers.next() )
    {
        if ( manager->hasChanged() )
            return true;
    }

    return false;
}

void KCModule::unmanagedWidgetChangeState(bool changed)
{
    d->_unmanagedWidgetChangeState = changed;
    widgetChanged();
}

const KAboutData *KCModule::aboutData() const
{
    return d->_about;
}

void KCModule::setAboutData( KAboutData* about )
{
    delete d->_about;
    d->_about = about;
}

void KCModule::setRootOnlyMsg(const QString& msg)
{
    d->_rootOnlyMsg = msg;
}

QString KCModule::rootOnlyMsg() const
{
    return d->_rootOnlyMsg;
}

void KCModule::setUseRootOnlyMsg(bool on)
{
    d->_useRootOnlyMsg = on;
}

bool KCModule::useRootOnlyMsg() const
{
    return d->_useRootOnlyMsg;
}

void KCModule::changed()
{
    emit changed(true);
}

KInstance *KCModule::instance() const
{
    return d->_instance;
}

void KCModule::setQuickHelp( const QString& help )
{
    d->_quickHelp = help;
    emit( quickHelpChanged() );
}

QString KCModule::quickHelp() const
{
    return d->_quickHelp;
}


const Q3PtrList<KConfigDialogManager>& KCModule::configs() const
{
    return d->managers;
}

void KCModule::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

// vim: sw=4 et sts=4
