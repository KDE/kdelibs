/*
   This file is part of the KDE libraries

   Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>

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

#include "kcmodule.h"
#include <kinstance.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

class KCModulePrivate
{
public:
        KInstance *_instance;
	QString _rootOnlyMsg;
	bool _useRootOnlyMsg;
        bool _hasOwnInstance;
};

KCModule::KCModule(QWidget *parent, const char *name, const QStringList &)
	: QWidget(parent, name), _btn(Help|Default|Apply)
{
    kdDebug( 281 ) << "KCModule " << name << endl;
    d = new KCModulePrivate;
    d->_useRootOnlyMsg = true;
	if (name && strlen(name)) {
		d->_instance = new KInstance(name);
		KGlobal::locale()->insertCatalogue(name);
	} else
		d->_instance = new KInstance("kcmunnamed");
    d->_hasOwnInstance = true;
    KGlobal::setActiveInstance(this->instance());
}

KCModule::KCModule(KInstance *instance, QWidget *parent, const QStringList & )
    : QWidget(parent, instance ? instance->instanceName().data() : 0), _btn(Help|Default|Apply)
{
    kdDebug( 281 ) << "KCModule instance " << (instance ? instance->instanceName().data() : "none") << endl;
    d = new KCModulePrivate;
    d->_useRootOnlyMsg = true;
    d->_instance = instance;
    KGlobal::locale()->insertCatalogue(instance->instanceName());
    d->_hasOwnInstance = false;
    KGlobal::setActiveInstance(this->instance());
}

KCModule::~KCModule()
{
    if (d->_hasOwnInstance)
       delete d->_instance;
    delete d;
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

KInstance *KCModule::instance() const
{
    return d->_instance;
}

void KCModule::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kcmodule.moc"

// vim: sw=4 et sts=4
