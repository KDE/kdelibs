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

class KCModulePrivate
{
public:
	QString _rootOnlyMsg;
	bool _useRootOnlyMsg;
};

KCModule::KCModule(QWidget *parent, const char *name)
	: QWidget(parent, name), _btn(Help|Default|Apply)
{
	d = new KCModulePrivate;
	d->_useRootOnlyMsg = true;
}

KCModule::~KCModule()
{
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

#include "kcmodule.moc"
